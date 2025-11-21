//
// Copyright (c) 2024 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//

#include <chrono>
#include <thread>
#include <unordered_set>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

#define ASSERT_OK(result) \
    if (result != Z_OK) { \
        assert(false);    \
        return result;    \
    }

#define ASSERT_TRUE(expr) \
    if (!(expr)) {        \
        assert(false);    \
        return -300;      \
    }

#define ASSERT_FALSE(expr) \
    if (expr) {            \
        assert(false);     \
        return -300;       \
    }

#define ASSERT_VALID(expr)               \
    if (!interop::detail::check(expr)) { \
        assert(false);                   \
        return -300;                     \
    }

#define ASSERT_NULL(expr)               \
    if (interop::detail::check(expr)) { \
        assert(false);                  \
        return -300;                    \
    }

int test_shm_buffer(ZShmMut&& buf) {
    ASSERT_VALID(buf);

    ZShm immut(std::move(buf));
    ASSERT_VALID(immut);
    ASSERT_NULL(buf);

    {
        ZShm immut2(immut);
        ASSERT_VALID(immut);
        ASSERT_VALID(immut2);
        {
            auto mut = ZShm::try_mutate(std::move(immut2));
            ASSERT_FALSE(mut);
            ASSERT_VALID(immut2);
        }
    }

    auto mut = ZShm::try_mutate(std::move(immut));
    ASSERT_TRUE(mut);
    ASSERT_NULL(immut);

    return 0;
}

template <typename Talloc>
bool check_alloc(Talloc&& alloc) {
    try {
        auto buf = std::get<ZShmMut>(std::move(alloc));
        ASSERT_OK(test_shm_buffer(std::move(buf)));
        ASSERT_NULL(buf);
        return true;
    } catch (...) {
    }
    return false;
}

bool test_layouted_allocation(const PrecomputedLayout& precomputed_layout) {
    auto alloc = precomputed_layout.alloc_gc();
    return check_alloc(std::move(alloc));
}

bool test_allocation(const ShmProvider& provider, size_t size, AllocAlignment alignment) {
    auto alloc = provider.alloc_gc(size, alignment);
    return check_alloc(std::move(alloc));
}

int test_provider(const ShmProvider& provider, AllocAlignment alignment, size_t buf_ok_size, size_t buf_err_size) {
    ASSERT_VALID(provider);

    // test allocation OK
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(test_allocation(provider, buf_ok_size, alignment));
    }

    // test allocation ERROR
    if (buf_err_size) {
        ASSERT_TRUE(!test_allocation(provider, buf_err_size, alignment));
    }

    // OK layouted allocations
    {
        // make OK allocation layout
        PrecomputedLayout precomputed_layout = provider.alloc_layout(buf_ok_size, alignment);
        ASSERT_VALID(precomputed_layout);

        // test layouted allocation OK
        for (int i = 0; i < 100; ++i) {
            ASSERT_TRUE(test_layouted_allocation(precomputed_layout));
        }
    }

    // ERR layouted allocation
    if (buf_err_size) {
        // make ERR allocation layout
        PrecomputedLayout precomputed_layout = provider.alloc_layout(buf_err_size, alignment);
        ASSERT_VALID(precomputed_layout);

        // test layouted allocation ERROR
        for (int i = 0; i < 100; ++i) {
            ASSERT_FALSE(test_layouted_allocation(precomputed_layout));
        }
    }

    // additional functions
    provider.defragment();
    provider.garbage_collect();

    return Z_OK;
}

class TestShmProviderBackend : public CppShmProviderBackend {
    uint8_t* bytes;
    bool* busy_flags;
    size_t count;
    size_t bytes_available;

   public:
    TestShmProviderBackend(size_t size)
        : bytes(new uint8_t[size]), busy_flags(new bool[size]), count(size), bytes_available(size) {}

    ~TestShmProviderBackend() {
        delete[] bytes;
        delete[] busy_flags;
    }

