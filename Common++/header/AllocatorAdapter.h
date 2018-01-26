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
		/**
		 * @brief Adapter class over any allocator with a simple interface.
		 * The interface of allocator is described in terms of template funation parameters.\n
		 * This class size is directly proportional to size of provided Allocator type.
		 * @tparam Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
		 * @tparam traits Defines a set of requirements for Allocator type (Default: pcpp::memory::allocator_traits<Allocator>).
		 * @tparam GetAllocator Represents the function that can generate an allocator of type Allocator (Default: staticAllocator<Allocator>).
		 * @tparam Allocate Represents the member function of Allocator type that will allocate memory (Default: Allocator::allocate).
		 * @tparam Deallocate Represents the member function of Allocator type that will deallocate memory (Default: Allocator::deallocate).
		 */ 
		template < typename Allocator,
		typename traits = allocator_traits<Allocator>,
		typename traits::allocator_type& (*GetAllocator)() = &staticAllocator<Allocator>,
		typename traits::allocator_type::pointer (traits::allocator_type::* Allocate)(std::size_t) = &traits::allocator_type::allocate,
		void (traits::allocator_type::* Deallocate)(typename traits::pointer) = &traits::allocator_type::deallocate >
		struct AllocatorAdapter
		{
			/**
			 * Represents the type of allocator traits.
			 */
			typedef traits allocator_traits;
			/**
			 * @brief Default constructor.
			 * Will call GetAllocator function to get a new allocator and construct internal allocator from result of that call.
			 */
			AllocatorAdapter() : m_Allocator(GetAllocator()) {}
			/**
			 * @brief Copy constructor.
			 * Will copy the allocator of other.
			 * @param[in] other The instance to make copy of.
			 */
			AllocatorAdapter(const AllocatorAdapter& other) : m_Allocator(other.m_Allocator) {}
			/**
			 * @brief Copy assignment operator.
			 * Don't allows self assignment.\n
			 * Will copy the allocator of other.
			 * @param[in] other The instance to make copy of.
			 */
			AllocatorAdapter& operator=(const AllocatorAdapter& other) 
			{ 
				if (this == &other)
					return *this;
				m_Allocator = other.m_Allocator;
				return *this;
			}
#ifdef ENABLE_CPP11_MOVE_SEMANTICS
			/**
			 * @brief Move constructor.
			 * Will move the allocator from other.\n
			 * This function is unavailable if ENABLE_CPP11_MOVE_SEMANTICS macro is not defined.
			 * @param[in:out] other The instance to move from.
			 */
			AllocatorAdapter(AllocatorAdapter&& other) : m_Allocator(std::move(other.m_Allocator)) {}
			/**
			 * @brief Move assignment operator.
			 * Don't allows self assignment.\n
			 * Will move the allocator from other.\n
			 * This function is unavailable if ENABLE_CPP11_MOVE_SEMANTICS macro is not defined.
			 * @param[in:out] other The instance to move from.
			 */
			AllocatorAdapter& operator=(AllocatorAdapter&& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				m_Allocator = std::move(other.m_Allocator);
				return *this;
			}
#endif
			/**
			 * @brief Destructor.
			 * May be a virtual member. Depends on PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER macro def.
			 */
			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ ~AllocatorAdapter() {};
			/**
			 * @brief Represents the read access facility to the underlying allocator object.
			 * May be a virtual member. Depends on PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER macro def.
			 * @return Reference to underlying allocator object.
			 */
			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline typename traits::allocator_type& getAllocator() const { return m_Allocator; }
			/**
			 * @brief Represents the write access facility to the underlying allocator object.
			 * May be a virtual member. Depends on PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER macro def.
			 * @param[in] allocator Reference to the new allocator object to be used.
			 */
			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline void setAllocator(typename traits::allocator_type& allocator) const { m_Allocator = allocator; }
			/**
			 * @brief The wrapper over internal allocator object's allocate member method.
			 * Will internally call Allocate class template parameter on internal allocator object and forward the length function parametr to it.\n
			 * May be a virtual member. Depends on PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER macro def.
			 * @param[in] length Some value to provide to internal allocator (may refer to length of array).
			 * @return Pointer to new memory allocated by internal allocator.
			 */
			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline typename traits::pointer allocate(std::size_t length = 1) const { return (m_Allocator.*Allocate)(length); }
			/**
			 * @brief The wrapper over internal allocator object's deallocate member method.
			 * Will internally call Deallocate class template parameter on internal allocator object and forward the p function parametr to it.\n
			 * May be a virtual member. Depends on PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER macro def.
			 * @param[in] p Pointer to provide to internal allocator (may refer to memory allocated by this allocator).
			 */
			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline void deallocate(typename traits::pointer p) const { (m_Allocator.*Deallocate)(p); }
			/**
			 * @brief Forces to create new allocator by a call to GetAllocator template class parameter.
			 * Will assign returned value to internal allocator via simple assignment.\n
			 * May be a virtual member. Depends on PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER macro def.
			 */
			PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER__ inline void initialize() { m_Allocator = GetAllocator(); }
		protected:
			mutable typename traits::allocator_type m_Allocator;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_ALLOCATORADAPTER */