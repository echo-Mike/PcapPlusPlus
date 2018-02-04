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
		/**
		 * @brief Special tag type for template dispatching with two specialisations.
		 * Represents true outcome.
		 */
		struct TrueType {};
		/**
		 * @brief Special tag type for template dispatching with two specialisations.
		 * Represents false outcome.
		 */
		struct FalseType {};

		/**
		 * Implementation of next classes are copied from:
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

	} // namespace pcpp::type_traits

} // namespace pcpp

#endif /* PCAPPP_TYPE_UTILS */