   private:
    static void deref_segemnt_fn(void* context) {}

   private:
    virtual ChunkAllocResult alloc(const MemoryLayout& layout) override {
        assert(interop::detail::check(layout));

        // check size and alignment
        const auto size = layout.size();
        const auto alignment = layout.alignment();
        assert(size == 1);
        assert(alignment.pow == 0);

        // perform allocation
        for (int i = 0; i < this->count; ++i) {
            if (!this->busy_flags[i]) {
                this->busy_flags[i] = true;
                this->bytes_available--;

                z_owned_ptr_in_segment_t ptr;
                zc_threadsafe_context_t segment = {{NULL}, &TestShmProviderBackend::deref_segemnt_fn};
                z_ptr_in_segment_new(&ptr, &this->bytes[i], segment);

                AllocatedChunk chunk;
                chunk.ptr = z_move(ptr);
                uint64_t data_ptr = (uint64_t)(&this->bytes[i]);
                chunk.descriptpr.chunk = data_ptr & 0xFFFFFFFF;
                chunk.descriptpr.len = 1;
                chunk.descriptpr.segment = (data_ptr >> 32) & 0xFFFFFFFF;

                return ChunkAllocResult(chunk);
            }
        }
        return ChunkAllocResult(AllocError::Z_ALLOC_ERROR_OUT_OF_MEMORY);
    }

    virtual void free(const ChunkDescriptor& chunk) override {
        assert(chunk.len == 1);

        // restore data ptr from chunk descriptor
        void* data = (void*)(((uint64_t)chunk.chunk) | ((((uint64_t)chunk.segment) << 32) & 0xFFFFFFFF00000000));

        // calc index from data ptr
        int64_t index = (int64_t)data - (int64_t)this->bytes;
        assert(index >= 0);
        assert(index < this->count);

        // mark this entry as free
        this->busy_flags[index] = false;
        this->bytes_available++;
    }

    virtual size_t defragment() override { return 0; }

    virtual size_t available() const override { return this->bytes_available; }

    virtual ProtocolId id() const override { return 100500; }

    virtual void layout_for(MemoryLayout& layout) override {
        assert(interop::detail::check(layout));

        // check size and alignment
        const auto size = layout.size();
        const auto alignment = layout.alignment();

        // incorrect layout will be invalidated
        if (size != 1 || alignment.pow != 0) {
            layout = interop::detail::null<MemoryLayout>();
        }
    }
};

int run_c_provider() {
    const size_t size = 1024;

    // create test backend
    auto backend = std::make_unique<TestShmProviderBackend>(size);

    // create provider
    CppShmProvider provider(std::move(backend));
    ASSERT_VALID(provider);

    // test provider
    AllocAlignment alignment = {0};
    ASSERT_OK(test_provider(provider, alignment, 1, 0));

    return Z_OK;
}

int run_posix_provider() {
    const size_t total_size = 4096;
    const size_t buf_ok_size = total_size / 4;
    const size_t buf_err_size = total_size * 100;

    const AllocAlignment alignment = {4};

    const MemoryLayout layout(total_size, alignment);
    ASSERT_VALID(layout);

    PosixShmProvider provider(layout);
    ASSERT_OK(test_provider(provider, alignment, buf_ok_size, buf_err_size));

    return Z_OK;
}

int test_client_storage(const ShmClientStorage& storage) {
    ASSERT_VALID(storage);

    auto session = Session::open(Config::create_default(), storage);
    ASSERT_VALID(session);
    ASSERT_VALID(storage);

    return Z_OK;
}

int run_default_client_storage() {
    ShmClientStorage storage;

    // test client storage
    ASSERT_OK(test_client_storage(storage));

    return Z_OK;
}

int run_global_client_storage() {
    // obtain defaul global client storage
    auto storage = ShmClientStorage::global();

    // test client storage
    ASSERT_OK(test_client_storage(storage));

    return Z_OK;
}

