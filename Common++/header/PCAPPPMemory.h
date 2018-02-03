#ifndef PCAPPP_MEMORY
#define PCAPPP_MEMORY

#include <memory>
#include <cstdlib>

#include "CPP11.h"

#ifdef PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR
	#define PCAPPP_SUPPRESS_VIRTUAL_BASE_ALLOCATOR__
#endif // PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR

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
		 * @brief Helper structure that wraps up a call to delete expression.
		 * @tparam T The base type of value to be deallocated via delete.
		 */
		template < typename T >
		struct default_delete
		{
			/**
			 * Type of pointer to value type.
			 */
			typedef T* pointer;
			/**
			 * @brief Calls delete expression on provided pointer.
			 * Does nothing if provided pointer is same as result of expression: pointer().
			 * @param ptr Pointer to object to be deleted.
			 */
			void operator()(pointer ptr) const
			{
				if (ptr != pointer()) 
				{
					delete ptr;
				}
			}
		};

		/**
		 * @brief Helper structure that wraps up a call to delete[] expression.
		 * This specialisation is provided for deallocation of arrays of object with type T.
		 * @tparam T The base type of value to be deallocated via delete[].
		 */
		template < typename T >
		struct default_delete<T[]>
		{
			/**
			 * Type of pointer to value type.
			 */
			typedef T* pointer;
			/**
			 * @brief Calls delete[] expression on provided pointer.
			 * Does nothing if provided pointer is same as result of expression: pointer().
			 * @param ptr Pointer to object to be deleted.
			 */
			void operator()(pointer ptr) const
			{
				if (ptr != pointer())
				{
					delete[] ptr;
				}
			}
		};

		/**
		* \namespace Implementation
		* \brief Special namespace that wraps up an implementation details of some classes from pcpp::memory namespace.
		*/
		namespace Implementation
		{
			/**
			 * @brief Special tag for template dispatching with two specialisations.
			 * Represents true outcome.
			 */
			struct TrueTag {};
			/**
			 * @brief Special tag for template dispatching with two specialisations.
			 * Represents false outcome.
			 */
			struct FalseTag {};

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

			/**
			 * @brief Helper template structure that wraps up two values.
			 * This is a main template that actually have nothing to do with the compression of provided values.
			 * It is a simple dummy class that must never be reached by template resolution.
			 * In case if it is reaced it can't be constructed and sizeof this class is explicitly some huge value.
			 * This main template is a fallback for incorrect usage of this class.\n
			 * PRINCIPLE:\n
			 * The compression magic happens in a specialisations.
			 * The basic principle is that deriving from empty (no value members only function members) base class is costless, 
			 * but storing same object as a value member costs one byte plus padding and alignment.
			 * This types of objects are commonly a function objects.
			 */
			template < typename T1, typename T2, typename TagType >
			class CompressedPair
			{ private: T2 dummy[1000]; CompressedPair() {} ~CompressedPair() {} };
			/**
			 * @brief Specialisation of ComplessedPair for two non-empty types.
			 * This template actually have nothing to do with the compression of provided types.
			 * It is simply stores them sequentially one after another.
			 * This template is a fallback for all provided non-empty types.
			 * This technique has a more general approach with variadic templates or variadic macros. 
			 * But the clearer way - variadic templates is not part of C++98 standard,
			 * so we decided to use template tag dispatching with explicit/partial specialisation for dispatcher in cases when it is needed.
			 * For this template the requirement is applied to T1 and T2 - they must be copy constructible.
			 * @tparam T1 The type of first value to store.
			 * @tparam T2 The type of second value to store.
			 */
			template < typename T1, typename T2>
			class CompressedPair<T1, T2, FalseTag>
			{
			private:
				T1 m_Val1;
				T2 m_Val2;
			public:
				/**
				 * @brief Main constructor of this calss.
				 * Simply calls copy constructors of members with corresponding parameters.\n
				 * The T1 and T2 must meet copy-constructible requrement.
				 * @param val1 The value to be provide to the copy constructor of first stored value.
				 * @param val2 The value to be provide to the copy constructor of secnd stored value.
				 */
				CompressedPair(const T1& val1, const T2& val2) :
					m_Val1(val1), m_Val2(val2) {}
				/**
				 * @brief Method to access the first stored value.
				 * @return Reference to the first stored value.
				 */
				T1& get_first() { return (m_Val1); } // () prevents compiler from certain type of RVO
				/**
				 * @brief Method to access the first stored value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the const-qualified first stored value.
				 */
				const T1& get_first() const { return (m_Val1); }
				/**
				 * @brief Method to access the second stored value.
				 * @return Reference to the second stored value.
				 */
				T2& get_second() { return (m_Val2); }
				/**
				 * @brief Method to access the second stored value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the const-qualified second stored value.
				 */
				const T2& get_second() const { return (m_Val2); }
			};
			/**
			 * @brief Specialisation of ComplessedPair for any empty type as it's first argument and some type as second.
			 * The compression magic is happening here.
			 * See the compression principle in CompressedPair main template description.
			 * For this template the requirement is applied to T2 - it must be copy constructible.
			 * @tparam T1 The type to be derived from.
			 * @tparam T2 The type to be stored.
			 */
			template < typename T1, typename T2 >
			class CompressedPair<T1, T2, TrueTag> :
				private T1 // private -> Don't add any names to namespace of this class
			{
			private:
				/**
				 * Type alias for base class.
				 */
				typedef T1 Base;

				T2 m_Val2;
			public:
				/**
				 * @brief Main constructor of this calss.
				 * By defult empty objects have great default constructors.
				 * The signature of this function must be the same as for other specialisation, 
				 * except base class must be provided by value to not fill the stack (sizeof Base == 1 Byte, sizeof Base& == 4/8 Byte).
				 * @param val2 The value to be provide to the copy constructor of stored value.
				 */
				CompressedPair(const Base, const T2& val2) :
					Base(), m_Val2(val2) {}
				/**
				 * @brief Method to access the compressed base value.
				 * @return Reference to the first stored type.
				 */
				Base& get_first() { return (*this); }
				/**
				 * @brief Method to access the compressed base value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the first stored type.
				 */
				const Base& get_first() const { return (*this); }
				/**
				 * @brief Method to access the actually stored value.
				 * @return Reference to the second stored value.
				 */
				T2& get_second() { return (m_Val2); }
				/**
				 * @brief Method to access the actually stored value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the const-qualified second stored value.
				 */
				const T2& get_second() const { return (m_Val2); }
			};

			/**
			 * @brief Special structure that helps to dispatch CompressedPair class depending on template arguments.
			 * This is a main template. It dispatches all not-known types to a CompressedPair<T1, T2, FalseTag>.
			 * If you need to add any custom deleters just add a specialisation of this template.
			 * As an example take a look provided specialisations.
			 * @tparam T1 The type to be passed to CompressedPair template as T1 template argument.
			 * @tparam T2 The type to be passed to CompressedPair template as T2 template argument.
			 */
			template < typename T1, typename T2 >
			struct CompressedPairDispatcher
			{
				typedef CompressedPair<T1, T2, FalseTag> pair_type;
			};
			/**
			 * @brief Specialisation that dispatches any default_delete<T> type to CompressedPair<default_delete<T>, T2, TrueFlag>
			 * This is an example of dispatching specialisation.
			 * This specialisation don't dispatches default_delete<T[]>,
			 * @tparam T2 Some type to be passed to default_delete template.
			 */
			template < typename T2 >
			struct CompressedPairDispatcher< default_delete< typename remove_pointer<T2>::type >, T2 >
			{
				typedef CompressedPair<default_delete< typename remove_pointer<T2>::type >, T2, TrueTag> pair_type;
			};
			/**
			 * @brief Specialisation that dispatches any default_delete<T[]> type to CompressedPair<default_delete<T[]>, T2, TrueFlag>
			 * This is an example of dispatching specialisation.
			 * This specialisation don't dispatches default_delete<T>,
			 * @tparam T2 Some type to be passed to default_delete template.
			 */
			template < typename T2 >
			struct CompressedPairDispatcher< default_delete< typename remove_pointer<T2>::type[] >, T2 >
			{
				typedef CompressedPair<default_delete< typename remove_pointer<T2>::type[] >, T2, TrueTag> pair_type;
			};

		} // namespace pcpp::memory::Implementation

		/**
		 * @brief The base interface class for all allocators.
		 * Defines the basic types used by allocators and their users.\n
		 * Defines the interface which all allocators must implement.\n
		 * May be an abstract class. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
		 * @tparam T The type for which the allocator is responsible to allocate memory.
		 */ 
		template < typename T >
		struct allocator
		{
			/**
			 * Verbose type for T class template parameter.
			 */
			typedef T value_type;
			/**
			 * Type of pointer to T.
			 */
			typedef T* pointer;
			/**
			 * Type of pointer to const T.
			 */
			typedef const T* const_pointer;
			/**
			 * Type of reference to T.
			 */
			typedef T& reference;
			/**
			 * Type of reference to const T.
			 */
			typedef const T& const_reference;
#ifdef PCAPPP_SUPPRESS_VIRTUAL_BASE_ALLOCATOR__
			/**
			 * @brief Represents the facility responsible for memory allocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] l Some allocators (array allocator for example) need this parameter to determine how much memory must be allocated.
			 * @return Pointer to newly allocated memory (as an interface function) or nullptr (as not interface function).
			 */
			pointer allocate(std::size_t l) { return nullptr; }
			/**
			 * @brief Represents the facility responsible for memory deallocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			void deallocate(pointer p) {}
#else
			/**
			 * @brief Represents the facility responsible for memory allocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] l Some allocators (array allocator for example) need this parameter to determine how much memory must be allocated.
			 * @return Pointer to newly allocated memory (as an interface function) or nullptr (as not interface function).
			 */
			virtual T* allocate(std::size_t l) = 0;
			/**
			 * @brief Represents the facility responsible for memory deallocation.
			 * This function may be an interface method. Depends on PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro def. More information may be found @ref memory_predef_mem_system_state "here".
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			virtual void deallocate(pointer p) = 0;
#endif // PCAPPP_SUPPRESS_VIRTUAL_BASE_ALLOCATOR__
		};

		/**
		 * @brief The default allocator main tamplate.
		 * At the time before C++11 there was no default allocator class so we must define our own.\n
		 * The most basic implementation of allocator abstract intarface.\n
		 * allocate method returns the result of "new T" expression. \n
		 * deallocate method applies "delete" expression to provided pointer.
		 * @tparam T The type for which the allocator is responsible to allocate memory.
		 */ 
		template < typename T >
		struct default_allocator
			: public allocator<T>
		{
			/**
			 * Base type of this class.
			 */
			typedef allocator<T> Base;
			/**
			 * @brief Allocates memory for one object of type T.
			 * WARNING!! The default constructor of object is called.\n
			 * Returns result of "new T" expression.
			 * @param[in] l Ignores it.
			 * @return Pointer to newly allocated memory.
			 */
			pointer allocate(std::size_t) { return new T; }
			/**
			 * @brief Deallocates memory for one object of type T.
			 * Internally applies "delete" expression to provided pointer.
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			void deallocate(pointer p) { delete p; }
		};
		/**
		 * @brief The partial specialisation of default allocator template for dynamic arrays of objects.
		 * At the time before C++11 there was no default allocator class so we must define our own.\n
		 * The most basic implementation of allocator abstract intarface for dynamic arrays.\n
		 * allocate method returns the result of "new T[size]" expression. \n
		 * deallocate method applies "delete[]" expression to provided pointer.
		 * @tparam T The type for which the allocator is responsible to allocate memory.
		 */
		template < typename T >
		struct default_allocator<T[]> :
			public allocator<T>
		{
			/**
			 * Base type of this class.
			 */
			typedef allocator<T> Base;
			/**
			 * @brief Allocates memory for size object of type T.
			 * WARNING!! The default constructor of every object is called.\n
			 * Returns result of "new T[size]" expression.
			 * @param[in] size Count of objects to be allocated.
			 * @return Pointer to newly allocated memory.
			 */
			pointer allocate(std::size_t size) { return new T[size]; }
			/**
			 * @brief Deallocates memory for array of objects of type T.
			 * Internally applies "delete[]" expression to provided pointer.
			 * @param[in] p Pointer to a memory previously allocated by this allocator.
			 */
			void deallocate(pointer p) { delete[] p; }
		};

		/**
		 * @brief Helper structure that wraps up types that may be declared by the provided allocator type.
		 * @tparam Allocator The allocator type to be wrapped up.
		 */
		template < typename Allocator >
		struct allocator_traits
		{
			/**
			 * Verbose type for Allocator class template parameter.
			 */
			typedef Allocator allocator_type;
			/**
			 * The type of value that Allocator is responsible to allocate memory for.
			 */
			typedef typename Allocator::value_type  value_type;
			/**
			 * The type of pointer to Allocator's value type.
			 */
			typedef typename Allocator::pointer pointer;
			/**
			 * The type of pointer to Allocator's constant value type.
			 */
			typedef typename Allocator::const_pointer const_pointer;
			/**
			 * The type of reference to Allocator's value type.
			 */
			typedef typename Allocator::reference reference;
			/**
			 * The type of reference to Allocator's constant value type.
			 */
			typedef typename Allocator::const_reference const_reference;
		};

#ifndef NO_TEMPLATE_FUNCTION_DEF_ARGS
		/**
		 * @brief Creates a static object of provided type and returns reference to it.
		 * @tparam Allocator The allocator type to be created.
		 * @tparam traits Defines a set of requirements for Allocator type (Default: pcpp::memory::allocator_traits<Allocator>).
		 */
		template < typename Allocator, typename traits = allocator_traits<Allocator> >
		typename traits::allocator_type& staticAllocator()
		{
			static typename traits::allocator_type allocator;
			return allocator;
		}
#else
		/**
		 * @brief Creates a static object of provided type and returns reference to it.
		 * This implementation is created for compilers where default function template arguments are not supported.
		 * @tparam Allocator The allocator type to be created.
		 */
		template < typename Allocator >
		Allocator& staticAllocator()
		{
			static Allocator allocator;
			return allocator;
		}
#endif

/**
 * Special macro that can select between two functional-macro definitions based on count of arguments
 */
#define PCAPPP_GET_MACRO_2(_1,_2,NAME,...) NAME
/**
 * Next section represents a custom implementstion of std::unique_ptr for this library.
 * The whole implementation is a corrected for C++98 copy from MSVC 16.0 (Microsoft Visual Studio 2015) standard library implementation.
 * About next 'ifndef': Basicaly every compiler that have move semantics support have it's own implementation of std::unique_ptr.
 * So we just provide a macro definition that swaps our inplementation with standard one when it is possible.
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

// In case of unsupported std::nullptr_t nullptr will be a macro def (from CPP11.h)
#ifndef nullptr
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
			typename Implementation::CompressedPairDispatcher<Deleter, pointer>::pair_type m_Pair;
		private:
			/**
			 * This function is explicitly hiden (there is no delete keyword for functions in C++98). 
			 * unique_ptr_base is not copyable
			 */
			unique_ptr_base(const unique_ptr_base&) {}
			/**
			 * This function is explicitly hiden (there is no delete keyword for functions in C++98). 
			 * unique_ptr_base is not copyable
			 */
			unique_ptr_base& operator=(const unique_ptr_base& a) { return *this; }
		};

		/**
		 * @brief Library implementation of std::unique_ptr.
		 * Read more: http://en.cppreference.com/w/cpp/memory/unique_ptr \n
		 * Does not have the swap function, some constructors and assignment operators.
		 * This class CAN NOT be returned in C++98.
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

// In case of unsupported std::nullptr_t nullptr will be a macro def (from CPP11.h)
#ifndef nullptr
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			unique_ptr(std::nullptr_t) : Base(nullptr) {}
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
			 * @brief Destructor.
			 * Deallocates memeory via calling the provided deleter with internal pointer as an argument.
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
			 * This function is explicitly hiden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr(const unique_ptr&) {}
			/**
			 * This function is explicitly hiden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr& operator=(const unique_ptr& a) { return *this; }
		};
		/**
		 * @brief Specialisation of unique_ptr for arrays.
		 * Read more: http://en.cppreference.com/w/cpp/memory/unique_ptr
		 * Does not have the swap function, some constructors and assignment operators.
		 * This class CAN NOT be returned in C++98.
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

// In case of unsupported std::nullptr_t nullptr will be a macro def (from CPP11.h)
#ifndef nullptr
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			unique_ptr(std::nullptr_t) : Base(nullptr) {}
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
			 * @brief Destructor.
			 * Deallocates memeory via calling the provided deleter with internal pointer as an argument.
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
			 * @return The element at index index, i.e. get()[i].
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
			 * This function is explicitly hiden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr(const unique_ptr&) {}
			/**
			 * This function is explicitly hiden (there is no delete keyword for functions in C++98). 
			 * unique_ptr is not copyable
			 */
			unique_ptr& operator=(const unique_ptr& a) { return *this; }
		};
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with single Type template argument.
 */
#define PCAPPP_UPTR_TYPE_ONLY(Type_) pcpp::memory::unique_ptr<Type_>
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with Type and Deleter template arguments.
 */
#define PCAPPP_UPTR_TYPE_AND_DELETER(Type_, Deleter_) pcpp::memory::unique_ptr<Type_, Deleter_>
/**
 * Macro that can choose between PCAPPP_UPTR_TYPE_ONLY and PCAPPP_UPTR_TYPE_AND_DELETER based on count of provided arguments.
 * If provided type is a tamplate instatiation with "," in it this macro will not work. Use some type alias method (using or typedef).
 */
#define PCAPPP_UNIQUE_PTR(...) PCAPPP_GET_MACRO_2(__VA_ARGS__, PCAPPP_UPTR_TYPE_AND_DELETER, PCAPPP_UPTR_TYPE_ONLY)(__VA_ARGS__)
/**
 * Macro that handles the instantiation of currently used default_delete implementation.
 */
#define PCAPPP_DEFAULT_DELETER(Type_) pcpp::memory::default_delete<Type_>
/**
 * Handles the choice between unique_ptr and auto_ptr pointer based on current environment.
 * In some cases we need to choose between unique_ptr and auto_ptr.
 * The case in which our implementation of unique_ptr cannot be used is when it is must be returned from function.
 */
#define PCAPPP_UNIQUE_OR_AUTO_PTR(Type_) std::auto_ptr<Type_>
/**
 * If move semantics is supported return std::move(Value_), otherwise return (Value_)
 */
#define PCAPPP_MOVE(Value_) (Value_)
/**
 * If move semantics is supported return std::forward(Value_), otherwise return (Value_)
 */
#define PCAPPP_FORWARD(Value_) (Value_)
#else
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with single Type template argument.
 */
#define PCAPPP_UPTR_TYPE_ONLY(Type_) std::unique_ptr<Type_>
/**
 * Macro that handles the instantiation of currently used unique_ptr implementation with Type and Deleter template arguments.
 */
#define PCAPPP_UPTR_TYPE_AND_DELETER(Type_, Deleter_) std::unique_ptr<Type_, Deleter_>
/**
 * Macro that can choose between PCAPPP_UPTR_TYPE_ONLY and PCAPPP_UPTR_TYPE_AND_DELETER based on count of provided arguments.
 * If provided type is a tamplate instatiation with "," in it this macro will not work. Use some type alias method (using or typedef).
 */
#define PCAPPP_UNIQUE_PTR(...) PCAPPP_GET_MACRO_2(__VA_ARGS__, PCAPPP_UPTR_TYPE_AND_DELETER, PCAPPP_UPTR_TYPE_ONLY)(__VA_ARGS__)
/**
 * Macro that handles the instantiation of currently used default_delete implementation.
 */
#define PCAPPP_DEFAULT_DELETER(Type_) std::default_delete<Type_>
/**
 * Handles the choice between unique_ptr and auto_ptr pointer based on current environment.
 * In some cases we need to choose between unique_ptr and auto_ptr.
 * The case in which our implementation of unique_ptr cannot be used is when it is must be returned from function.
 */
#define PCAPPP_UNIQUE_OR_AUTO_PTR(Type_) std::unique_ptr<Type_>
#include <utility>
 /**
 * If move semantics is supported return std::move((Value_)), otherwise return (Value_)
 */
#define PCAPPP_MOVE(Value_) std::move((Value_))
 /**
 * If move semantics is supported return std::forward((Value_)), otherwise return (Value_)
 */
#define PCAPPP_FORWARD(Value_) std::forward((Value_))
#endif // !ENABLE_CPP11_MOVE_SEMANTICS

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORY */