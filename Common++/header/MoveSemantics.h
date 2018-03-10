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
		/**
		 * @brief Special tag type that used to tag classes and structures that are declared movable.
		 * @tparam T Type to declare movable.
		 */
		template < typename T >
		struct MovableTag { typedef T type; };
		/**
		 * @brief Special tag type that used to tag classes and structures that are declared not copyable.
		 * @tparam T Type to declare not copyable.
		 */
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
			 * Sets up internal pointer to point to provided object.
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
			 * @brief Enables automatic conversion between proxy objects that proxies classes from one hierarchy.
			 * Converts from proxy of derived class to proxy of base class.
			 * @param[in] other Instance to convert.
			 */
			template < typename U >
			CopyProxy(
				const CopyProxy< U >& other, 
				typename pcpp::type_traits::enable_if<
					pcpp::type_traits::is_base_of< T, U >::value
				>::type* = PCAPPP_NULLPTR
			) : obj(dynamic_cast<const_pointer>(other.obj)) {}

			/**
			 * @brief Converts object to reference to handled object.
			 * @retur Reference to proxied object.
			 */
			reference get() const { return const_cast<reference>(*obj); }

			const_pointer obj; //!< Pointer to proxied object
		};

		/**
		 * \namespace detail
		 * \brief Namespace that contains implementation details of implementation of C++11 move semantics
		 */
		namespace detail 
		{
			template < typename T >
			struct DeclaredMovable_Helper
			{
				template<typename U, void (U::*)(MovableTag<U>) > struct SFINAE {};
				template<typename U> static char Test(SFINAE<U, &U::pcpp_declared_movable>*);
				template<typename U> static int Test(...);
				static const bool value = sizeof(Test<T>(0)) == sizeof(char);
			};

			template < typename T >
			struct DeclaredNotCopyable_Helper
			{
				template<typename U, void (U::*)(NotCopyableTag<U>) > struct SFINAE {};
				template<typename U> static char Test(SFINAE<U, &U::pcpp_declared_not_copyable>*);
				template<typename U> static int Test(...);
				static const bool value = sizeof(Test<T>(0)) == sizeof(char);
			};
		}

		/**
		 * @brief Special trait that have compile-time boolean member value that is true if provided
		 * type T was declared movable via PCAPPP_DECLARE_MOVABLE, and false otherwise.
		 * @tparam T Type to check.
		 */
		template < typename T >
		struct DeclaredMovable : 
			public ::pcpp::type_traits::integral_constant< bool, detail::DeclaredMovable_Helper< T >::value >
		{};
		/**
		 * @brief Special trait that have compile-time boolean member value that is true if provided
		 * type T was declared not copyable via PCAPPP_DECLARE_NOT_COPYABLE, and false otherwise.
		 * @tparam T Type to check.
		 */
		template < typename T >
		struct DeclaredNotCopyable :
			public ::pcpp::type_traits::integral_constant< bool, detail::DeclaredNotCopyable_Helper< T >::value >
		{};

		/**
		 * @brief Assigns a CopyProxy object to provided object of any type.
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
		 * @tparam T The type of provided object.
		 */
		template < typename T >
		typename ::pcpp::type_traits::enable_if<
			!::pcpp::type_traits::is_constructible<
				T,
				CopyProxy< const T >
			>::value &&
			DeclaredNotCopyable< T >::value
		>::type copy(const T&) {}

		/**
		 * This overload is triggered for simple types (any time) and for types that are declared movable with PCAPPP_DECLARE_MOVABLE but
		 * only when PCAPPP_ENABLE_MOVE_OPTIMIZATION is not defined.
		 * @tparam T The type of provided object.
		 * @return Provided reference.
		 */
		template < typename T >
		typename ::pcpp::type_traits::enable_if<
			!::pcpp::type_traits::is_constructible<
				T,
				CopyProxy< const T >
			>::value &&
			!DeclaredNotCopyable< T >::value,
			const T&
		>::type copy(const T& ref) { return ref; }

	} // namespace pcpp::move_semantics

} // namespace pcpp

/**
 * Declares provided type movable for PcapPlusPlus
 * Must be used in body of class declaration.
 * After line where this macro is used the public member access begins.
 */
#define PCAPPP_DECLARE_MOVABLE(Type_Name) public: void pcpp_declared_movable(::pcpp::move_semantics::MovableTag<Type_Name>) {};

#ifndef ENABLE_CPP11_MOVE_SEMANTICS
/**
 * Macro that defines the move operation over provided value.
 */
#	define PCAPPP_MOVE(Value) Value
/**
 * Macro that defines the move with cast operation over provided value.
 * Cast is made first.
 * This macro is used in body of derived class move constructors and assignment operators.
 * It properly casts provided object to base class.
 */
#	define PCAPPP_MOVE_WITH_CAST(Type_To_Cast, Value) dynamic_cast<Type_To_Cast>(Value)

/**
 * Macro that defines the copy operation over provided value.
 */
#	define PCAPPP_COPY(Value) ::pcpp::move_semantics::copy( Value )
/**
 * Macro that defines the copy with cast operation over provided value.
 * Cast is made first.
 * This macro is used in body of derived class copy constructors and assignment operators.
 * It properly casts provided object to base class.
 */
#	define PCAPPP_COPY_WITH_CAST(Type_To_Cast, Value) ::pcpp::move_semantics::copy( dynamic_cast<Type_To_Cast>(Value) )

/**
 * Macro that properly defines move operation when movable or not copyable object is returned from function.
 */
#	define PCAPPP_MOVE_OR_RVO(Value) PCAPPP_MOVE(Value)

/**
 * Internal macro. NOT TO BE USED ANYWHERE!!!
 */
#	define PCAPPP_MOVE_TYPE(Type_name) const Type_name&

#	define PCAPPP_PREPARE_MOVE_OTHER_I(Type_name) Type_name& other_ = const_cast<Type_name&>(other);

/**
 * Returns reference to other instance of same type in move-constructor or 
 * move-assignment operator that was generated using PCAPPP_MOVE_CONSTRUCTOR, 
 * PCAPPP_MOVE_ASSIGNMENT, PCAPPP_MOVE_CONSTRUCTOR_NC, PCAPPP_MOVE_ASSIGNMENT_NC macro.
 */
#	define PCAPPP_MOVE_OTHER_O other
#	define PCAPPP_MOVE_OTHER_I other_

/**
 * Generates declaration of move-constructor with correct move-reference type for NOT COPYABLE (_NC) objects.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR_NC(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) PCAPPP_MOVE_OTHER_O)
/**
 * Generates declaration of move-assignment operator with correct move-reference type for NOT COPYABLE (_NC) objects.
 */
#	define PCAPPP_MOVE_ASSIGNMENT_NC(Type_name) Type_name& operator=(PCAPPP_MOVE_TYPE(Type_name) PCAPPP_MOVE_OTHER_O)

/**
 * Generates definition of move-constructor with correct move-reference type for NOT COPYABLE (_NC) objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR_IMPL_NC(Type_name) Type_name::Type_name(PCAPPP_MOVE_TYPE(Type_name) PCAPPP_MOVE_OTHER_O)
/**
 * Generates definition of move-assignment operator with correct move-reference type for NOT COPYABLE (_NC) objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_ASSIGNMENT_IMPL_NC(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_TYPE(Type_name) PCAPPP_MOVE_OTHER_O)

/**
 * Generates declaration of copy-constructor with correct copy-reference type for MOVABLE objects.
 */
#	define PCAPPP_COPY_CONSTRUCTOR(Type_name) Type_name(PCAPPP_COPY_TYPE(Type_name) PCAPPP_MOVE_OTHER_O)
/**
 * Generates declaration of copy-assignment operator with correct copy-reference type for MOVABLE objects.
 */
#	define PCAPPP_COPY_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_COPY_TYPE(Type_name) PCAPPP_MOVE_OTHER_O)

/**
 * Generates definition of copy-constructor with correct copy-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_COPY_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_COPY_TYPE(Type_name) other)
/**
 * Generates definition of copy-assignment operator with correct copy-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_COPY_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_COPY_TYPE(Type_name) other)

#	ifdef PCAPPP_ENABLE_MOVE_OPTIMIZATION

/**
 * Internal macro. NOT TO BE USED ANYWHERE!!!
 */
#		define PCAPPP_COPY_TYPE(Type_name) ::pcpp::move_semantics::CopyProxy<const Type_name>

/**
 * Generates declaration of move-constructor with correct move-reference type for MOVABLE objects.
 */
#		define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) other)
/**
 * Generates declaration of move-assignment operator with correct move-reference type for MOVABLE objects.
 */
#		define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_TYPE(Type_name) other)

/**
 * Generates definition of move-constructor with correct move-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#		define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_TYPE(Type_name) other)
/**
 * Generates definition of move-assignment operator with correct move-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#		define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_TYPE(Type_name) other)

/**
 * Returns reference to other instance of same type in copy-constructor or 
 * copy-assignment operator that was generated using PCAPPP_COPY_CONSTRUCTOR or 
 * PCAPPP_COPY_ASSIGNMENT macro.
 */
#		define PCAPPP_COPY_OTHER other.get()

/**
 * Declares provided type not copyable for PcapPlusPlus
 * Must be used in body of class declaration.
 * After line where this macro is used the public member access begins.
 */
#		define PCAPPP_DECLARE_NOT_COPYABLE(Type_Name) \
	public: \
		void pcpp_declared_not_copyable(::pcpp::move_semantics::NotCopyableTag<Type_Name>) {}; \
	private: \
		PCAPPP_COPY_CONSTRUCTOR(Type_Name) {} \
		PCAPPP_COPY_ASSIGNMENT(Type_Name) { return *this; } \
	public:

#	else // PCAPPP_ENABLE_MOVE_OPTIMIZATION

/**
 * Internal macro. NOT TO BE USED ANYWHERE!!!
 */
#		define PCAPPP_COPY_TYPE(Type_name) const Type_name&

/**
 * Generates declaration of move-constructor with correct move-reference type for MOVABLE objects.
 * This macro declares suppressed move-constructor when PCAPPP_ENABLE_MOVE_OPTIMIZATION is not defined.
 */
#		define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_TYPE(Type_name) other, Type_name*)
/**
 * Generates declaration of move-assignment operator with correct move-reference type for MOVABLE objects.
 * This macro declares suppressed move-assignment operator when PCAPPP_ENABLE_MOVE_OPTIMIZATION is not defined.
 */
#		define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& pcpp_suppress_move_assignment(PCAPPP_MOVE_TYPE(Type_name) other)

/**
 * Generates definition of move-constructor with correct move-reference type for MOVABLE objects.
 * This macro defines suppressed move-constructor when PCAPPP_ENABLE_MOVE_OPTIMIZATION is not defined.
 * Use this macro in .cpp files.
 */
#		define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_TYPE(Type_name) other, Type_name*)
/**
 * Generates definition of move-assignment operator with correct move-reference type for MOVABLE objects.
 * This macro defines suppressed move-assignment operator when PCAPPP_ENABLE_MOVE_OPTIMIZATION is not defined.
 * Use this macro in .cpp files.
 */
#		define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::pcpp_suppress_move_assignment(PCAPPP_MOVE_TYPE(Type_name) other)

/**
 * Returns reference to other instance of same type in copy-constructor or 
 * copy-assignment operator that was generated using PCAPPP_COPY_CONSTRUCTOR or 
 * PCAPPP_COPY_ASSIGNMENT macro.
 */
#		define PCAPPP_COPY_OTHER other

/**
 * Declares provided type not copyable for PcapPlusPlus
 * Must be used in body of class declaration.
 * After line where this macro is used the public member access begins.
 */
#		define PCAPPP_DECLARE_NOT_COPYABLE(Type_Name) \
	public: \
		void pcpp_declared_not_copyable(::pcpp::move_semantics::NotCopyableTag<Type_Name>) {};

#	endif // PCAPPP_ENABLE_MOVE_OPTIMIZATION

#else // ENABLE_CPP11_MOVE_SEMANTICS
// Enables std::move
#	include <utility>

/**
 * Declares provided type not copyable for PcapPlusPlus
 * Must be used in body of class declaration.
 * After line where this macro is used the public member access begins.
 */
#	define PCAPPP_DECLARE_NOT_COPYABLE(Type_Name) \
	public: \
		void pcpp_declared_not_copyable(::pcpp::move_semantics::NotCopyableTag<Type_Name>) {}; \
	private: \
		Type_Name(const Type_Name&) {} \
		Type_Name& operator=(const Type_Name&) { return *this; } \
	public:

/**
 * Macro that defines the move operation over provided value.
 */
#	define PCAPPP_MOVE(Value_) ::std::move( Value_ )
/**
 * Macro that defines the move with cast operation over provided value.
 * Cast is made first.
 * This macro is used in body of derived class move constructors and assignment operators.
 * It properly casts provided object to base class.
 */
#	define PCAPPP_MOVE_WITH_CAST(Type_to_cast, Value_) ::std::move( Value_ )

/**
 * Macro that defines the copy operation over provided value.
 */
#	define PCAPPP_COPY(Value_) Value_
/**
 * Macro that defines the copy with cast operation over provided value.
 * Cast is made first.
 * This macro is used in body of derived class copy constructors and assignment operators.
 * It properly casts provided object to base class.
 */
#	define PCAPPP_COPY_WITH_CAST(Type_To_Cast, Value_) Value_

/**
 * Macro that properly defines move operation when movable or not copyable object is returned from function.
 */
