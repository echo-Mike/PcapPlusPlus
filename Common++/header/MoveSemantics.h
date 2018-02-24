#ifndef PCAPPP_MOVE_SEMANTICS
#define PCAPPP_MOVE_SEMANTICS

#include "CPP11.h"
#include "TypeUtils.h"

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
		 * @brief Represents rvalue reference.
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
			 * @param ref Reference to object to be proxied.
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
			/**
			 * @brief Converts object to handled reference.
			 * This function is called by compiler when object is moved but it have no special constructor that
			 * implements our move semantics. The object is copied in that case.
			 * @retur Reference to proxied object.
			 */
			operator reference() const { return ref; }
		};

		/**
		 * @brief Assigns any MoveProxy object to provided object of any type.
		 * This function serve similar purpose as std::move.
		 * @tparam T The type of provided object.
		 * @param ref Object to make MoveProxy from.
		 * @return MoveProxy that represents rvalue reference to provided object.
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
/**
 * Generates a type to be used as a move-reference.
 */
#define PCAPPP_MOVE_TYPE(Type_name) ::pcpp::move_semantics::MoveProxy<Type_name>
/**
 * Returns name of variable declared as a parameter that represents other instance of same type 
 * in move-constructor or move-assignment operator generated using PCAPPP_MOVE_CONSTRUCTOR or 
 * PCAPPP_MOVE_ASSIGNMENT macro.
 */
#define PCAPPP_MOVE_OTHER proxy.ref
/**
 * Generates declaration of move-constructor with correct move-reference type.
 */
#define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) proxy)
/**
 * Generates declaration of move-assignment operator with correct move-reference type.
 */
#define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_TYPE(Type_name) proxy)

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
/**
 * Generates a type to be used as a move-reference.
 */
#define PCAPPP_MOVE_TYPE(Type_name) Type_name&&
/**
 * Returns name of variable declared as a parameter that represents other instance of same type 
 * in move-constructor or move-assignment operator generated using PCAPPP_MOVE_CONSTRUCTOR or 
 * PCAPPP_MOVE_ASSIGNMENT macro.
 */
#define PCAPPP_MOVE_OTHER other
/**
 * Generates declaration of move-constructor with correct move-reference type.
 */
#define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) other)
/**
 * Generates declaration of move-assignment operator with correct move-reference type.
 */
#define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_TYPE(Type_name) other)

#endif

#endif /* PCAPPP_MOVE_SEMANTICS */