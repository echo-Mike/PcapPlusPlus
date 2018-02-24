#ifndef PCAPPP_MEMORYPROXYINTERFACE
#define PCAPPP_MEMORYPROXYINTERFACE

#include <cstdint>
#include <exception>

#include "CPP11.h"
#include "MemoryUtils.h"
#include "MoveSemantics.h"
#include "AllocatorAdapter.h"

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
		 * @class MemoryProxyInterface
		 * @brief This class defines an interface for objects that are capable of managing some amount of memory. 
		 * Common known name for this objects is "Memory resource".
		 */ 
		template < typename T >
		class MemoryProxyInterface 
		{
		public:
			/**
			 * Defines underlying memory type.
			 */
			typedef T value_type;
			/**
			 * Defines type of pointer to underlying memory type.
			 */
			typedef T* pointer;
			/**
			 * Defines type of constant pointer to underlying memory type.
			 */
			typedef const T* const_pointer;
			/**
			 * Defines type of reference to element of underlying memory type.
			 */
			typedef T& reference;
			/**
			 * Defines type of constant reference to element of underlying memory type.
			 */
			typedef const T& const_reference;

			/**
			 * Defines type of variables that represents size values.
			 */
			typedef std::size_t size;
			/**
			 * Defines type of variables that represents index values.
			 */
			typedef int index;
			/**
			 * Defines type of variables that represents initial memory values for std::memset.
			 */
			typedef int memory_value;

			/* Virtual interface */ // YES - virtual call may be inlined.

			/**
			 * @brief Default virtual destructor.
			 * This class is an abstract base class so we need a virtual destructor
			 * to deallocate derived classes by polymorphic pointer or reference to base class.
			 */
			virtual ~MemoryProxyInterface() {};
			/**
			 * @brief Represents the access facility to a common length member field used in derived classes.
			 * Derived class must overload this function to add read access to it's length member field.\n
			 * The length member field may represent anything but most commonly it is a length of data helded by derived class.
			 * @return Length value specified by derived class.
			 */
			inline virtual size getLength() const = 0;
			/**
			 * @brief Represents the access facility to a common member field that represents current owning status of underlying data used in derived classes.
			 * Derived class must overload this function to add read access to it's owning member field.\n
			 * The owning member field may represent anything but most commonly it is an owning status of underlying data helded by derived class.
			 * @return Mostly common: true if object owns it's underlying data, false otherwise.
			 */
			inline virtual bool isOwning() const = 0;
			/**
			 * @brief Represents the access facility to the beginning of underlying data in derived classes.
			 * Derived class must overload this function to add access to it's underlying data.
			 * @return Mostly common: Pointer to beginning of underlying data.
			 */
			inline virtual pointer get() = 0;
			/**
			 * @brief Represents the access facility to the beginning of underlying data in derived classes.
			 * This is an overload selected by compiler if object is const-qualified.\n
			 * Derived class must overload this function to add read-only access to it's underlying data.
			 * @return Mostly common: Pointer to beginning of const-qualified underlying data.
			 */
			inline virtual const_pointer get() const = 0;
			/**
			 * @brief Represents the facility to release the ownership of underlying data.
			 * Derived class must overload this function if they can release ownership of data and set their self to some null-state.
			 * @return Mostly common: Pointer to beginning of underlying data.
			 */
			virtual pointer relese() = 0;
			/**
			 * @brief Represents the reset operation.
			 * Derived class must overload this function if they can deallocate current data and set provided data as current.
			 * @param[in] ptr Pointer to new data to take care off.
			 * @param[in] length Length of new data.
			 * @param[in] ownership Ownership indicator.
			 * @return Mostly common: true if reset operation is successful, false otherwise.
			 */
			virtual bool reset(pointer ptr, size length = 0, bool ownership = true) = 0;
			/**
			 * @brief Represents the facility to check if current object is in the null-state.
			 * Derived class must overload this function if they have some null-state and can check that this object is in that null-state.
			 * @return Mostly common: true if object is in null-state, false otherwise.
			 */
			virtual operator bool() const = 0;
			/**
			 * @brief Method to check if current object is in the null-state.
			 * Basically the negation of operator bool call.
			 * @return Opposite of the result of operator bool call.
			 */
			inline bool isInNullState() const { return !(this->operator bool()); }
			
			/* Underlying data modification API */

			/**
			 * @brief Represents the reallocation operation of underlying data.
			 * Reallocation operation is defined as following set of steps:  
			 * * Allocate new memory with size newBufferLength via allocator;  
			 * * Set new memory content per byte as initialValue;  
			 * * Copy old memory content to new memory;  
			 * * Deallocate old memory via allocator.  
			 *  
			 * @param[in] newBufferLength New size of data.
			 * @param[in] initialValue Per-byte initial value of new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
			virtual bool reallocate(size newBufferLength, memory_value initialValue = 0) = 0;
			/**
			 * @brief Represents the clear operation.
			 * Call to clear operation signifies object to deallocate it's underlying data and set it self into a null-state.
			 * @return true if operation finished successfully, false otherwise.
			 */
			virtual bool clear() = 0;
			/**
			 * @brief Represents the append operation over underlying data.
			 * Append operation must add dataToAppendLen count of data entries to the end of data.\n
			 * This overload sets memory for appended data to specified initialValue.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
			virtual bool append(size dataToAppendLen, memory_value initialValue = 0) = 0;
			/**
			 * @brief Represents the concatenation operation over underlying and provided data.
			 * Append operation must add dataToAppendLen count of data entries to the end of data.\n
			 * This overload sets memory for appended data same as in provided dataToAppend buffer.
			 * @param[in] dataToAppend Buffer memory to be appended to current data.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @return true if operation finished successfully, false otherwise.
			 */
			virtual bool append(const_pointer dataToAppend, size dataToAppendLen) = 0;
			/**
			 * @brief Represents the insertion operation over underlying data.
			 * Insert operation must add dataToAppendLen count of data entries before the specified location in current data.\n
			 * This overload sets memory for inserted data to specified initialValue.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
			virtual bool insert(index atIndex, size dataToInsertLen, memory_value initialValue = 0) = 0;
			/**
			 * @brief Represents the insertion operation over underlying and provided data.
			 * Insert operation must add dataToAppendLen count of data entries before the specified location in current data.\n
			 * This overload sets memory for inserted data same as in provided dataToAppend buffer.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsert Buffer memory to be inserted to current data.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @return true if operation finished successfully, false otherwise.
			 */
			virtual bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen) = 0;
			/**
			 * @brief Represents the remove operation over underlying data.
			 * Remove operation must remove numOfBytesToRemove count of data entries starting from the specified location in current data.
			 * @param[in] atIndex Index from which removal take place.
			 * @param[in] numOfBytesToRemove Size of data to be removed.
			 * @return true if operation finished successfully, false otherwise.
			 */
			virtual bool remove(index atIndex, size numOfBytesToRemove) = 0;
		};
		
		/**
		 * \namespace MemoryProxyTags
		 * \brief Wrapper namespace over the predefined set of tags used in MemoryProxy template class tag dispatch.
		 */
		namespace MemoryProxyTags
		{
			struct SizeAwareTag {};
			// struct SizeUnawareTag {};
			struct ContentAwareTag {};
			struct OldMemoryModelTag {};
			// struct SafeOldMemoryModelTag {};
			struct DpdkTag {};
		} // namespace pcpp::memory::MemoryProxyTags

		/**
		 * @class MemoryProxy
		 * @brief This is a main template used in MemoryProxy tag dispatch.
		 * This template will be instantiated if provided in ProxyTag tag haven't got it's partial specialization.\n
		 * Objects of class generated by this template can't be created.\n
		 * Fatal error must be raised by compiler.\n
		 * This class is a fallback for incorrect usage.
		 * @tparam[in] Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
		 * @tparam[in] ProxyTag Tag that defines which class will be created.
		 */ 
		template < typename Allocator, typename ProxyTag >
		class MemoryProxy { private: char dummy[1234];  MemoryProxy() {} ~MemoryProxy() {} };

		/**
		 * @class AllocatorPointerPair
		 * @brief This is a helper class that is manages compressed pair of allocator and pointer. 
		 * @tparam[in] Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
		 * @tparam[in] traits Defines a set of requirements for Allocator type (Default: pcpp::memory::allocator_traits<Allocator>)
		 */ 
		template < typename Allocator, typename traits = allocator_traits<Allocator> >
		struct AllocatorPointerPair
		{
			/**
			 * Defines type of widely used allocator adapter for derived classes.
			 */
			typedef AllocatorAdapter<Allocator> Adapter;
			/**
			 * Defines type of pointer to memory.
			 */
			typedef typename traits::pointer pointer;
			/**
			 * Defines type of pair that stores allocator and pointer compressed if it is possible.
			 */
			typedef typename pcpp::memory::Implementation::CompressedPairDispatcher< Adapter, typename traits::pointer >::pair_type allocator_pointer_pair_t;

			/**
			 * @brief Default constructor.
			 * Value-initializes the stored pointer and the stored allocator. 
			 * Requires that Allocator is DefaultConstructible.
			 */
			AllocatorPointerPair() : m_Pair(Adapter(), pointer()) {}

#ifdef PCAPPP_HAVE_NULLPTR_T
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			AllocatorPointerPair(std::nullptr_t) : m_Pair(Adapter(), pointer()) {}
#endif
			/**
			 * @brief Main constructor.
			 * Constructs a MemoryProxyAllocatorHelper initializing the stored pointer with p 
			 * and value-initializing the stored allocator. 
			 * Requires that Allocator is DefaultConstructible.
			 * @param p Pointer to memory.
			 */
			explicit AllocatorPointerPair(pointer p) : m_Pair(Adapter(), p) {}

			/**
			 * @brief Copy constructor.
			 * @param other Instance to be copied.
			 */
			AllocatorPointerPair(const AllocatorPointerPair& other) :
				m_Pair(other.m_Pair) {}

			/**
			 * @brief Copy assignment operator.
			 * @param other Instance to be copied.
			 * @return Reference to this object.
			 */
			AllocatorPointerPair& operator=(const AllocatorPointerPair& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				m_Pair = other.m_Pair;
				return *this;
			}

			/**
			 * @brief Move constructor.
			 * This is the move constructor which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
			 * @param other Instance to be moved from.
			 */
			PCAPPP_MOVE_CONSTRUCTOR(AllocatorPointerPair) :
				m_Pair(PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Pair))
			{
				// Nullify provided object
				PCAPPP_MOVE_OTHER.m_Pair.get_first() = Adapter();
				PCAPPP_MOVE_OTHER.m_Pair.get_second() = pointer();
			}

			/**
			 * @brief Move assignment operator.
			 * This is the move assignment operator which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
			 * @param other Instance to be moved from.
			 * @return Reference to this object.
			 */
			PCAPPP_MOVE_ASSIGNMENT(AllocatorPointerPair)
			{
				// Handle self assignment case
				if (this == &PCAPPP_MOVE_OTHER)
					return *this;
				m_Pair = PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Pair);
				// Nullify provided object
				PCAPPP_MOVE_OTHER.m_Pair.get_first() = Adapter();
				PCAPPP_MOVE_OTHER.m_Pair.get_second() = pointer();
				return *this;
			}

			/**
			 * @brief Method to access the stored allocator object.
			 * @return Reference to the allocator object.
			 */
			inline Adapter& get_allocator() { return (m_Pair.get_first()); }
			/**
			 * @brief Method to access the stored allocator object.
			 * This overload is selected by compiler if object is const-qualified.
			 * @return Reference to the allocator object.
			 */
			inline const Adapter& get_allocator() const { return (m_Pair.get_first()); }

			/**
			 * @brief Method to access the stored pointer.
			 * @return Reference to the pointer.
			 */
			inline typename traits::pointer& get_pointer() { return (m_Pair.get_second()); }
			/**
			 * @brief Method to access the stored pointer.
			 * This overload is selected by compiler if object is const-qualified.
			 * @return Reference to the pointer.
			 */
			inline const typename traits::pointer& get_pointer() const { return (m_Pair.get_second()); }

			allocator_pointer_pair_t m_Pair;    //<! Pair of allocator and pointer.
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORYPROXYINTERFACE */