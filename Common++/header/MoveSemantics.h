#ifndef PCAPPP_MOVE_SEMANTICS
#define PCAPPP_MOVE_SEMANTICS

#include "CPP11.h"
#include "TypeUtils.h"

/// @file

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
		template < typename T >
		struct MovableTag { typedef T type; };

		template < typename T >
		struct NotCopyableTag { typedef T type; };

		/**
		 * @brief Represents an object to be copied.
		 * @tparam T Type to be proxied.
		 */
		template < typename T >
		struct CopyProxy
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

			/**
			 * @brief Main constructor.
			 * Sets up internal pointer to provided one.
			 * @param[in] ref Reference to object to be proxied.
			 */
			CopyProxy(const_reference ref) : obj(&ref) {}
			/**
			 * @brief Basic copy constructor.
			 * @param[in] other Instance to make copy of.
			 */
			CopyProxy(const CopyProxy& other) : obj(other.obj) {}
			/**
			 * @brief Basic copy assignment operator.
			 * @param[in] other Instance to make copy of.
			 */
			CopyProxy& operator=(const CopyProxy& other)
			{
				if (this == &other)
					return *this;
				obj = other.obj; 
				return *this; 
			}
			/**
			 * @brief Converts object to reference to handled object.
			 * @retur Reference to proxied object.
			 */
			reference get() const { return const_cast<reference>(*obj); }

			const_pointer obj; //!< Pointer to proxied object
		};

		namespace detail 
		{
			template < typename T >
			struct HasMovableTag_Helper
			{
				template<typename U, void (U::*)(MovableTag<U>) > struct SFINAE {};
				template<typename U> static char Test(SFINAE<U, &U::pcpp_declared_movable>*);
				template<typename U> static int Test(...);
				static const bool Has = sizeof(Test<T>(0)) == sizeof(char);
			};

			template < typename T >
			struct HasNotCopyableTag_Helper
			{
				template<typename U, void (U::*)(NotCopyableTag<U>) > struct SFINAE {};
				template<typename U> static char Test(SFINAE<U, &U::pcpp_declared_not_copyable>*);
				template<typename U> static int Test(...);
				static const bool Has = sizeof(Test<T>(0)) == sizeof(char);
			};

			template < typename T >
			struct HasMovableTag : 
				public ::pcpp::type_traits::integral_constant< bool, HasMovableTag_Helper< T >::Has >
			{};

			template < typename T >
			struct HasNotCopyableTag :
				public ::pcpp::type_traits::integral_constant< bool, HasNotCopyableTag_Helper< T >::Has >
			{};
		}

		/**
		 * @brief Assigns any CopyProxy object to provided object of any type.
		 * This overload is triggered for types that are declared movable with PCAPPP_DECLARE_MOVABLE and
		 * only when PCAPPP_ENABLE_MOVE_OPTIMIZATION is defined.
		 * @tparam T The type of provided object.
		 * @param ref Object to make CopyProxy from.
		 * @return CopyProxy that triggers the copy construction of new object.
		 */
		template < typename T >
		typename ::pcpp::type_traits::enable_if<
			::pcpp::type_traits::is_constructible<
				T,
				CopyProxy< const T >
			>::value,
			CopyProxy< const T >
		>::type copy(const T& ref) { return ref; }

		/**
		 * This overload triggers compile time error if not copyable type - one which
		 * declared with PCAPPP_DECLARE_NOT_COPYABLE, is copied via PCAPPP_COPY.
		 */
		template < typename T >
		typename ::pcpp::type_traits::enable_if<
			!::pcpp::type_traits::is_constructible<
				T,
				CopyProxy< const T >
			>::value &&
			detail::HasNotCopyableTag< T >::value
		>::type copy(const T& ref) {}

		/**
		 * This overload is triggered for simple types (any time) and types that are declared movable with PCAPPP_DECLARE_MOVABLE but
		 * only when PCAPPP_ENABLE_MOVE_OPTIMIZATION is not defined.
		 */
		template < typename T >
		typename ::pcpp::type_traits::enable_if<
			!::pcpp::type_traits::is_constructible<
				T,
				CopyProxy< const T >
			>::value &&
			!detail::HasNotCopyableTag< T >::value,
			const T&
		>::type copy(const T& ref) { return ref; }

	} // namespace pcpp::move_semantics

} // namespace pcpp

#ifndef ENABLE_CPP11_MOVE_SEMANTICS
#	define PCAPPP_DECLARE_MOVABLE(Type_Name) public: void pcpp_declared_movable(::pcpp::move_semantics::MovableTag<Type_Name>) {};

#	define PCAPPP_MOVE(Value) Value
#	define PCAPPP_MOVE_WITH_CAST(Type_To_Cast, Value) dynamic_cast<Type_To_Cast>(Value)

#	define PCAPPP_COPY(Value) ::pcpp::move_semantics::copy( Value )
#	define PCAPPP_COPY_WITH_CAST(Type_To_Cast, Value) ::pcpp::move_semantics::copy( dynamic_cast<Type_To_Cast>(Value) )

#	define PCAPPP_MOVE_OR_RVO(Value) PCAPPP_MOVE(Value)

#	define PCAPPP_MOVE_TYPE(Type_name) Type_name&
#	define PCAPPP_MOVE_OTHER other

