#ifndef PCAPPP_ALLOCATORADAPTER
#define PCAPPP_ALLOCATORADAPTER

#include "CPP11.h"
#include "PCAPPPMemory.h"

#ifdef PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER
	#define PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ virtual
#else
	#define PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__
	#define PCAPPP_SUPPRESS_VIRTUAL_BASE_ALLOCATOR_ADAPTER__
#endif

/// @file

/**
 * \namespace pcpp
 * \brief The main namespace for the PcapPlusPlus lib
 */
namespace pcpp
{
	/**
	 * \namespace memory
	 * \brief Namespace that contains memory manipulation features
	 */
	namespace memory
	{

		template < typename Allocator,
		typename traits = allocator_traits<Allocator>,
		typename traits::allocator_type& (*GetAllocator)() = &staticAllocator<Allocator>,
		typename traits::allocator_type::pointer (traits::allocator_type::* Allocate)(std::size_t) = &traits::allocator_type::allocate,
		void (traits::allocator_type::* Deallocate)(typename traits::pointer) = &traits::allocator_type::deallocate >
		struct AllocatorAdapter
		{
			typedef traits allocator_traits;

			AllocatorAdapter() : m_Allocator(GetAllocator()) {}

			AllocatorAdapter(const AllocatorAdapter& other) : m_Allocator(other.m_Allocator) {}

			AllocatorAdapter& operator=(const AllocatorAdapter& other) 
			{ 
				if (this == &other)
					return *this;
				m_Allocator = other.m_Allocator;
				return *this;
			}

#ifdef ENABLE_CPP11_MOVE_SEMANTICS

			AllocatorAdapter(AllocatorAdapter&& other) : m_Allocator(std::move(other.m_Allocator)) {}

			AllocatorAdapter& operator=(AllocatorAdapter&& other)
			{
				if (this == &other)
					return *this;
				m_Allocator = std::move(other.m_Allocator);
				return *this;
			}
#endif
			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ ~AllocatorAdapter() {};

			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline typename traits::allocator_type& getAllocator() const { return m_Allocator; }

			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline void setAllocator(typename traits::allocator_type& allocator) const { m_Allocator = allocator; }

			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline typename traits::pointer allocate(std::size_t length = 1) const { return (m_Allocator.*Allocate)(length); }

			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline void deallocate(typename traits::pointer p) const { (m_Allocator.*Deallocate)(p); }

			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline void initialize() { m_Allocator = GetAllocator(); }

		protected:
			mutable typename traits::allocator_type m_Allocator;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_ALLOCATORADAPTER */