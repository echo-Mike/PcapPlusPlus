#ifndef PCAPPP_TYPE_UTILS
#define PCAPPP_TYPE_UTILS

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
		/**
		 * @brief Standard type traits remove_const trait replacement for C++98.
		 * Contains type "type" if provided T is not const-qualified.
		 */
		template< class T > struct remove_const { typedef T type; };
		/**
		 * @brief Standard type traits remove_const trait replacement for C++98.
		 * Contains type "type" if provided T is const-qualified.
		 */
		template< class T > struct remove_const<const T> { typedef T type; };
		/**
		 * @brief Standard type traits remove_volatile trait replacement for C++98.
		 * Contains type "type" if provided T is not volatile-qualified.
		 */
		template< class T > struct remove_volatile { typedef T type; };
		/**
		 * @brief Standard type traits remove_volatile trait replacement for C++98.
		 * Contains type "type" if provided T is volatile-qualified.
		 */
		template< class T > struct remove_volatile<volatile T> { typedef T type; };
		/**
		 * @brief Standard type traits remove_cv trait replacement for C++98.
		 * Removes const- volatile- qualifiers from provided type T.
		 */
		template< class T >
		struct remove_cv {
			typedef typename remove_volatile<typename remove_const<T>::type>::type type;
		};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/remove_extent
		 */
		/**
		 * @brief Standard type traits remove_extent trait replacement for C++98.
		 * Contains type "type" if provided T is not an array.
		 */
		template<class T>
		struct remove_extent { typedef T type; };
		/**
		 * @brief Standard type traits remove_extent trait replacement for C++98.
		 * Contains type "type" if provided T is an array of unknown size.
		 */
		template<class T>
		struct remove_extent<T[]> { typedef T type; };
		/**
		 * @brief Standard type traits remove_extent trait replacement for C++98.
		 * Contains type "type" if provided T is not an array of some size.
		 */
		template<class T, unsigned long N>
		struct remove_extent<T[N]> { typedef T type; };

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/remove_all_extents
		 */
		/**
		 * @brief Standard type traits remove_all_extents trait replacement for C++98.
		 * Contains type "type" if provided T is not an array.
		 */
		template<class T>
		struct remove_all_extents { typedef T type; };
		/**
		 * @brief Standard type traits remove_all_extents trait replacement for C++98.
		 * Contains type "type" if provided T is an array of unknown size.
		 */
		template<class T>
		struct remove_all_extents<T[]> {
			typedef typename remove_all_extents<T>::type type;
		};
		/**
		 * @brief Standard type traits remove_all_extents trait replacement for C++98.
		 * Contains type "type" if provided T is an array of some size.
		 */
		template<class T, unsigned long N>
		struct remove_all_extents<T[N]> {
			typedef typename remove_all_extents<T>::type type;
		};

		/* Add type specifiers */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/add_cv
		 */
		/**
		 * @brief Standard type traits add_cv trait replacement for C++98.
		 * Contains type "type" that is defined as const volatile T.
		 */
		template< class T >
		struct add_cv { typedef const volatile T type; };
		/**
		 * @brief Standard type traits add_const trait replacement for C++98.
		 * Contains type "type" that is defined as const T.
		 */
		template< class T> struct add_const { typedef const T type; };
		/**
		 * @brief Standard type traits add_volatile trait replacement for C++98.
		 * Contains type "type" that is defined as volatile T.
		 */
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
		/**
		 * @brief Standard type traits is_pointer trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if T is a pointer type and false otherwise.
		 */
		template< class T > struct is_pointer : detail::is_pointer_helper<typename remove_cv<T>::type> {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_class
		 */

		namespace detail
		{
			struct two { char c[2]; };
			template <class T> char class_test(int T::*);
			template <class T> two class_test(...);
		} // namespace pcpp::type_traits::detail
		/**
		 * @brief Standard type traits is_class trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if T is a class, enum or struct type and false otherwise.
		 */
		template <class T>
		struct is_class : integral_constant<bool, sizeof(detail::class_test<T>(0)) == 1 > {};
		/**
		 * @brief Standard type traits is_empty trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if T is an empty class type and false otherwise.
		 */
		template <class T>
		struct is_empty : integral_constant<bool, is_class<T>::value && sizeof(T) == sizeof(true_type) > {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_array
		 */
		/**
		 * @brief Standard type traits is_array trait replacement for C++98.
		 * Fallback for not array types.
		 * Contains static constant boolean "value" which is false.
		 */
		template<class T>
		struct is_array : false_type {};
		/**
		 * @brief Standard type traits is_array trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if T is an type of array of unknown size.
		 */
		template<class T>
		struct is_array<T[]> : true_type {};
		/**
		 * @brief Standard type traits is_array trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if T is an type of array of some size.
		 */
		template<class T, unsigned long N>
		struct is_array<T[N]> : true_type {};

		/* Template argument checkers */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/conditional
		 */
		/**
		 * @brief Standard type traits conditional trait replacement for C++98.
		 * Contains type "type" that is defined as T if B is true.
		 */
		template<bool B, class T, class F>
		struct conditional { typedef T type; };
		/**
		 * @brief Standard type traits conditional trait replacement for C++98.
		 * Contains type "type" that is defined as F if B is false.
		 */
		template<class T, class F>
		struct conditional<false, T, F> { typedef F type; };

		/**
		 * Implementations of next classes were inspired by:
		 * http://en.cppreference.com/w/cpp/types/is_base_of
		 * No visible behavior differences from listed above implementation was found on clang, gcc and other compilers (in CompilerExplorer).
		 */
		namespace detail
		{
			template <typename Base> char is_base_of_test_func(const volatile Base*);
			template <typename Base> two  is_base_of_test_func(const volatile void*);
			template <typename Base, typename Derived, typename DerPtr = Derived* >
			struct pre_is_base_of : integral_constant<bool, sizeof(is_base_of_test_func<Base>(DerPtr())) == 1 > {};
		}
		/**
		 * @brief Standard type traits is_base_of trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if Base is a base type of Derived.
		 */
		template < typename Base, typename Derived >
		struct is_base_of : 
			public conditional <
				is_class< Base >::value && is_class< Derived >::value,
				detail::pre_is_base_of< Base, Derived >,
				false_type
		>::type {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_same
		 */
		/**
		 * @brief Standard type traits is_same trait replacement for C++98.
		 * Contains static constant boolean "value" which is false if T and U are not same type.
		 */
		template<class T, class U>
		struct is_same : false_type {};
		/**
		 * @brief Standard type traits is_same trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if T and U are same type.
		 */
		template<class T>
		struct is_same<T, T> : true_type {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/is_void
		 */
		/**
		 * @brief Standard type traits is_void trait replacement for C++98.
		 * Contains static constant boolean "value" which is true if T is a possibly cv-qualified void.
		 */
		template< class T >
		struct is_void : is_same< void, typename remove_cv<T>::type > {};
		
		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/enable_if
		 */
		/**
		 * @brief Standard type traits enable_if trait replacement for C++98.
		 * Contains type "type" that is defined as T if B is true otherwise it is false.
		 */
		template<bool B, class T = void>
		struct enable_if {};
		/**
		 * @brief Standard type traits enable_if trait replacement for C++98.
		 * Contains type "type" that is defined as T if B is true otherwise it is false.
		 */
		template<class T>
		struct enable_if<true, T> { typedef T type; };

		namespace detail
		{
			template< typename T, typename AT_1 = void, typename AT_2 = void, typename AT_3 = void, typename AT_4 = void >
			class is_constructible_impl
			{
			private:
				template< typename T_T, typename T_AT_1, typename T_AT_2, typename T_AT_3, typename T_AT_4 >
				static char test(
					typename enable_if<
						sizeof( T_T ) ==
						sizeof( T_T(
							static_cast< T_AT_1 >( *static_cast< typename remove_reference< T_AT_1 >::type* >( NULL ) ),
							static_cast< T_AT_2 >( *static_cast< typename remove_reference< T_AT_2 >::type* >( NULL ) ),
							static_cast< T_AT_3 >( *static_cast< typename remove_reference< T_AT_3 >::type* >( NULL ) ),
							static_cast< T_AT_4 >( *static_cast< typename remove_reference< T_AT_4 >::type* >( NULL ) )
						) )
					>::type*
				);

				template< typename, typename, typename, typename, typename >
				static int test( ... );

			public:
				static const bool value = ( sizeof( test< T, AT_1, AT_2, AT_3, AT_4 >( NULL ) ) == sizeof( char ) );
			};

			template< typename T, typename AT_1, typename AT_2, typename AT_3 >
			class is_constructible_impl< T, AT_1, AT_2, AT_3, void >
			{
			private:
				template< typename T_T, typename T_AT_1, typename T_AT_2, typename T_AT_3 >
				static char test(
					typename enable_if<
						sizeof( T_T ) ==
						sizeof( T_T(
							static_cast< T_AT_1 >( *static_cast< typename remove_reference< T_AT_1 >::type* >( NULL ) ),
							static_cast< T_AT_2 >( *static_cast< typename remove_reference< T_AT_2 >::type* >( NULL ) ),
							static_cast< T_AT_3 >( *static_cast< typename remove_reference< T_AT_3 >::type* >( NULL ) )
						) )
					>::type*
				);

				template< typename, typename, typename, typename >
				static int test( ... );

			public:
				static const bool value = ( sizeof( test< T, AT_1, AT_2, AT_3 >( NULL ) ) == sizeof( char ) );
			};

			template< typename T, typename AT_1, typename AT_2 >
			class is_constructible_impl< T, AT_1, AT_2, void, void >
			{
			private:

				template< typename T_T, typename T_AT_1, typename T_AT_2 >
				static char test(
					typename enable_if<
						sizeof( T_T ) ==
						sizeof( T_T(
							static_cast< T_AT_1 >( *static_cast< typename remove_reference< T_AT_1 >::type* >( NULL ) ),
							static_cast< T_AT_2 >( *static_cast< typename remove_reference< T_AT_2 >::type* >( NULL ) )
						) )
					>::type*
				);

				template< typename, typename, typename >
				static int test( ... );

			public:
				static const bool value = ( sizeof( test< T, AT_1, AT_2 >( NULL ) ) == sizeof( char ) );
			};

			template< typename T, typename AT_1 >
			class is_constructible_impl< T, AT_1, void, void, void >
			{
			private:
				template< typename T_T, typename T_AT_1 >
				static char test(
					typename enable_if<
						sizeof( T_T ) ==
						sizeof( T_T(
							static_cast< T_AT_1 >( *static_cast< typename remove_reference< T_AT_1 >::type* >( NULL ) )
						) )
					>::type*
				);

				template< typename, typename >
				static int test( ... );

			public:
				static const bool value = ( sizeof( test< T, AT_1 >( NULL ) ) == sizeof( char ) );
			};

			template< typename T >
			class is_constructible_impl< T, void, void, void, void >
			{
			private:
				template< typename T_T >
				static T_T testFun( T_T );

				template< typename T_T >
				static char test( typename enable_if< sizeof( T_T ) == sizeof( testFun( T_T() ) ) >::type* );

				template< typename >
				static int test( ... );

			public:
				static const bool value = ( sizeof( test< T >( NULL ) ) == sizeof( char ) );
			};
		}

		template< typename T, typename AT_1 = void, typename AT_2 = void, typename AT_3 = void, typename AT_4 = void >
		struct is_constructible : 
			public integral_constant< 
				bool, 
				detail::is_constructible_impl<T, AT_1, AT_2, AT_3, AT_4>::value
			> 
		{};

		/* Arrays miscellaneous */

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/rank
		 */

		template<class T>
		struct rank : public integral_constant<unsigned long, 0> {};

		template<class T>
		struct rank<T[]> : public integral_constant<unsigned long, rank<T>::value + 1> {};

		template<class T, unsigned long N>
		struct rank<T[N]> : public integral_constant<unsigned long, rank<T>::value + 1> {};

		/**
		 * Implementations of next classes were copied from:
		 * http://en.cppreference.com/w/cpp/types/extent
		 */

		template<class T, unsigned N = 0>
		struct extent : integral_constant<unsigned long, 0> {};

		template<class T>
		struct extent<T[], 0> : integral_constant<unsigned long, 0> {};

		template<class T, unsigned N>
		struct extent<T[], N> : extent<T, N - 1> {};

		template<class T, unsigned long I>
		struct extent<T[I], 0> : integral_constant<unsigned long, I> {};

		template<class T, unsigned long I, unsigned N>
		struct extent<T[I], N> : extent<T, N - 1> {};

	} // namespace pcpp::type_traits

} // namespace pcpp

#endif /* PCAPPP_TYPE_UTILS */