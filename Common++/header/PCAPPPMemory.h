#ifndef PCAPPP_MEMORY
#define PCAPPP_MEMORY

#include <memory>
#include <cstdlib>

#include "CPP11.h"

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
#ifndef PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR
		template < typename T >
		struct allocator
		{
			typedef T value_type;
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef T& reference;
			typedef const T& const_reference;

			virtual T* allocate(std::size_t) = 0;

			virtual void deallocate(pointer) = 0;
		};

		template < typename T >
		struct default_allocator :
			public allocator<T>
		{
			typedef allocator<T> Base;

			T* allocate(std::size_t) { return new T(); }

			void deallocate(pointer p) { delete p; }

		};

		template < typename T >
		struct default_allocator<T[]> :
			public allocator<T>
		{
			typedef allocator<T> Base;

			T* allocate(std::size_t size) { return new T[size]; }
			
			void deallocate(pointer p) { delete[] p; }
		};
#else
		template < typename T >
		struct default_allocator
		{
			typedef T value_type;
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef T& reference;
			typedef const T& const_reference;

			T* allocate(std::size_t) { return new T(); }

			void deallocate(pointer p) { delete p; }

		};

		template < typename T >
		struct default_allocator<T[]>
		{
			typedef T value_type;
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef T& reference;
			typedef const T& const_reference;

			T* allocate(std::size_t size) { return new T[size]; }
			
			void deallocate(pointer p) { delete[] p; }
		};
#endif
		template < typename Allocator >
		struct allocator_traits
		{
			typedef Allocator allocator_type;
			typedef typename Allocator::value_type  value_type;
			typedef typename Allocator::pointer pointer;
			typedef typename Allocator::const_pointer const_pointer;
			typedef typename Allocator::reference reference;
			typedef typename Allocator::const_reference const_reference;
		};

#ifndef NO_TEMPLATE_FUNCTION_DEF_ARGS

		template < typename Allocator, typename traits = allocator_traits<Allocator> >
		typename traits::allocator_type& staticAllocator()
		{
			static typename traits::allocator_type allocator;
			return allocator;
		}
#else

		template < typename Allocator >
		Allocator& staticAllocator()
		{
			static Allocator allocator;
			return allocator;
		}
#endif

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORY */