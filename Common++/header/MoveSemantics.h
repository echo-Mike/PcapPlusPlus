#ifndef PCAPPP_MOVE_SEMANTICS
#define PCAPPP_MOVE_SEMANTICS

#include "CPP11.h"
#include "TypeUtils.h"
#include <iostream>

/// @file

#ifndef ENABLE_CPP11_MOVE_SEMANTICS
/**
 * \namespace pcpp
 * \brief The main namespace for the PcapPlusPlus lib
 */
namespace pcpp
{
	/**
	 * \namespace move_semantics
	 * \brief Namespace that contains implementation of C++11 move semantics
	 */
	namespace move_semantics
	{
		/**
		 * @brief Rrepresents rvalue refernce.
		 * @tparam T Type to be proxied.
		 */
		template < typename T >
		struct MoveProxy
		{
			/**
			 * Type to be proxied.
			 */
			typedef T value_type;
			/**
			 * Type of reference to proxied type.
			 */
			typedef typename ::pcpp::type_traits::remove_reference<T>::type& reference;

			reference ref;
			/**
			 * @brief Main constructor.
			 * Sets up internal reference to provided one.
			 * @param ref Referenceto object to be proxied.
			 */
			MoveProxy(reference ref_) : ref(ref_) {}
			/**
			 * @brief Basic copy constructor.
			 * @param other Instance to make copy of.
			 */
			MoveProxy(const MoveProxy& other) : ref(other.ref) {}
			/**
			 * @brief Basic copy assignment operator.
			 * @param other Instance to make copy of.
			 */
			MoveProxy& operator=(const MoveProxy& other) { ref = other.ref; return *this; }
		};

		/**
		 * @brief Assigns any MoveProxy object to provided object of any type.
		 * This function serve similar purpose as std::move.
		 * @tparam T The type of provided object.
		 * @param ref Object to make MoveProxy from.
		 * @return MoveProxy that represents rvalue reference to proveded object.
		 */
		template < typename T >
		MoveProxy<T> move(T& ref) { return ref; }

		/**
		 * @brief This function serve similar purpose as std::forward.
		 */
		template < typename T >
		T& forward(T& ref) { return ref; }

	} // namespace pcpp::move_semantics

} // namespace pcpp

/**
 * If move semantics is supported return std::move( Value_ ), otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#define PCAPPP_MOVE(Value_) ::pcpp::move_semantics::move( Value_ )
/**
 * If move semantics is supported return Value_ , otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#define PCAPPP_MOVE_OR_RVO(Value_) ::pcpp::move_semantics::move( Value_ )
/**
* If move semantics is supported return std::forward( Value_ ), otherwise return internal implementation ::pcpp::move_semantics::forward( Value_ )
 */
#define PCAPPP_FORWARD(Value_) ::pcpp::move_semantics::forward( Value_ )

#else

#include <utility>
 /**
  * If move semantics is supported return std::move( Value_ ), otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
  */
#define PCAPPP_MOVE(Value_) ::std::move( Value_ )
/**
 * If move semantics is supported return Value_ , otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#define PCAPPP_MOVE_OR_RVO(Value_) Value_
 /**
  * If move semantics is supported return std::forward( Value_ ), otherwise return internal implementation ::pcpp::move_semantics::forward( Value_ )
  */
#define PCAPPP_FORWARD(Value_) ::std::forward( Value_ )

#endif

#endif /* PCAPPP_MOVE_SEMANTICS */