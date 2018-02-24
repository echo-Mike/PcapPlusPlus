#ifndef PCAPPP_MEMORY
#define PCAPPP_MEMORY

#include "CPP11.h"

/**
 * If PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR is defined then the allocator class has no virtual methods (and it is not an abstract class).
 * It's size is dropped from a size of pointer on your platform to 1 byte (or less). And therefore the size of default_allocator.
 * We are heavily rely on the size of this class because it is a member of RawPacket class around which a whole library constructed and it's size
 * is significant for efficiency.
 */
#define PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR

#ifdef PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR
	#define PCAPPP_SUPPRESS_VIRTUAL_BASE_ALLOCATOR__
#endif // PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR

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
		 * @brief Helper structure that wraps up a call to delete expression.
		 * @tparam T The base type of value to be deallocated via delete.
		 */
		template < typename T >
		struct default_delete
		{
			/**
			 * Type of pointer to value type.
			 */
			typedef T* pointer;
			/**
			 * @brief Calls delete expression on provided pointer.
			 * Does nothing if provided pointer is same as result of expression: pointer().
			 * @param ptr Pointer to object to be deleted.
			 */
			void operator()(pointer ptr) const
			{
				if (ptr != pointer()) 
				{
					delete ptr;
				}
			}
		};
		/**
		 * @brief Helper structure that wraps up a call to delete[] expression.
		 * This specialization is provided for deallocation of arrays of object with type T.
		 * @tparam T The base type of value to be deallocated via delete[].
		 */
		template < typename T >
		struct default_delete<T[]>
		{
			/**
			 * Type of pointer to value type.
			 */
			typedef T* pointer;
			/**
			 * @brief Calls delete[] expression on provided pointer.
			 * Does nothing if provided pointer is same as result of expression: pointer().
			 * @param ptr Pointer to object to be deleted.
			 */
			void operator()(pointer ptr) const
			{
				if (ptr != pointer())
				{
					delete[] ptr;
				}
			}
		};

		/**
		 * @brief The base interface class for all allocators.
		 * Defines the basic types used by allocators and their users.\n
		 * Defines the interface which all allocators must implement.\n
		 * May be an abstract class. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
		 * @tparam T The type for which the allocator is responsible to allocate memory.
		 */ 
		template < typename T >
		struct allocator
		{
			/**
			 * Verbose type for T class template parameter.
			 */
			typedef T value_type;
			/**
			 * Type of pointer to T.
			 */
			typedef T* pointer;
			/**
			 * Type of pointer to const T.
			 */
			typedef const T* const_pointer;
			/**
			 * Type of reference to T.
			 */
			typedef T& reference;
			/**
			 * Type of reference to const T.
			 */
			typedef const T& const_reference;
#ifdef PCAPPP_SUPPRESS_VIRTUAL_BASE_ALLOCATOR__
			/**
			 * @brief Represents the facility responsible for memory allocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] l Some allocators (array allocator for example) need this parameter to determine how much memory must be allocated.
			 * @return Pointer to newly allocated memory (as an interface function) or nullptr (as not interface function).
			 */
			pointer allocate(std::size_t l) { return nullptr; }
			/**
			 * @brief Represents the facility responsible for memory deallocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			void deallocate(pointer p) {}
#else
			/**
			 * @brief Represents the facility responsible for memory allocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] l Some allocators (array allocator for example) need this parameter to determine how much memory must be allocated.
			 * @return Pointer to newly allocated memory (as an interface function) or nullptr (as not interface function).
			 */
			virtual T* allocate(std::size_t l) = 0;
			/**
			 * @brief Represents the facility responsible for memory deallocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			virtual void deallocate(pointer p) = 0;
#endif // PCAPPP_SUPPRESS_VIRTUAL_BASE_ALLOCATOR__
		};

		/**
		 * @brief The default allocator main template.
		 * At the time before C++11 there was no default allocator class so we must define our own.\n
		 * The most basic implementation of allocator abstract interface.\n
		 * allocate method returns the result of "new T" expression. \n
		 * deallocate method applies "delete" expression to provided pointer.
		 * @tparam T The type for which the allocator is responsible to allocate memory.
		 */ 
		template < typename T >
		struct default_allocator
			: public allocator<T>
		{
			/**
			 * Base type of this class.
			 */
			typedef allocator<T> Base;
			/**
			 * @brief Allocates memory for one object of type T.
			 * WARNING!! The default constructor of object is called.\n
			 * Returns result of "new T" expression.
			 * @param[in] l Ignores it.
			 * @return Pointer to newly allocated memory.
			 */
			pointer allocate(std::size_t) { return new T; }
			/**
			 * @brief Deallocates memory for one object of type T.
			 * Internally applies "delete" expression to provided pointer.
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			void deallocate(pointer p)
			{
				if (p != typename Base::pointer())
				{
					delete p;
				}
			}
		};
		/**
		 * @brief The partial specialization of default allocator template for dynamic arrays of objects.
		 * At the time before C++11 there was no default allocator class so we must define our own.\n
		 * The most basic implementation of allocator abstract interface for dynamic arrays.\n
		 * allocate method returns the result of "new T[size]" expression. \n
		 * deallocate method applies "delete[]" expression to provided pointer.
		 * @tparam T The type for which the allocator is responsible to allocate memory.
		 */
		template < typename T >
		struct default_allocator<T[]> :
			public allocator<T>
		{
			/**
			 * Base type of this class.
			 */
			typedef allocator<T> Base;
			/**
			 * @brief Allocates memory for size object of type T.
			 * WARNING!! The default constructor of every object is called.\n
			 * Returns result of "new T[size]" expression.
			 * @param[in] size Count of objects to be allocated.
			 * @return Pointer to newly allocated memory.
			 */
			pointer allocate(std::size_t size) { return new T[size]; }
			/**
			 * @brief Deallocates memory for array of objects of type T.
			 * Internally applies "delete[]" expression to provided pointer.
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			void deallocate(pointer p)
			{
				if (p != typename Base::pointer())
				{
					delete[] p;
				}
			}
		};

		/**
		 * @brief Helper structure that wraps up types that may be declared by the provided allocator type.
		 * @tparam Allocator The allocator type to be wrapped up.
		 */
		template < typename Allocator >
		struct allocator_traits
		{
			/**
			 * Verbose type for Allocator class template parameter.
			 */
			typedef Allocator allocator_type;
			/**
			 * The type of value that Allocator is responsible to allocate memory for.
			 */
			typedef typename Allocator::value_type  value_type;
			/**
			 * The type of pointer to Allocator's value type.
			 */
			typedef typename Allocator::pointer pointer;
			/**
			 * The type of pointer to Allocator's constant value type.
			 */
			typedef typename Allocator::const_pointer const_pointer;
			/**
			 * The type of reference to Allocator's value type.
			 */
			typedef typename Allocator::reference reference;
			/**
			 * The type of reference to Allocator's constant value type.
			 */
			typedef typename Allocator::const_reference const_reference;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORY */