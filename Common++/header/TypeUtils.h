#ifndef PCAPPP_TYPE_UTILS
#define PCAPPP_TYPE_UTILS

#include <cstdint>

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
	 * \brief Namespace that contains type traits features not presented in C++98
	 */
	namespace type_traits
	{

		/* Basic structures */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/integral_constant
		 */

		template < typename T, T v >
		struct integral_constant
		{
			static const T value = v;
			typedef T value_type;
			typedef integral_constant type;
			operator value_type() const { return value; }
			value_type operator()() const { return value; }
		};

		/**
		 * @brief Special tag type for template dispatching with two specializations.
		 * Represents true outcome.
		 */
		typedef integral_constant<bool, true> true_type;
		/**
		 * @brief Special tag type for template dispatching with two specializations.
		 * Represents false outcome.
		 */
		typedef integral_constant<bool, false> false_type;

		/* Remove type specifiers */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/remove_pointer
		 */
		/**
		 * @brief Standard type traits remove_pointer trait replacement for C++98.
		 * Contains type "type" if provided T is not a pointer.
		 */
		template< class T > struct remove_pointer { typedef T type; };
		/**
		 * @brief Standard type traits remove_pointer trait replacement for C++98.
		 * Contains type "type" if provided type is simple pointer.
		 */
		template< class T > struct remove_pointer<T*> { typedef T type; };
		/**
		 * @brief Standard type traits remove_pointer trait replacement for C++98.
		 * Contains type "type" if provided type is a pointer to some const-qualified type.
		 */
		template< class T > struct remove_pointer<T* const> { typedef T type; };
		/**
		 * @brief Standard type traits remove_pointer trait replacement for C++98.
		 * Contains type "type" if provided type is a pointer to some volatile-qualified type.
		 */
		template< class T > struct remove_pointer<T* volatile> { typedef T type; };
		/**
		 * @brief Standard type traits remove_pointer trait replacement for C++98.
		 * Contains type "type" if provided type is a pointer to some const-volatile qualified type.
		 */
		template< class T > struct remove_pointer<T* const volatile> { typedef T type; };

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/remove_reference
		 */
		/**
		 * @brief Standard type traits remove_reference trait replacement for C++98.
		 * Contains type "type" if provided T is not a reference.
		 */
		template< class T > struct remove_reference { typedef T type; };
		/**
		 * @brief Standard type traits remove_reference trait replacement for C++98.
		 * Contains type "type" if provided T is a reference.
		 */
		template< class T > struct remove_reference<T&> { typedef T type; };
#ifdef ENABLE_CPP11_MOVE_SEMANTICS
		/**
		 * @brief Standard type traits remove_reference trait replacement for C++98.
		 * Contains type "type" if provided T is a rvalue reference.
		 */
		template< class T > struct remove_reference<T&&> { typedef T type; };
#endif

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/remove_cv
		 */

		template< class T > struct remove_const { typedef T type; };

		template< class T > struct remove_const<const T> { typedef T type; };

		template< class T > struct remove_volatile { typedef T type; };

		template< class T > struct remove_volatile<volatile T> { typedef T type; };

		template< class T >
		struct remove_cv {
			typedef typename std::remove_volatile<typename std::remove_const<T>::type>::type type;
		};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/remove_extent
		 */

		template<class T>
		struct remove_extent { typedef T type; };

		template<class T>
		struct remove_extent<T[]> { typedef T type; };

		template<class T, std::size_t N>
		struct remove_extent<T[N]> { typedef T type; };

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/remove_all_extents
		 */

		template<class T>
		struct remove_all_extents { typedef T type; };

		template<class T>
		struct remove_all_extents<T[]> {
			typedef typename remove_all_extents<T>::type type;
		};

		template<class T, std::size_t N>
		struct remove_all_extents<T[N]> {
			typedef typename remove_all_extents<T>::type type;
		};

		/* Add type specifiers */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/add_cv
		 */

		template< class T >
		struct add_cv { typedef const volatile T type; };

		template< class T> struct add_const { typedef const T type; };

		template< class T> struct add_volatile { typedef volatile T type; };
		
		/* "is something" checkers */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_pointer
		 */

		namespace detail
		{
			template< class T > struct is_pointer_helper : false_type {};
			template< class T > struct is_pointer_helper<T*> : true_type {};
		} // namespace pcpp::type_traits::detail
		
		template< class T > struct is_pointer : detail::is_pointer_helper<typename remove_cv<T>::type> {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_void
		 */

		template< class T >
		struct is_void : is_same<void, typename remove_cv<T>::type> {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_array
		 */

		template<class T>
		struct is_array : false_type {};

		template<class T>
		struct is_array<T[]> : true_type {};

		template<class T, std::size_t N>
		struct is_array<T[N]> : true_type {};

		/* Template argument checkers */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/conditional
		 */

		template<bool B, class T, class F>
		struct conditional { typedef T type; };

		template<class T, class F>
		struct conditional<false, T, F> { typedef F type; };

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_same
		 */

		template<class T, class U>
		struct is_same : false_type {};

		template<class T>
		struct is_same<T, T> : true_type {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/enable_if
		 */

		template<bool B, class T = void>
		struct enable_if {};

		template<class T>
		struct enable_if<true, T> { typedef T type; };

		/* Arrays miscellaneous */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/rank
		 */

		template<class T>
		struct rank : public integral_constant<std::size_t, 0> {};

		template<class T>
		struct rank<T[]> : public integral_constant<std::size_t, rank<T>::value + 1> {};

		template<class T, std::size_t N>
		struct rank<T[N]> : public integral_constant<std::size_t, rank<T>::value + 1> {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/extent
		 */

		template<class T, unsigned N = 0>
		struct extent : integral_constant<std::size_t, 0> {};

		template<class T>
		struct extent<T[], 0> : integral_constant<std::size_t, 0> {};

		template<class T, unsigned N>
		struct extent<T[], N> : extent<T, N - 1> {};

		template<class T, std::size_t I>
		struct extent<T[I], 0> : integral_constant<std::size_t, I> {};

		template<class T, std::size_t I, unsigned N>
		struct extent<T[I], N> : extent<T, N - 1> {};

	} // namespace pcpp::type_traits

} // namespace pcpp

#endif /* PCAPPP_TYPE_UTILS */