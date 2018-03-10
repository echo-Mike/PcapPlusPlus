#ifndef PCAPPP_UNIQUE_PTR_H
#define PCAPPP_UNIQUE_PTR_H

#include "CPP11.h"
#include "MacroUtils.h"
#include "MemoryUtils.h"
#include "CompressedPair.h"
#include "MoveSemantics.h"

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
		 * @brief Base class for unique_ptr implementation.
		 * @tparam T The type of values to be stored.
		 * @tparam Deleter The function object or lvalue reference to function or to function object, to be called from the destructor.
		 */
		template < typename T, typename Deleter >
		class unique_ptr_base
		{
		public:
			/**
			 * Type of pointer to stored value.
			 */
			typedef T* pointer;
			/**
			 * Type of stored value.
			 */
			typedef T element_type;
			/**
			 * Type of deleter.
			 */
			typedef Deleter deleter_type;

			/**
			 * @brief Default constructor.
			 * Constructs a unique_ptr_base that owns nothing. 
			 * Value-initializes the stored pointer and the stored deleter. 
			 * Requires that Deleter is DefaultConstructible.
			 */
			unique_ptr_base() : m_Pair(Deleter(), pointer()) {}

#ifdef PCAPPP_HAVE_NULLPTR_T
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			unique_ptr_base(std::nullptr_t) : m_Pair(Deleter(), pointer()) {}
#endif
			/**
			 * @brief Main constructor.
			 * Constructs a unique_ptr_base which owns p, initializing the stored pointer with p 
			 * and value-initializing the stored deleter. 
			 * Requires that Deleter is DefaultConstructible.
			 * @param[in] p Pointer to memory to be owned.
			 */
			explicit unique_ptr_base(pointer p) : m_Pair(Deleter(), p) {}

			///@cond
			// Doxygen do not generate proper documentation for functions defined by macro.

			PCAPPP_DECLARE_NOT_COPYABLE(unique_ptr_base)

			/**
			 * @brief Move constructor.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_CONSTRUCTOR_NC(unique_ptr_base) :
				m_Pair(PCAPPP_MOVE(PCAPPP_MOVE_OTHER_O.m_Pair))
			{
				PCAPPP_PREPARE_MOVE_OTHER_I(unique_ptr_base)
				// Nullify provided object
				PCAPPP_MOVE_OTHER_I.m_Pair.get_first() = Deleter();
				PCAPPP_MOVE_OTHER_I.m_Pair.get_second() = pointer();
			}

			/**
			 * @brief Move assignment operator.
			 * Don't allows self assignment.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_ASSIGNMENT_NC(unique_ptr_base)
			{
				PCAPPP_PREPARE_MOVE_OTHER_I(unique_ptr_base)
				// Handle self assignment case
				if (this == &PCAPPP_MOVE_OTHER_I)
					return *this;
				m_Pair = PCAPPP_MOVE(PCAPPP_MOVE_OTHER_I.m_Pair);
				// Nullify provided object
				PCAPPP_MOVE_OTHER_I.m_Pair.get_first() = Deleter();
				PCAPPP_MOVE_OTHER_I.m_Pair.get_second() = pointer();
				return *this;
			}

			///@endcond

			/**
			 * @brief Method to access the stored deleter object.
			 * @return Reference to the deleter object.
			 */
			deleter_type& get_deleter()	{ return (m_Pair.get_first()); }
			/**
			 * @brief Method to access the stored deleter object.
			 * This overload is selected by compiler if object is const-qualified.
			 * @return Reference to the deleter object.
			 */
			const deleter_type& get_deleter() const { return (m_Pair.get_first()); }

			/**
			 * @brief Method to access the stored pointer.
			 * @return Reference to the pointer.
			 */
			pointer& get_pointer() { return (m_Pair.get_second()); }
			/**
			 * @brief Method to access the stored pointer.
			 * This overload is selected by compiler if object is const-qualified.
			 * @return Reference to the pointer.
			 */
			const pointer& get_pointer() const { return (m_Pair.get_second()); }

			/**
			 * Pair that stores deleter and pointer compressed if it is possible.
			 */
			Implementation::CompressedPair<Deleter, pointer> m_Pair;
		};

		/**
		 * @brief Library implementation of std::unique_ptr.
		 * Read more: http://en.cppreference.com/w/cpp/memory/unique_ptr \n
		 * Does not have the swap function, some constructors and assignment operators.
		 * @tparam T The type of values to be stored.
		 * @tparam Deleter The function object or lvalue reference to function or to function object, to be called from the destructor.
		 */
		template < typename T, typename Deleter = default_delete<T> >
		class unique_ptr :
			public unique_ptr_base<T, Deleter>
		{
		public:
			/**
			 * Base type that defines basic interface.
			 */
			typedef unique_ptr_base<T, Deleter> Base;
			/**
			 * Propagation of pointer to stored value type.
			 * Required by standard.
			 */
			typedef typename Base::pointer pointer;
			/**
			 * Represents the type of stored value.
			 * Required by standard.
			 */
			typedef T element_type;
			/**
			 * Represents the type of deleter.
			 * Required by standard.
			 */
			typedef Deleter deleter_type;
			/**
			 * Propagation of interface defined in base class.
			 */
			using Base::get_deleter;

			/**
			 * @brief Default constructor.
			 * Constructs a unique_ptr that owns nothing. 
			 * Value-initializes the stored pointer and the stored deleter. 
			 * Requires that Deleter is DefaultConstructible.
			 */
			unique_ptr() : Base() {}

#ifdef PCAPPP_HAVE_NULLPTR_T
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			unique_ptr(std::nullptr_t) : Base(PCAPPP_NULLPTR) {}
#endif
			/**
			 * @brief Main constructor.
			 * Constructs a unique_ptr which owns p, initializing the stored pointer with p 
			 * and value-initializing the stored deleter. 
			 * Requires that Deleter is DefaultConstructible.
			 * @param[in] p Pointer to memory to be owned.
			 */
			explicit unique_ptr(pointer p) : Base(p) {}

			///@cond
			// Doxygen do not generate proper documentation for functions defined by macro.

			PCAPPP_DECLARE_NOT_COPYABLE(unique_ptr)

			/**
			 * @brief Move constructor.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_CONSTRUCTOR_NC(unique_ptr) :
				Base(PCAPPP_MOVE_WITH_CAST(const Base&, PCAPPP_MOVE_OTHER_O)) {}

			/**
			 * @brief Move assignment operator.
			 * Don't allows self assignment.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_ASSIGNMENT_NC(unique_ptr)
			{
				// Handle self assignment case
				if (this == &PCAPPP_MOVE_OTHER_O)
					return *this;
				Base::operator=(PCAPPP_MOVE_WITH_CAST(const Base&, PCAPPP_MOVE_OTHER_O));
				return *this;
			}

			///@endcond

			/**
			 * @brief Destructor.
			 * Deallocates memory via calling the provided deleter with internal pointer as an argument.
			 */
			~unique_ptr()
			{
				if (get() != pointer())
					this->get_deleter()(get());
			}

			/**
			 * @brief Method that provides access to stored object by reference.
			 * @return Reference to stored object.
			 */
			element_type& operator*() const { return (*get()); }

			/**
			 * @brief Method that provides access to stored object by pointer.
			 * @return Pointer to stored object.
			 */
			pointer operator->() const { return (get()); }

			/**
			 * @brief Returns a pointer to the managed object or nullptr if no object is owned.
			 * @return Pointer to the managed object or nullptr if no object is owned.
			 */
			pointer get() const { return (this->get_pointer()); }

			/**
			 * @brief Checks whether *this owns an object, i.e. whether get() != nullptr.
			 * @return true if *this owns an object, false otherwise.
			 */
			operator bool() const { return (get() != pointer()); }

			/**
			 * @brief Releases the ownership of the managed object if any. get() returns nullptr after the call.
			 * @return Pointer to the managed object or nullptr if there was no managed object, i.e. the value which would be returned by get() before the call.
			 */
			pointer release() 
			{	
				pointer old = get();
				this->get_pointer() = pointer();
				return (old);
			}

			/**
			 * @brief Replaces the managed object.
			 * Given current_ptr, the pointer that was managed by *this, performs the following actions, in this order:\n
			 * Saves a copy of the current pointer old_ptr = current_ptr\n
			 * Overwrites the current pointer with the argument current_ptr = ptr\n
			 * If the old pointer was non-empty, deletes the previously managed object if(old_ptr != nullptr) get_deleter()(old_ptr).
			 * @param[in] ptr Pointer to replace with.
			 */
			void reset(pointer ptr = pointer())
			{	
				pointer old = get();
				this->get_pointer() = ptr;
				if (old != pointer())
					this->get_deleter()(old);
			}
		};

		/**
		 * @brief Specialization of unique_ptr for arrays.
		 * Read more: http://en.cppreference.com/w/cpp/memory/unique_ptr
		 * Does not have the swap function, some constructors and assignment operators.
		 * @tparam T The type of values to be stored.
		 * @tparam Deleter The function object or lvalue reference to function or to function object, to be called from the destructor.
		 */
		template < typename T, typename Deleter >
		class unique_ptr<T[], Deleter> :
			public unique_ptr_base<T, Deleter>
		{
		public:
			/**
			 * Base type that defines basic interface.
			 */
			typedef unique_ptr_base<T, Deleter> Base;
			/**
			 * Propagation of pointer to stored value type.
			 * Required by standard.
			 */
			typedef typename Base::pointer pointer;
			/**
			 * Represents the type of stored value.
			 * Required by standard.
			 */
			typedef T element_type;
			/**
			 * Represents the type of deleter.
			 * Required by standard.
			 */
			typedef Deleter deleter_type;
			/**
			 * Propagation of interface defined in base class.
			 */
			using Base::get_deleter;

			/**
			 * @brief Default constructor.
			 * Constructs a unique_ptr that owns nothing. 
			 * Value-initializes the stored pointer and the stored deleter. 
			 * Requires that Deleter is DefaultConstructible.
			 */
			unique_ptr() : Base() {}

#ifdef PCAPPP_HAVE_NULLPTR_T
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			unique_ptr(std::nullptr_t) : Base(PCAPPP_NULLPTR) {}
#endif
			/**
			 * @brief Main constructor.
			 * Constructs a unique_ptr which owns p, initializing the stored pointer with p 
			 * and value-initializing the stored deleter. 
			 * Requires that Deleter is DefaultConstructible.
			 * @param[in] p Pointer to memory to be owned.
			 */
			explicit unique_ptr(pointer p) : Base(p) {}

			///@cond
			// Doxygen do not generate proper documentation for functions defined by macro.

			PCAPPP_DECLARE_NOT_COPYABLE(unique_ptr)

			/**
			 * @brief Move constructor.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_CONSTRUCTOR_NC(unique_ptr) :
				Base(PCAPPP_MOVE_WITH_CAST(const Base&, PCAPPP_MOVE_OTHER_O)) {}

			/**
			 * @brief Move assignment operator.
			 * Don't allows self assignment.
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_ASSIGNMENT_NC(unique_ptr)
			{
				// Handle self assignment case
				if (this == &PCAPPP_MOVE_OTHER_O)
					return *this;
				Base::operator=(PCAPPP_MOVE_WITH_CAST(const Base&, PCAPPP_MOVE_OTHER_O));
				return *this;
			}

			///@endcond

			/**
			 * @brief Destructor.
			 * Deallocates meeory via calling the provided deleter with internal pointer as an argument.
			 */
			~unique_ptr()
			{
				if (get() != pointer())
					this->get_deleter()(get());
			}

			/**
			 * @brief Provides access to elements of an array managed by a unique_ptr.
			 * The parameter index shall be less than the number of elements in the array; otherwise, the behavior is undefined.
			 * @param index Index of element to be returned.
			 * @return The element at index index, i.e. get()[index].
			 */
			element_type& operator[](size_t index) const { return (get()[index]); }

			/**
			 * @brief Returns a pointer to the managed object or nullptr if no object is owned.
			 * @return Pointer to the managed object or nullptr if no object is owned.
			 */
			pointer get() const { return (this->get_pointer()); }

			/**
			 * @brief Checks whether *this owns an object, i.e. whether get() != nullptr.
			 * @return true if *this owns an object, false otherwise.
			 */
			operator bool() const { return (get() != pointer()); }

			/**
			 * @brief Replaces the managed object.
			 * Given current_ptr, the pointer that was managed by *this, performs the following actions, in this order:\n
			 * Saves a copy of the current pointer old_ptr = current_ptr\n
			 * Overwrites the current pointer with the argument current_ptr = ptr\n
			 * If the old pointer was non-empty, deletes the previously managed object if(old_ptr != nullptr) get_deleter()(old_ptr).
			 */
			pointer release()
			{
				pointer old = get();
				this->get_pointer() = pointer();
				return (old);
			}

			/**
			 * @brief Replaces the managed object.
			 * Given current_ptr, the pointer that was managed by *this, performs the following actions, in this order:\n
			 * Saves a copy of the current pointer old_ptr = current_ptr\n
			 * Overwrites the current pointer with the argument current_ptr = ptr\n
			 * If the old pointer was non-empty, deletes the previously managed object if(old_ptr != nullptr) get_deleter()(old_ptr).
			 * @param[in] ptr Pointer to replace with.
			 */
			void reset(pointer ptr = pointer())
			{
				pointer old = get();
				this->get_pointer() = ptr;
				if (old != pointer())
					this->get_deleter()(old);
			}
		};

		} // namespace pcpp::memory

} // namespace pcpp

#ifndef ENABLE_CPP11_MOVE_SEMANTICS

/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with single Type template argument.
 */
#	define PCAPPP_UPTR_TYPE_ONLY(Type_) ::pcpp::memory::unique_ptr<Type_>
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with Type and Deleter template arguments.
 */
#	define PCAPPP_UPTR_TYPE_AND_DELETER(Type_, Deleter_) ::pcpp::memory::unique_ptr<Type_, Deleter_>
/**
 * Macro that can choose between PCAPPP_UPTR_TYPE_ONLY and PCAPPP_UPTR_TYPE_AND_DELETER based on count of provided arguments.
 * If provided type is a template instantiation with "," in it this macro will not work. Use some type alias method (using or typedef).
 */
#	define PCAPPP_UNIQUE_PTR(...) PCAPPP_GET_MACRO_2(__VA_ARGS__, PCAPPP_UPTR_TYPE_AND_DELETER, PCAPPP_UPTR_TYPE_ONLY)(__VA_ARGS__)
/**
 * Macro that handles the instantiation of currently used default_delete implementation.
 */
#	define PCAPPP_DEFAULT_DELETER(Type_) ::pcpp::memory::default_delete<Type_>
/**
 * Handles the choice between unique_ptr and auto_ptr pointer based on current environment.
 * In some cases we need to choose between unique_ptr and auto_ptr.
 * The case in which our implementation of unique_ptr cannot be used is when it is must be returned from function.
 */
#	define PCAPPP_UNIQUE_OR_AUTO_PTR(Type_) ::std::auto_ptr<Type_>
/**
 * This macro defined only if library implementation of unique_ptr is used.
 */
#	define PCAPPP_LIB_BASED_UNIQUE_PTR

#else // ENABLE_CPP11_MOVE_SEMANTICS

#	include <memory>
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with single Type template argument.
 */
#	define PCAPPP_UPTR_TYPE_ONLY(Type_) ::std::unique_ptr<Type_>
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with Type and Deleter template arguments.
 */
#	define PCAPPP_UPTR_TYPE_AND_DELETER(Type_, Deleter_) ::std::unique_ptr<Type_, Deleter_>
/**
 * Macro that can choose between PCAPPP_UPTR_TYPE_ONLY and PCAPPP_UPTR_TYPE_AND_DELETER based on count of provided arguments.
 * If provided type is a template instantiation with "," in it this macro will not work. Use some type alias method (using or typedef).
 */
#	define PCAPPP_UNIQUE_PTR(...) PCAPPP_GET_MACRO_2(__VA_ARGS__, PCAPPP_UPTR_TYPE_AND_DELETER, PCAPPP_UPTR_TYPE_ONLY)(__VA_ARGS__)
/**
 * Macro that handles the instantiation of currently used default_delete implementation.
 */
#	define PCAPPP_DEFAULT_DELETER(Type_) ::std::default_delete<Type_>
/**
 * Handles the choice between unique_ptr and auto_ptr pointer based on current environment.
 * In some cases we need to choose between unique_ptr and auto_ptr.
 * The case in which our implementation of unique_ptr cannot be used is when it is must be returned from function.
 */
#	define PCAPPP_UNIQUE_OR_AUTO_PTR(Type_) ::std::unique_ptr<Type_>

#endif // ENABLE_CPP11_MOVE_SEMANTICS

#endif /* PCAPPP_UNIQUE_PTR_H */