#ifndef PCAPPP_UNIQUE_PTR_H
#define PCAPPP_UNIQUE_PTR_H

#include "CPP11.h"
#include "MemoryUtils.h"
#include "MemoryImplementation.h"
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
 * Special macro that can select between two functional-macro definitions based on count of arguments
 */
#define PCAPPP_GET_MACRO_2(_1,_2,NAME,...) NAME
/**
 * Next section represents a custom implementation of std::unique_ptr for this library.
 * The whole implementation is a corrected for C++98 copy from MSVC 16.0 (Microsoft Visual Studio 2015) standard library implementation.
 * About next 'ifndef': Basically every compiler that have move semantics support have it's own implementation of std::unique_ptr.
 * So we just provide a macro definition that swaps our implementation with standard one when it is possible.
 */
#ifndef ENABLE_CPP11_MOVE_SEMANTICS
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
			 * @param p Pointer to memory to be owned.
			 */
			explicit unique_ptr_base(pointer p) : m_Pair(Deleter(), p) {}

			/**
			 * @brief Move constructor.
			 * This is the move constructor which is based on library implementation of C++11 move semantics.
			 * This function is basically the unique_ptr_base(unique_ptr_base&& other) move constructor.
			 * @param proxy Special object which represents rvalue reference to other unique_ptr_base instance.
			 */
			unique_ptr_base(::pcpp::move_semantics::MoveProxy<unique_ptr_base> proxy) :
				m_Pair(proxy.ref.m_Pair.get_first(), proxy.ref.m_Pair.get_second())
			{
				// Nullify provided object
				proxy.ref.m_Pair.get_first() = Deleter();
				proxy.ref.m_Pair.get_second() = pointer();
			}

			/**
			 * @brief Move assignment operator.
			 * This is the move assignment operator which is based on library implementation of C++11 move semantics.
			 * This function is basically the unique_ptr_base& operator=(unique_ptr_base&& other) move assignment operator.
			 * @param proxy Special object which represents rvalue reference to other unique_ptr_base instance.
			 * @return Reference to this object.
			 */
			unique_ptr_base& operator=(::pcpp::move_semantics::MoveProxy<unique_ptr_base> proxy)
			{
				// Handle self assignment case
				if (this == &proxy.ref)
					return *this;
				// This member may optimized with move semantics but this case is to hard to handle in client side.
				// So we don't make any assumptions and just copy it.
				// For zero-size object this operation is costless.
				m_Pair.get_first() = proxy.ref.m_Pair.get_first();
				// Second is definitely a pointer
				m_Pair.get_second() = proxy.ref.m_Pair.get_second();
				// Nullify provided object
				proxy.ref.m_Pair.get_first() = Deleter();
				proxy.ref.m_Pair.get_second() = pointer();
				return *this;
			}

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
		private:
			/**
			 * This function is explicitly hidden (there is no delete keyword for functions in C++98). 
			 * unique_ptr_base is not copyable
			 */
			unique_ptr_base(const unique_ptr_base&) {}
			/**
			 * This function is explicitly hidden (there is no delete keyword for functions in C++98). 
			 * unique_ptr_base is not copyable
			 */
			unique_ptr_base& operator=(const unique_ptr_base& a) { return *this; }
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
			 * @param p Pointer to memory to be owned.
			 */
			explicit unique_ptr(pointer p) : Base(p) {}

			/**
			 * @brief Move constructor.
			 * This is the move constructor which is based on library implementation of C++11 move semantics.
			 * This function is basically the unique_ptr(unique_ptr&& other) move constructor.
			 * @param proxy Special object which represents rvalue reference to other unique_ptr instance.
			 */
			unique_ptr(::pcpp::move_semantics::MoveProxy<unique_ptr> proxy) :
				Base(PCAPPP_MOVE(dynamic_cast<Base&>(proxy.ref))) {} // This is basically the Base(std::move(other)) expression but cast must be specified explicitly.
			
			/**
			 * @brief Move assignment operator.
			 * This is the move assignment operator which is based on library implementation of C++11 move semantics.
			 * This function is basically the unique_ptr& operator=(unique_ptr&& other) move assignment operator.
			 * @param proxy Special object which represents rvalue reference to other unique_ptr instance.
			 * @return Reference to this object.
			 */
			unique_ptr& operator=(::pcpp::move_semantics::MoveProxy<unique_ptr> proxy)
			{
				if (this == &proxy.ref)
					return *this;
				// This is basically the Base::operator=(std::move(other)) expression but cast must be specified explicitly.
				Base::operator=(PCAPPP_MOVE( dynamic_cast<Base&>(proxy.ref) ));
				return *this;
			}

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
			explicit operator bool() const { return (get() != pointer()); }

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
			 */
			void reset(pointer ptr = pointer())
			{	
				pointer old = get();
				this->get_pointer() = ptr;
				if (old != pointer())
					this->get_deleter()(old);
			}
		private:
			/**
			 * This function is explicitly hidden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr(const unique_ptr&) {}
			/**
			 * This function is explicitly hidden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr& operator=(const unique_ptr& a) { return *this; }
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
			 * @param p Pointer to memory to be owned.
			 */
			explicit unique_ptr(pointer p) : Base(p) {}

			/**
			 * @brief Move constructor.
			 * This is the move constructor which is based on library implementation of C++11 move semantics.
			 * This function is basically the unique_ptr(unique_ptr&& other) move constructor.
			 * @param proxy Special object which represents rvalue reference to other unique_ptr instance.
			 */
			unique_ptr(::pcpp::move_semantics::MoveProxy<unique_ptr> proxy) :
				Base(PCAPPP_MOVE( dynamic_cast<Base&>(proxy.ref) )) {} // This is basically the Base(std::move(other)) expression but cast must be specified explicitly.

			/**
			 * @brief Move assignment operator.
			 * This is the move assignment operator which is based on library implementation of C++11 move semantics.
			 * This function is basically the unique_ptr& operator=(unique_ptr&& other) move assignment operator.
			 * @param proxy Special object which represents rvalue reference to other unique_ptr instance.
			 * @return Reference to this object.
			 */
			unique_ptr& operator=(::pcpp::move_semantics::MoveProxy<unique_ptr> proxy)
			{
				if (this == &proxy.ref)
					return *this;
				// This is basically the Base::operator=(std::move(other)) expression but cast must be specified explicitly.
				Base::operator=(PCAPPP_MOVE( dynamic_cast<Base&>(proxy.ref) ));
				return *this;
			}

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
			explicit operator bool() const { return (get() != pointer()); }

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
			 */
			void reset(pointer ptr = pointer())
			{
				pointer old = get();
				this->get_pointer() = ptr;
				if (old != pointer())
					this->get_deleter()(old);
			}
		private:
			/**
			 * This function is explicitly hidden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr(const unique_ptr&) {}
			/**
			 * This function is explicitly hidden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr& operator=(const unique_ptr& a) { return *this; }
		};

/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with single Type template argument.
 */
#define PCAPPP_UPTR_TYPE_ONLY(Type_) ::pcpp::memory::unique_ptr<Type_>
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with Type and Deleter template arguments.
 */
#define PCAPPP_UPTR_TYPE_AND_DELETER(Type_, Deleter_) ::pcpp::memory::unique_ptr<Type_, Deleter_>
/**
 * Macro that can choose between PCAPPP_UPTR_TYPE_ONLY and PCAPPP_UPTR_TYPE_AND_DELETER based on count of provided arguments.
 * If provided type is a template instantiation with "," in it this macro will not work. Use some type alias method (using or typedef).
 */
#define PCAPPP_UNIQUE_PTR(...) PCAPPP_GET_MACRO_2(__VA_ARGS__, PCAPPP_UPTR_TYPE_AND_DELETER, PCAPPP_UPTR_TYPE_ONLY)(__VA_ARGS__)
/**
 * Macro that handles the instantiation of currently used default_delete implementation.
 */
#define PCAPPP_DEFAULT_DELETER(Type_) ::pcpp::memory::default_delete<Type_>
/**
 * Handles the choice between unique_ptr and auto_ptr pointer based on current environment.
 * In some cases we need to choose between unique_ptr and auto_ptr.
 * The case in which our implementation of unique_ptr cannot be used is when it is must be returned from function.
 */
#define PCAPPP_UNIQUE_OR_AUTO_PTR(Type_) ::std::auto_ptr<Type_>
#else

#include <memory>

/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with single Type template argument.
 */
#define PCAPPP_UPTR_TYPE_ONLY(Type_) ::std::unique_ptr<Type_>
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with Type and Deleter template arguments.
 */
#define PCAPPP_UPTR_TYPE_AND_DELETER(Type_, Deleter_) ::std::unique_ptr<Type_, Deleter_>
/**
 * Macro that can choose between PCAPPP_UPTR_TYPE_ONLY and PCAPPP_UPTR_TYPE_AND_DELETER based on count of provided arguments.
 * If provided type is a template instantiation with "," in it this macro will not work. Use some type alias method (using or typedef).
 */
#define PCAPPP_UNIQUE_PTR(...) PCAPPP_GET_MACRO_2(__VA_ARGS__, PCAPPP_UPTR_TYPE_AND_DELETER, PCAPPP_UPTR_TYPE_ONLY)(__VA_ARGS__)
/**
 * Macro that handles the instantiation of currently used default_delete implementation.
 */
#define PCAPPP_DEFAULT_DELETER(Type_) ::std::default_delete<Type_>
/**
 * Handles the choice between unique_ptr and auto_ptr pointer based on current environment.
 * In some cases we need to choose between unique_ptr and auto_ptr.
 * The case in which our implementation of unique_ptr cannot be used is when it is must be returned from function.
 */
#define PCAPPP_UNIQUE_OR_AUTO_PTR(Type_) ::std::unique_ptr<Type_>

#endif // !ENABLE_CPP11_MOVE_SEMANTICS

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_UNIQUE_PTR_H */