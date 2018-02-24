#ifndef PCAPPP_ALLOCATORADAPTER
#define PCAPPP_ALLOCATORADAPTER

#include "CPP11.h"
#include "MemoryUtils.h"
#include "MemoryImplementation.h"

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
		 * @brief Adapter class over any allocator with a simple allocate/deallocate interface.
		 * The interface of allocator is described in terms of template function parameters.\n
		 * This class size is exactly the size of provided Allocator type.\n
		 * For types that doesn't expose mentioned interface the compile-time error raises.
		 * @tparam Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
		 * @tparam traits Defines a set of requirements for Allocator type (Default: pcpp::memory::allocator_traits<Allocator>).
		 * @tparam Allocate Represents the member function of Allocator type that will allocate memory (Default: Allocator::allocate).
		 * @tparam Deallocate Represents the member function of Allocator type that will deallocate memory (Default: Allocator::deallocate).
		 */
		template < typename Allocator,
			typename traits = allocator_traits<Allocator>,
			typename traits::allocator_type::pointer (traits::allocator_type::* Allocate)(std::size_t) = &traits::allocator_type::allocate,
			void (traits::allocator_type::* Deallocate)(typename traits::pointer) = &traits::allocator_type::deallocate 
		>
		struct AllocatorAdapter :
			protected Allocator
		{
			/**
			 * Represents the type of allocator traits.
			 */
			typedef traits allocator_traits;
			/**
			 * Type alias for convenient usage of base class.
			 */
			typedef Allocator Base;

			/**
			 * @brief Default constructor.
			 * Will call GetAllocator function to get a new allocator and construct internal allocator from result of that call.
			 */
			AllocatorAdapter() : Base() {}
			/**
			 * @brief Copy constructor.
			 * Will copy the allocator of other.
			 * @param[in] other The instance to make copy of.
			 */
			AllocatorAdapter(const AllocatorAdapter& other) : Base(other) {}
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
				Base::operator=(other);
				return *this;
			}
#ifdef ENABLE_CPP11_MOVE_SEMANTICS
			/**
			 * @brief Move constructor.
			 * Will move the allocator from other.\n
			 * This function is unavailable if ENABLE_CPP11_MOVE_SEMANTICS macro is not defined.
			 * @param[in:out] other The instance to move from.
			 */
			AllocatorAdapter(AllocatorAdapter&& other) : Base(std::move(other)) {}
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
				Base::operator=(std::move(other));
				return *this;
			}
#endif
			/**
			 * @brief Destructor.
			 */
			~AllocatorAdapter() {};

			/**
			 * @brief Represents the access facility to the underlying allocator object.
			 * @return Reference to underlying allocator object.
			 */
			inline typename traits::allocator_type& getAllocator() { return *this; }
			/**
			 * @brief The wrapper over internal allocator object's allocate member method.
			 * Will internally call Allocate class template parameter on internal allocator object and forward the length function parameter to it.\n
			 * @param[in] length Some value to provide to internal allocator (may refer to length of array).
			 * @return Pointer to new memory allocated by internal allocator.
			 */
			inline typename traits::pointer allocate(std::size_t length = 1) { return (getAllocator().*Allocate)(length); }
			/**
			 * @brief The wrapper over internal allocator object's deallocate member method.
			 * Will internally call Deallocate class template parameter on internal allocator object and forward the p function parameter to it.\n
			 * @param[in] p Pointer to provide to internal allocator (may refer to memory allocated by this allocator).
			 */
			inline void deallocate(typename traits::pointer p) { (getAllocator().*Deallocate)(p); }
		};

		/**
		 * \namespace Implementation
		 * \brief Special namespace that wraps up an implementation details of some classes from pcpp::memory namespace.
		 */
		namespace Implementation
		{
			/**
			 * @brief Specialization that dispatches any default_delete<T[]> type to CompressedPair<default_delete<T[]>, T2, TrueFlag>
			 * This is an example of dispatching specialization.
			 * This specialization don't dispatches default_delete<T>,
			 * @tparam T2 Some type to be passed to default_delete template.
			 */
			template < typename T1, typename T2 >
			struct CompressedPairDispatcher< AllocatorAdapter< T1 >, T2 >
			{
				typedef CompressedPair<AllocatorAdapter< T1 >, T2, type_traits::true_type> pair_type;
			};

		} // namespace pcpp::memory::Implementation

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_ALLOCATORADAPTER */