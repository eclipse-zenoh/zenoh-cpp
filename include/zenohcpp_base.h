#pragma once

#include <utility>
#include <optional>

namespace zenoh {

template<typename ZC_OWNED_TYPE> class Owned {
public:
    Owned() = delete;
    Owned& operator=(const Owned& v) = delete;
    Owned(const Owned& v) = delete;
    Owned(ZC_OWNED_TYPE&& v) : _0(v) { ::z_null(v); }
    Owned(Owned&& v) : Owned(std::move(v._0)) {}
    ~Owned() { ::z_drop(&_0); }
    ZC_OWNED_TYPE take() { auto r = _0; _0 = {}; return r; }
    bool check() const { return ::z_check(_0); }
protected:
    ZC_OWNED_TYPE _0;
};

template<typename ZC_CLOSURE_TYPE, typename ZC_CLOSURE_CALL_PARAM, typename ZCPP_CLOSURE_CALL_PARAM> class Closure : public Owned<ZC_CLOSURE_TYPE> {
public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Construct closure from any object which can be called with std::optional<param> where param is 'Reply', 'Query', etc.
    // Object is called with empty std::optional before drop.
    template<typename LAMBDA> Closure(LAMBDA&& lambda) : Owned<ZC_CLOSURE_TYPE>(wrap_lambda_to_closure<LAMBDA>(std::move(lambda))) {}

    // TODO: more constructors to be added here

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }
private:
    template<typename LAMBDA> ZC_CLOSURE_TYPE wrap_lambda_to_closure(LAMBDA&& lambda) {
        return {
            context: new LAMBDA(std::move(lambda)),
            call: [](ZC_CLOSURE_CALL_PARAM pvalue, void* ctx){ 
                    static_cast<LAMBDA*>(ctx)->operator()(std::optional<ZCPP_CLOSURE_CALL_PARAM>(std::in_place, std::move(*pvalue))); 
                },
            drop: [](void* ctx){
                    static_cast<LAMBDA*>(ctx)->operator()(std::optional<ZCPP_CLOSURE_CALL_PARAM>());
                    delete static_cast<LAMBDA*>(ctx);
                },
        };
    }
};

}