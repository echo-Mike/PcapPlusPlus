#ifndef PCAPPP_ALLOCATORADAPTER
#define PCAPPP_ALLOCATORADAPTER

#include "CPP11.h"
#include "MemoryUtils.h"
#include "MoveSemantics.h"
#include "CompressedPair.h"

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
			 */
			AllocatorAdapter() : Base() {}

			/**
			 * @brief Basic constructor.
			 * Constructs allocator via copy-construction.
			 */
			AllocatorAdapter(Allocator& alloc) : Base(alloc) {}

			/**
			 * @brief Copy constructor.
			 * @param[in] other The instance to make copy of.
			 */
			AllocatorAdapter(const AllocatorAdapter& other) : Base(other) {}

			/**
			 * @brief Copy assignment operator.
			 * Don't allows self assignment.
			 * @param[in] other The instance to make copy of.
			 */
			AllocatorAdapter& operator=(const AllocatorAdapter& other) 
			{ 
				if (this == &other)
					return *this;
				Base::operator=(other);
				return *this;
			}

			/**
			 * @brief Move constructor.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_CONSTRUCTOR(AllocatorAdapter) :
				Base(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER)) {}

			/**
			 * @brief Move assignment operator.
			 * Don't allows self assignment.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_ASSIGNMENT(AllocatorAdapter)
			{
				// Handle self assignment case
				if (this == &PCAPPP_MOVE_OTHER)
					return *this;
				Base::operator=(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER));
				return *this;
			}

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

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_ALLOCATORADAPTER */