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
			 * Real type to be proxied.
			 */
			typedef typename ::pcpp::type_traits::remove_cv< typename ::pcpp::type_traits::remove_reference<T>::type >::type real_type;
			/**
			 * Type of reference to proxied type.
			 */
			typedef real_type& reference;
			/**
			 * Type of const reference to proxied type.
			 */
			typedef const real_type& const_reference;
			/**
			 * Type of pointer to proxied type.
			 */
			typedef real_type* pointer;
			/**
			 * Type of const pointer to proxied type.
			 */
			typedef const real_type* const_pointer;

			const_pointer ref;
			/**
			 * @brief Main constructor.
			 * Sets up internal reference to provided one.
			 * @param[in] ref Reference to object to be proxied.
			 */
			MoveProxy(const_reference ref_) : ref(&ref_) {}
			/**
			 * @brief Basic copy constructor.
			 * @param[in] other Instance to make copy of.
			 */
			MoveProxy(const MoveProxy& other) : ref(other.ref) {}
			/**
			 * @brief Basic copy assignment operator.
			 * @param[in] other Instance to make copy of.
			 */
			MoveProxy& operator=(const MoveProxy& other) { ref = other.ref; return *this; }
			/**
			 * @brief Converts object to handled reference.
			 * @retur Reference to proxied object.
			 */
			reference get() const { return const_cast<reference>(*ref); }
			/**
			 * @brief Converts object to handled reference.
			 * This function is called by compiler when object is moved but it have no special constructor that
			 * implements our move semantics. The object is copied in that case.
			 * @retur Reference to proxied object.
			 */
			//operator reference() const { return const_cast<reference>(*ref); }
		};

		/**
		 * @brief Assigns any MoveProxy object to provided object of any type.
		 * This function serve similar purpose as std::move.
		 * @tparam T The type of provided object.
		 * @param ref Object to make MoveProxy from.
		 * @return MoveProxy that represents rvalue reference to provided object.
		 */
		template < typename T >
		typename ::pcpp::type_traits::enable_if<
			::pcpp::type_traits::is_constructible<
				T, 
				const MoveProxy<const T>
			>::value,
			const MoveProxy<const T>
		>::type move(const T& ref) { return ref; }
		
		template < typename T >
		typename ::pcpp::type_traits::enable_if<
			!::pcpp::type_traits::is_constructible<
				T, 
				const MoveProxy<const T>
			>::value,
			const T&
		>::type move(const T& ref) { return ref; }

		/**
		 * @brief This function serve similar purpose as std::forward.
		 */
		template < typename T >
		T& forward(T& ref) { return ref; }

	} // namespace pcpp::move_semantics

} // namespace pcpp

/**
 * If move semantics is supported return std::move( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#define PCAPPP_MOVE(Value_) ::pcpp::move_semantics::move( Value_ )
/**
 * If move semantics is supported return std::move( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::move( dynamic_cast<Type_to_cast>(Value_) )
 * Used in generated move constructors and assignment operators.
 */
#define PCAPPP_MOVE_WITH_CAST(Type_to_cast, Value_) ::pcpp::move_semantics::move( dynamic_cast<Type_to_cast>(Value_) )
/**
 * If move semantics is supported return Value_ , 
 * otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#define PCAPPP_MOVE_OR_RVO(Value_) { ::pcpp::move_semantics::move( Value_ ) }
/**
 * If move semantics is supported return std::forward( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::forward( Value_ )
 */
#define PCAPPP_FORWARD(Value_) ::pcpp::move_semantics::forward( Value_ )
/**
 * Generates a type to be used as a move-reference.
 */
#define PCAPPP_MOVE_TYPE(Type_name) const ::pcpp::move_semantics::MoveProxy<const Type_name>
/**
 * Returns name of variable declared as a parameter that represents other instance of same type 
 * in move-constructor or move-assignment operator generated using PCAPPP_MOVE_CONSTRUCTOR or 
 * PCAPPP_MOVE_ASSIGNMENT macro.
 */
#define PCAPPP_MOVE_OTHER proxy.get()
/**
 * Generates declaration of function parameter to be used by PCAPPP_MOVE_OTHER.
 */
#define PCAPPP_MOVE_PARAMETER(Type_name) PCAPPP_MOVE_TYPE(Type_name) proxy, const Type_name*

/**
 * Generates declaration of move-constructor with correct move-reference type.
 */
#define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) proxy)
/**
 * Generates declaration of move-assignment operator with correct move-reference type.
 */
#define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_TYPE(Type_name) proxy)

/**
 * Generates definition of move-constructor with correct move-reference type.
 * Use this macro in .cpp files.
 */
#define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_TYPE(Type_name) proxy)
/**
 * Generates definition of move-assignment operator with correct move-reference type.
 * Use this macro in .cpp files.
 */
#define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_TYPE(Type_name) proxy)

#else

#include <utility>
/**
 * If move semantics is supported return std::move( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#define PCAPPP_MOVE(Value_) ::std::move( Value_ )
/**
 * If move semantics is supported return std::move( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::move( dynamic_cast<Type_to_cast>(Value_) )
 * Used in generated move constructors and assignment operators.
 */
#define PCAPPP_MOVE_WITH_CAST(Type_to_cast, Value_) ::std::move( Value_ )
/**
 * If move semantics is supported return Value_ , 
 * otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#define PCAPPP_MOVE_OR_RVO(Value_) Value_
/**
 * If move semantics is supported return std::forward( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::forward( Value_ )
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
 * Generates declaration of function parameter to be used by PCAPPP_MOVE_OTHER.
 */
#define PCAPPP_MOVE_PARAMETER(Type_name) PCAPPP_MOVE_TYPE(Type_name) PCAPPP_MOVE_OTHER

/**
 * Generates declaration of move-constructor with correct move-reference type.
 */
#define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_PARAMETER(Type_name))
/**
 * Generates declaration of move-assignment operator with correct move-reference type.
 */
#define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_PARAMETER(Type_name))

/**
 * Generates definition of move-constructor with correct move-reference type.
 * Use this macro in .cpp files.
 */
#define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_PARAMETER(Type_name))
/**
 * Generates definition of move-assignment operator with correct move-reference type.
 * Use this macro in .cpp files.
 */
#define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_PARAMETER(Type_name))

#endif

#endif /* PCAPPP_MOVE_SEMANTICS */