#	define PCAPPP_MOVE_CONSTRUCTOR_NC(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) other)
#	define PCAPPP_MOVE_ASSIGNMENT_NC(Type_name) Type_name& operator=(PCAPPP_MOVE_TYPE(Type_name) other)

#	define PCAPPP_MOVE_CONSTRUCTOR_NC_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_TYPE(Type_name) other)
#	define PCAPPP_MOVE_ASSIGNMENT_NC_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_TYPE(Type_name) other)

#	define PCAPPP_COPY_CONSTRUCTOR(Type_name) Type_name(PCAPPP_COPY_TYPE(Type_name) other)
#	define PCAPPP_COPY_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_COPY_TYPE(Type_name) other)

#	define PCAPPP_COPY_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_COPY_TYPE(Type_name) other)
#	define PCAPPP_COPY_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_COPY_TYPE(Type_name) other)

#	ifdef PCAPPP_ENABLE_MOVE_OPTIMIZATION

#		define PCAPPP_COPY_TYPE(Type_name) ::pcpp::move_semantics::CopyProxy<const Type_name>

#		define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) other)
#		define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_TYPE(Type_name) other)

#		define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_TYPE(Type_name) other)
#		define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_TYPE(Type_name) other)

#		define PCAPPP_COPY_OTHER other.get()

#		define PCAPPP_DECLARE_NOT_COPYABLE(Type_Name) \
	public: \
		void pcpp_declared_not_copyable(::pcpp::move_semantics::NotCopyableTag<Type_Name>) {}; \
	private: \
		PCAPPP_COPY_CONSTRUCTOR(Type_Name) {} \
		PCAPPP_COPY_ASSIGNMENT(Type_Name) { return *this; } \
	public:

#	else

#		define PCAPPP_COPY_TYPE(Type_name) const Type_name&

#		define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) other, Type_name*)
#		define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& pcpp_suppress_move_assignment(PCAPPP_MOVE_TYPE(Type_name) other)

#		define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_TYPE(Type_name) other, Type_name*)
#		define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::pcpp_suppress_move_assignment(PCAPPP_MOVE_TYPE(Type_name) other)

#		define PCAPPP_COPY_OTHER other

#		define PCAPPP_DECLARE_NOT_COPYABLE(Type_Name) \
	public: \
		void pcpp_declared_not_copyable(::pcpp::move_semantics::NotCopyableTag<Type_Name>) {};

#	endif // PCAPPP_ENABLE_MOVE_OPTIMIZATION

#else

#include <utility>
#	define PCAPPP_DECLARE_MOVABLE(Type_Name) public:
#	define PCAPPP_DECLARE_NOT_COPYABLE(Type_Name) public:
/**
 * If move semantics is supported return std::move( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#	define PCAPPP_MOVE(Value_) ::std::move( Value_ )
/**
 * If move semantics is supported return std::move( Value_ ), 
 * otherwise return internal implementation ::pcpp::move_semantics::move( dynamic_cast<Type_to_cast>(Value_) )
 * Used in generated move constructors and assignment operators.
 */
#	define PCAPPP_MOVE_WITH_CAST(Type_to_cast, Value_) ::std::move( Value_ )

#	define PCAPPP_COPY(Value_) Value_

#	define PCAPPP_COPY_WITH_CAST(Type_To_Cast, Value_) Value_
/**
 * If move semantics is supported return Value_ , 
 * otherwise return internal implementation ::pcpp::move_semantics::move( Value_ )
 */
#	define PCAPPP_MOVE_OR_RVO(Value_) Value_
/**
 * Generates a type to be used as a move-reference.
 */
#	define PCAPPP_MOVE_TYPE(Type_name) Type_name&&

#	define PCAPPP_COPY_TYPE(Type_name) const Type_name&
/**
 * Returns name of variable declared as a parameter that represents other instance of same type 
 * in move-constructor or move-assignment operator generated using PCAPPP_MOVE_CONSTRUCTOR or 
 * PCAPPP_MOVE_ASSIGNMENT macro.
 */
#	define PCAPPP_MOVE_OTHER other

#	define PCAPPP_COPY_OTHER other
/**
 * Generates declaration of function parameter to be used by PCAPPP_MOVE_OTHER.
 */
#	define PCAPPP_MOVE_PARAMETER(Type_name) PCAPPP_MOVE_TYPE(Type_name) PCAPPP_MOVE_OTHER

#	define PCAPPP_COPY_PARAMETER(Type_name) PCAPPP_COPY_TYPE(Type_name) PCAPPP_COPY_OTHER
/**
 * Generates declaration of move-constructor with correct move-reference type.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_PARAMETER(Type_name))

#	define PCAPPP_COPY_CONSTRUCTOR(Type_name) Type_name(PCAPPP_COPY_PARAMETER(Type_name))
/**
 * Generates declaration of move-assignment operator with correct move-reference type.
 */
#	define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_PARAMETER(Type_name))

#	define PCAPPP_COPY_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_COPY_PARAMETER(Type_name))

/**
 * Generates definition of move-constructor with correct move-reference type.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_PARAMETER(Type_name))

#	define PCAPPP_COPY_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_COPY_PARAMETER(Type_name))
/**
 * Generates definition of move-assignment operator with correct move-reference type.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_PARAMETER(Type_name))

#	define PCAPPP_COPY_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_COPY_PARAMETER(Type_name))

#endif

#endif /* PCAPPP_MOVE_SEMANTICS */