#	define PCAPPP_MOVE_OR_RVO(Value_) Value_

/**
 * Internal macro. NOT TO BE USED ANYWHERE!!!
 */
#	define PCAPPP_MOVE_TYPE(Type_name) Type_name&&
/**
 * Internal macro. NOT TO BE USED ANYWHERE!!!
 */
#	define PCAPPP_COPY_TYPE(Type_name) const Type_name&

#	define PCAPPP_PREPARE_MOVE_OTHER_I(Type_name)

 /**
 * Returns reference to other instance of same type in move-constructor or 
 * move-assignment operator that was generated using PCAPPP_MOVE_CONSTRUCTOR, 
 * PCAPPP_MOVE_ASSIGNMENT, PCAPPP_MOVE_CONSTRUCTOR_NC, PCAPPP_MOVE_ASSIGNMENT_NC macro.
 */
#	define PCAPPP_MOVE_OTHER_O other
#	define PCAPPP_MOVE_OTHER_I PCAPPP_MOVE_OTHER_O
/**
 * Returns reference to other instance of same type in copy-constructor or 
 * copy-assignment operator that was generated using PCAPPP_COPY_CONSTRUCTOR or 
 * PCAPPP_COPY_ASSIGNMENT macro.
 */
#	define PCAPPP_COPY_OTHER other

/**
 * Internal macro. NOT TO BE USED ANYWHERE!!!
 */
#	define PCAPPP_MOVE_PARAMETER(Type_name) PCAPPP_MOVE_TYPE(Type_name) PCAPPP_MOVE_OTHER_O
/**
 * Internal macro. NOT TO BE USED ANYWHERE!!!
 */
#	define PCAPPP_COPY_PARAMETER(Type_name) PCAPPP_COPY_TYPE(Type_name) PCAPPP_COPY_OTHER

/**
 * Generates declaration of move-constructor with correct move-reference type for MOVABLE objects.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR(Type_name) Type_name(PCAPPP_MOVE_PARAMETER(Type_name))
/**
 * Generates declaration of move-constructor with correct move-reference type for NOT COPYABLE (_NC) objects.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR_NC(Type_name) PCAPPP_MOVE_CONSTRUCTOR(Type_name)

/**
 * Generates declaration of copy-constructor with correct copy-reference type for MOVABLE objects.
 */
#	define PCAPPP_COPY_CONSTRUCTOR(Type_name) Type_name(PCAPPP_COPY_PARAMETER(Type_name))

/**
 * Generates declaration of move-assignment operator with correct move-reference type for MOVABLE objects.
 */
#	define PCAPPP_MOVE_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_MOVE_PARAMETER(Type_name))
/**
 * Generates declaration of move-assignment operator with correct move-reference type for NOT COPYABLE (_NC) objects.
 */
#	define PCAPPP_MOVE_ASSIGNMENT_NC(Type_name) PCAPPP_MOVE_ASSIGNMENT(Type_name)

/**
 * Generates declaration of copy-assignment operator with correct copy-reference type for MOVABLE objects.
 */
#	define PCAPPP_COPY_ASSIGNMENT(Type_name) Type_name& operator=(PCAPPP_COPY_PARAMETER(Type_name))

/**
 * Generates definition of move-constructor with correct move-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_MOVE_PARAMETER(Type_name))
/**
 * Generates definition of move-constructor with correct move-reference type for NOT COPYABLE (_NC) objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_CONSTRUCTOR_IMPL_NC(Type_name) PCAPPP_MOVE_CONSTRUCTOR_IMPL(Type_name)

/**
 * Generates definition of copy-constructor with correct copy-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_COPY_CONSTRUCTOR_IMPL(Type_name) Type_name::Type_name(PCAPPP_COPY_PARAMETER(Type_name))

/**
 * Generates definition of move-assignment operator with correct move-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_MOVE_PARAMETER(Type_name))
/**
 * Generates definition of move-assignment operator with correct move-reference type for NOT COPYABLE (_NC) objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_MOVE_ASSIGNMENT_IMPL_NC(Type_name) PCAPPP_MOVE_ASSIGNMENT_IMPL(Type_name)

/**
 * Generates definition of copy-assignment operator with correct copy-reference type for MOVABLE objects.
 * Use this macro in .cpp files.
 */
#	define PCAPPP_COPY_ASSIGNMENT_IMPL(Type_name) Type_name& Type_name::operator=(PCAPPP_COPY_PARAMETER(Type_name))

#endif // ENABLE_CPP11_MOVE_SEMANTICS

#endif /* PCAPPP_MOVE_SEMANTICS */