template <bool list_api>
int run_client_storage_for_list(std::vector<ShmClient>&& list) {
    // create storage
    auto storage = [list = std::move(list)]() mutable {
        if constexpr (list_api) {
            // create client storage from the list
            return ShmClientStorage(std::move(list), true);
        }
        // create client storage from iterators of the list
        return ShmClientStorage(std::make_move_iterator(list.begin()), std::make_move_iterator(list.end()), true);
    }();

    // test client storage
    ASSERT_OK(test_client_storage(storage));

    return Z_OK;
}

template <bool list_api>
int run_client_storage_impl() {
    // create client list
    std::vector<ShmClient> list;

    // create POSIX SHM Client
    PosixShmClient client;
    ASSERT_VALID(client);

    // add client to the list
    list.push_back(std::move(client));

    return run_client_storage_for_list<list_api>(std::move(list));
}

int run_client_storage() {
    ASSERT_OK(run_client_storage_impl<true>());
    ASSERT_OK(run_client_storage_impl<false>());
    return Z_OK;
}

class TestShmSegment : public CppShmSegment {
    SegmentId segment_id;

   public:
    TestShmSegment(SegmentId segment_id) : segment_id(segment_id) {}

   private:
    virtual uint8_t* map(z_chunk_id_t chunk_id) override {
        return (uint8_t*)((uint64_t)chunk_id | ((((uint64_t)segment_id) << 32) & 0xFFFFFFFF00000000));
    }
};

class TestShmClient : public CppShmClient {
    virtual std::unique_ptr<CppShmSegment> attach(SegmentId segment_id) override {
        return std::make_unique<TestShmSegment>(segment_id);
    }

    virtual ProtocolId id() const override { return 100500; }
};

int run_c_client() {
    // create client list
    std::vector<ShmClient> list;

    // create C SHM Client
    auto client = ShmClient(std::make_unique<TestShmClient>());
    ASSERT_VALID(client);

    // add client to the list
    list.push_back(std::move(client));
    ASSERT_NULL(client);

    // create client storage from the list
    // auto storage = ShmClientStorage(std::make_move_iterator(list.begin()), std::make_move_iterator(list.end()),
    // true);
    auto storage = ShmClientStorage(std::move(list), true);

    // test client storage
    ASSERT_OK(test_client_storage(storage));
    return Z_OK;
}

int run_cleanup() {
    cleanup_orphaned_shm_segments();
    return Z_OK;
}

int run_transport_provider() {
    auto session = Session::open(Config::create_default());

    auto shared_provider_state = session.obtain_shm_provider();
    ASSERT_TRUE(std::holds_alternative<ShmProviderNotReadyState>(shared_provider_state));

    while (std::holds_alternative<ShmProviderNotReadyState>(shared_provider_state)) {
        z_sleep_ms(100);
        shared_provider_state = session.obtain_shm_provider();
    }
    ASSERT_TRUE(std::holds_alternative<SharedShmProvider>(shared_provider_state));

    auto shared_provider = std::get<SharedShmProvider>(std::move(shared_provider_state));
    auto& provider = shared_provider.shm_provider();

    const size_t total_size = 4096;
    const size_t buf_ok_size = total_size / 4;
    const size_t buf_err_size = total_size * 1024 * 1024;
    const AllocAlignment alignment = {0};

    ASSERT_OK(test_provider(provider, alignment, buf_ok_size, buf_err_size));

    return Z_OK;
}

int main() {
    ASSERT_OK(run_posix_provider());
    ASSERT_OK(run_c_provider());
    ASSERT_OK(run_default_client_storage());
    ASSERT_OK(run_global_client_storage());
    ASSERT_OK(run_client_storage());
    ASSERT_OK(run_c_client());
    ASSERT_OK(run_cleanup());
    ASSERT_OK(run_transport_provider());
    return Z_OK;
}
