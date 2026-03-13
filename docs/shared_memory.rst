..
.. SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
..
.. Contributors:
..   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
..

Shared Memory
=============
.. doxygenclass:: zenoh::ZShm
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ZShmMut
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::CppShmClient
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ShmClient
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::CppShmSegment
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ShmClientStorage
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::PosixShmClient
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::PosixShmProvider
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ShmProvider
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::SharedShmProvider
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::CppShmProvider
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ShmProviderAsyncInterface
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::CppShmProviderBackendIface
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::CppShmProviderBackend
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::CppShmProviderBackendThreadsafe
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::PrecomputedLayout
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenstruct:: zenoh::PrecomputedLayoutAsyncInterface
   :members:

.. doxygenclass:: zenoh::MemoryLayout
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ChunkAllocResult
   :members:
   :membergroups: Constructors Operators Methods


.. doxygentypedef:: zenoh::ProtocolId
.. doxygentypedef:: zenoh::SegmentId
.. doxygentypedef:: zenoh::ChunkId

.. doxygentypedef:: zenoh::ChunkDescriptor
.. doxygentypedef:: zenoh::AllocatedChunk

.. doxygentypedef:: zenoh::AllocError
.. doxygentypedef:: zenoh::LayoutError
.. doxygentypedef:: zenoh::AllocAlignment

.. doxygentypedef:: zenoh::BufAllocResult
.. doxygentypedef:: zenoh::BufLayoutAllocResult

.. doxygenfunction:: zenoh::cleanup_orphaned_shm_segments


