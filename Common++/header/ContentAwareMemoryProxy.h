#ifndef PCAPPP_CONTENTAWAREMEMORYPROXY
#define PCAPPP_CONTENTAWAREMEMORYPROXY

#include <cstdint>
#include <exception>

#include "CPP11.h"
#include "MemoryProxyInterface.h"

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
		 * @brief Content aware memory proxy class specialization. 
		 * This specialization held two separate counters for internal data.\n
		 * m_Length - represents the length of known data in underlying storage.\n
		 * m_Capacity - represents the real length of memory in underlying storage.\n
		 * This is the safest specialization to handle complex data manipulation.
		 * @tparam Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
		 */ 
		template < typename Allocator >
		class MemoryProxy< Allocator, MemoryProxyTags::ContentAwareTag > :
			public MemoryProxyInterface< Allocator >
		{
		public:
			/**
			 * Base type of this class.
			 */
			typedef MemoryProxyInterface< Allocator > Base;
		protected:
			/**
			 * @brief Checks the condition in which underlying data may be safely deallocated.
			 * @return true if condition is satisfied, false otherwise.
			 */
			inline bool SafeToDeleteCondition() { return m_Ownership && m_Data; }
			/**
			 * @brief Checks the condition in which underlying data may be safely copied to another object.
			 * @return true if condition is satisfied, false otherwise.
			 */
			inline bool SafeToCopyCondition() { return m_Data && m_Capacity > 0; }
			/**
			 * @brief Data deallocation routine.
			 * @return true if deallocation ended without exceptions, false otherwise.
			 * @todo Add exception msg x 2.
			 */
			inline bool deallocateData()
			{
				try
				{
					if (SafeToDeleteCondition())
						m_Allocator.deallocate(m_Data);
				}
				catch (const std::exception& e)
				{
					// TODO:: add exception handler
					return false;
				}
				catch (...)
				{
					// TODO:: add exception handler
					return false;
				}
				return true;
			}
			/**
			 * @brief Setups object to a null-state.
			 * Basically zeroes all fields, no data is touched.
			 */
			inline void zeroFields() 
			{
				// Set all fields to their initial values
				m_Data = PCAPPP_NULLPTR;
				m_Length = 0;
				m_Capacity = 0;
				m_Ownership = false;
			}
			/**
			 * @brief Setups object to special null-state.
			 * Basically zeroes all fields, no data is touched.\n
			 * initialize method of allocator object is called.
			 */
			inline void initialize()
			{
				m_Allocator.initialize();
				zeroFields();
			}
			/**
			 * @brief Makes current object a copy of the other.
			 * Underlying data is deallocated first. Then other allocator object is copied.
			 * Next the SafeToCopyCondition is checked on other.\n
			 * If check is passed new data allocated by copied allocator with same size as other.m_Capacity and then data is copied from other.\n
			 * Else object will be set in null-state.
			 * If allocator can't allocate new memory object will be set in null-state.
			 * @param[in] other Object to make copy of.
			 * @return true if successful copy of other was made, false otherwise.
			 */
			inline bool copyDataFrom(const MemoryProxy& other)
			{
				// Deallocate current data
				deallocateData();
				// Copy allocator of other object
				m_Allocator = other.m_Allocator;
				// Check that copying of data is safe
				if (other.SafeToCopyCondition())
				{
					// Allocate new data
					// It is allocator's responsibility to handle memory allocation exceptions
					m_Data = m_Allocator.allocate(other.m_Capacity);
					// Check if new buffer was allocated
					if (m_Data == PCAPPP_NULLPTR) {// Expect nullptr/NULL returned when exception thrown on allocation
						zeroFields();
						return false;
					};
					// Copy whole data to this object memory
					std::memcpy(m_Data, other.m_Data, other.m_Capacity * sizeof(typename Base::value_type));
					// We definitely own data now
					m_Ownership = true;
					// Copy length and capacity values
					m_Length = other.m_Length;
					m_Capacity = other.m_Capacity;
					return true;
				}
				else // Other object must be in some null-state
				{	// Put our object in proper null-state
					zeroFields();
					return false;
				}
			}
#ifdef ENABLE_CPP11_MOVE_SEMANTICS
			/**
			 * @brief Moves data from other.
			 * Underlying data is deallocated first. Then other allocator object is moved.
			 * Next all other data members are copied. In the end initialize member function is called on other.\n
			 * This function is unavailable if ENABLE_CPP11_MOVE_SEMANTICS macro is not defined.
			 * @param[in:out] other Object to move from.
			 */
			inline void moveDataFrom(MemoryProxy&& other)
			{
				// Deallocate current data
				deallocateData();
				// Move allocator from other
				m_Allocator = std::move(other.m_Allocator);
				// Copy other data members
				m_Data = other.m_Data;
				m_Ownership = other.m_Ownership;
				m_Length = other.m_Length;
				m_Capacity = other.m_Capacity;
				// Set other object to null-state
				other.initialize();
			}
#endif
		public:
			/**
			 * @brief Default constructor.
			 * Sets object to a null-state by internally calling initialize method.
			 */
			MemoryProxy() { initialize(); }
#ifdef PCAPPP_HAVE_NULLPTR_T
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			explicit MemoryProxy(std::nullptr_t) { initialize(); }
#endif
			/**
			 * @brief General object constructor.
			 * Simply sets data field to provided values.
			 * @param[in] p Pointer to memory to take handle of.
			 * @param[in] length Size of provided memory (NOT in bytes).
			 * @param[in] ownership Indicator of ownership over provided memory.
			 */
			explicit MemoryProxy(const_pointer p, size length = 0, bool ownership = true) :
				m_Data(p), m_Length(length), m_Capacity(length), m_Allocator(), m_Ownership(ownership) {}
			/**
			 * @brief Copy constructor.
			 * Object is set to a null-state first. Then other allocator object is copied.
			 * Next the SafeToCopyCondition is checked on other.\n
			 * If check is passed new data allocated by copied allocator with same size as other.m_Capacity and then data is copied from other.\n
			 * Else object will be set in null-state.
			 * If allocator can't allocate new memory object will be set in null-state.
			 * @param[in] other The instance to make copy of.
			 */
			MemoryProxy(const MemoryProxy& other) 
			{ 
				initialize(); 
				copyDataFrom(other); 
			}
			/**
			 * @brief Copy assignment operator.
			 * Don't allows self assignment.\n
			 * Underlying data is deallocated first. Then other allocator object is copied.
			 * Next the SafeToCopyCondition is checked on other.\n
			 * If check is passed new data allocated by copied allocator with same size as other.m_Capacity and then data is copied from other.\n
			 * Else object will be set in null-state.
			 * If allocator can't allocate new memory object will be set in null-state.
			 * @param[in] other The instance to make copy of.
			 */
			MemoryProxy& operator=(const MemoryProxy& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				copyDataFrom(other);
				return *this;
			}
#ifdef ENABLE_CPP11_MOVE_SEMANTICS
			/**
			 * @brief Move constructor.
			 * Object is set to a null-state first. Then other allocator object is moved.
			 * Next all other data members are copied. In the end the initialize member function is called on other.\n
			 * This function is unavailable if ENABLE_CPP11_MOVE_SEMANTICS macro is not defined.
			 * @param[in:out] other The instance to move from.
			 */
			MemoryProxy(MemoryProxy&& other)
			{
				initialize();
				moveDataFrom(std::move(other));
			}
			/**
			 * @brief Move assignment operator.
			 * Don't allows self assignment.\n
			 * Underlying data is deallocated first. Then other allocator object is moved.
			 * Next all other data members are copied. In the end the initialize member function is called on other.\n
			 * This function is unavailable if ENABLE_CPP11_MOVE_SEMANTICS macro is not defined.
			 * @param[in:out] other The instance to move from.
			 */
			MemoryProxy& operator=(MemoryProxy&& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				moveDataFrom(std::move(other));
				return *this;
			}
#endif
			/**
			 * @brief Destructor.
			 * Deallocates underlying data if SafeToDeleteCondition is satisfied.
			 */
			~MemoryProxy() { deallocateData(); }
			/**
			 * @brief Returns known underlying data length (NOT capacity).
			 * This function is in MemoryProxyInterface function set.
			 * @return Known underlying data length (NOT capacity).
			 */
			inline size getLength() const PCAPPP_OVERRIDE { return m_Length; }
			/**
			 * @brief Returns known underlying data capacity (NOT length).
			 * This function is NOT in MemoryProxyInterface function set.
			 * @return Known underlying data capacity (NOT length).
			 */
			inline size getCapacity() const { return m_Capacity; }
			/**
			 * @brief Returns owning status of underlying data.
			 * @return true if object owns it's underlying data, false otherwise.
			 */
			inline bool isOwning() const PCAPPP_OVERRIDE { return m_Ownership; }
			/**
			 * @brief Returns pointer to the beginning of underlying data.
			 * @return Pointer to the beginning of underlying data.
			 */
			inline pointer get() PCAPPP_OVERRIDE { return m_Data; }
			/**
			 * @brief Returns pointer to the beginning of const-qualified underlying data.
			 * This overload is called in object is const-qualified.
			 * @return Pointer to the beginning of const-qualified underlying data.
			 */
			inline const_pointer get() const PCAPPP_OVERRIDE { return m_Data; }
			/**
			 * @brief The ownership release method.
			 * Firstly saves current data pointer in temporary pointer.
			 * Next internally calls initialize method.
			 * Then returns saved pointer.
			 * @return Pointer to the beginning of underlying data.
			 */
			pointer relese() override
			{
				typename Base::pointer old = m_Data;
				initialize();
				return old;
			}
			/**
			 * @brief Underlying data reset method.
			 * Firstly implicitly deallocates current data.
			 * Then if deallocation is successful set provided values as fields values of this object.
			 * Else sets object in null-state.
			 * @param[in] ptr Pointer to memory to take handle of.
			 * @param[in] length Size of provided memory (NOT in bytes).
			 * @param[in] ownership Indicator of ownership over provided memory.
			 * @return true if new data was successfully accepted, false otherwise (object is in null-state in that case).
			 */
			bool reset(pointer ptr, size length = 0, bool ownership = true) PCAPPP_OVERRIDE
			{
				if (deallocateData())
				{
					m_Data = ptr;
					m_Length = length;
					m_Capacity = length;
					m_Ownership = ownership;
					return true;
				}
				initialize();
				return false;
			}
			/**
			 * @brief Method to check if current object is in the null-state.
			 * @return true if object is NOT in the null-state, false otherwise.
			 */
			operator bool() const { return m_Data || m_Ownership || m_Length > 0 || m_Capacity > 0; }
			/**
			 * @brief Represents the read access facility to the underlying allocator object.
			 * @return Reference to underlying allocator object.
			 */
			inline typename Adapter::allocator_traits::allocator_type& getAllocator() const { return m_Allocator.getAllocator(); }
			/**
			 * @brief Represents the write access facility to the underlying allocator object.
			 * @param[in] allocator Reference to the new allocator object to be used.
			 */
			inline void setAllocator(typename Adapter::allocator_traits::allocator_type& allocator) const { return m_Allocator.setAllocator(allocator); }
			/**
			 * @brief Reallocates underlying data.
			 * If m_Capacity is greater or equal to newBufferLength - Immediately returns true.\n
			 * newBufferLength set to 0 interpreted as clear operation a.e. deallocate data and call zeroFields member function.\n
			 * In any other case firstly allocates memory for newBufferLength data entries. 
			 * Then sets per-byte value of that memory to initialValue. After that copies old data to beginning of new memory.
			 * At the end deallocates old data and sets data member fields to their corresponding values.\n
			 * Correctly handles the case when newBufferLength is less than m_Length -> only data that fits new memory will be copied and m_Length is changed.\n
			 * If old data can't be deallocated sets object to a null-state.
			 * @param[in] newBufferLength New size of data.
			 * @param[in] initialValue Per-byte initial value of new memory on allocation.
			 * @return true if operation ended successfully, false otherwise (you may expect that object is in null-state).
			 */
			bool reallocate(size newBufferLength, memory_value initialValue = 0) PCAPPP_OVERRIDE
			{
				// Provided zero length is interpreted as clear operation
				if (!newBufferLength)
				{
					deallocateData();
					zeroFields();
					return true;
				}
				// Immediately return if there is enough memory
				if (newBufferLength <= m_Capacity)
					return true;
				// Allocate new buffer of requested 
				// It is allocator's responsibility to handle memory allocation exceptions
				typename Base::pointer newBuffer = m_Allocator.allocate(newBufferLength); 
				// Check if new buffer was allocated
				if (!newBuffer) // Expect nullptr/NULL returned when exception thrown on allocation
					return false;
				// Clear new buffer
				std::memset(newBuffer, initialValue, newBufferLength * sizeof(typename Base::value_type));
				// Copy data from previous storage
				// If new storage length is smaller than previous -> copy only data that fits new storage size
				// else -> copy only old data
				if (SafeToCopyCondition())
					std::memcpy(newBuffer, m_Data, (newBufferLength < m_Length ? newBufferLength : m_Length) * sizeof(typename Base::value_type));
				// Deallocate old data
				if (!deallocateData())
				{
					zeroFields(); // Clear object data
					m_Allocator.deallocate(newBuffer); // Avoid memory leak
					return false;
				}
				// Set new data as current handled
				m_Data = newBuffer;
				// If buffer was reallocated to be smaller then data length must be corrected
				if (newBufferLength < m_Length)
					m_Length = newBufferLength;
				// New capacity is set to the actual size of memory stored 
				m_Capacity = newBufferLength;
				// We definitely own data now
				m_Ownership = true;
				return true;
			}
			/**
			 * @brief Clear underlying data and set object to a null-state.
			 * Internally calls reallocate(0).
			 * @return true if operation ended successfully, false otherwise.
			 */
			bool clear() override { return reallocate(0); }
			/**
			 * @brief Append memory capable of holding dataToAppendLen data entries and set it per-byte to initialValue on allocation.
			 * Appending 0 bytes is always a success.
			 * m_Length is corrected.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation ended successfully, false otherwise.
			 */
			bool append(size dataToAppendLen, memory_value initialValue = 0) override
			{
				// Append of 0 bytes is always a success
				if (!dataToAppendLen)
					return true;
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				// Set new data space to zero
				std::memset(m_Data + m_Length, initialValue, dataToAppendLen * sizeof(typename Base::value_type));
				// Increase current data length by new data length
				m_Length += dataToAppendLen;
				return true;
			}
			/**
			 * @brief Append memory capable of holding dataToAppendLen data entries and copy data from dataToAppend to it (concatenate).
			 * Appending 0 bytes is always a success.
			 * m_Length is corrected.
			 * @param[in] dataToAppend Buffer memory to be appended to current data.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @return true if operation ended successfully, false otherwise.
			 * @todo Add error msg
			 */
			bool append(const_pointer dataToAppend, size dataToAppendLen) override
			{
				// Append of 0 bytes from any memory is a success
				if (!dataToAppendLen)
					return true;
				// If memory to append is not presented it is an error
				if (!dataToAppend)
				{
					// TODO: Add error msg
					return false;
				}
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				// Copy memory from source data
				// dataToAppend may (if reallocate call does not allocate) point to some byte in current data so use std::memmove
				// m_Data + m_Length is points to where the old data past-the-end byte is
				std::memmove(m_Data + m_Length, dataToAppend, dataToAppendLen * sizeof(typename Base::value_type));
				// Increase current data length by new data length
				m_Length += dataToAppendLen;
				return true;
			}

			/**
			 * @page memory_indexes_in_insert_and_remove_operation Indexes in Insert and Remove operations
			 * 
			 * NOTE: Indexes in Insert and Remove operations.
			 * 
			 * Next table describes atIndex parameter value and corresponding normal array index.
			 * Normal working range for this operations is : [-length : length-1]
			 * \# - before-the-start index; \@ - pass-the-end index; 
			 * +================+=============+=========+===========+===========+=====+==========+==========+==================+
			 * | Normal Indexes |     \#      |    0    |     1     |     2     | ... | length-2 | length-1 |       \@         |
			 * +================+=============+=========+===========+===========+=====+==========+==========+==================+
			 * | Insert Indexes | insert back | 0       | 1         | 2         | ... | length-2 | length-1 | append operation |
			 * +----------------+-------------+---------+-----------+-----------+-----+----------+----------+------------------+
			 * | Insert Back    | not handled | -length | -length+1 | -length+2 | ... | -2       | -1       | not handled      |
			 * +----------------+-------------+---------+-----------+-----------+-----+----------+----------+------------------+
			 * | Remove Indexes | remove back | 0       | 1         | 2         | ... | length-2 | length-1 | not handled      |
			 * +----------------+-------------+---------+-----------+-----------+-----+----------+----------+------------------+
			 * | Remove Back    | not handled | -length | -length+1 | -length+2 | ... | -2       | -1       | not handled      |
			 * +----------------+-------------+---------+-----------+-----------+-----+----------+----------+------------------+
			 * 
			 * ASCII table generator: https://ozh.github.io/ascii-tables/
			 */

			/**
			 * @brief Handles negative values of atIndex for insert operation.
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 */
			bool insert_back(index atIndex, size dataToInsertLen, memory_value initialValue = 0)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "insert before last byte of current data"
				// and -m_Length to "insert before first byte of current data" (a.e. at the beginning)
				if (m_Length < (typename Base::size)(-atIndex))
					atIndex = -((typename Base::index)m_Length);
				// Compute normal index position that correspond to negative atIndex value
				const typename Base::index index = ((typename Base::index)m_Length) + atIndex;
				// Here index must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory
				std::memmove(m_Data + index + dataToInsertLen, m_Data + index, (-atIndex) * sizeof(typename Base::value_type));
				// Fill new data with zeros
				std::memset(m_Data + index, initialValue, dataToInsertLen * sizeof(typename Base::value_type));
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}
			/**
			 * @brief Inserts memory capable of holding dataToInsertLen data entries and set it per-byte to initialValue on allocation.
			 * Case with negative atIndex is handled correctly.\n
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool insert(index atIndex, size dataToInsertLen, memory_value initialValue = 0) PCAPPP_OVERRIDE
			{
				// Inserting 0 bytes is always a success
				if (!dataToInsertLen)
					return true;
				// If object has no data insert is equal to append
				if (!m_Length)
					return append(dataToInsertLen);
				// Handle cases when insertion must start from the back
				if (atIndex < 0)
					return insert_back(atIndex, dataToInsertLen, initialValue);
				// At this point atIndex must be in bound [0, m_Length]
				// where 0 correspond to "insert before first byte of current data"
				// and m_Length to "insert after last byte of current data" (a.e. append)
				if ((typename Base::size)atIndex >= m_Length)
					return append(dataToInsertLen);
				// Here atIndex must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, (m_Length - atIndex) * sizeof(typename Base::value_type));
				// Fill new data with zeros
				std::memset(m_Data + atIndex, initialValue, dataToInsertLen * sizeof(typename Base::value_type));
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}
			/**
			 * @brief Handles negative values of atIndex for insert operation (with data).
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 */
			bool insert_back(index atIndex, const_pointer dataToInsert, size dataToInsertLen)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "insert before last byte of current data"
				// and -m_Length to "insert before first byte of current data" (a.e. at the beginning)
				if (m_Length < (typename Base::size)(-atIndex))
					atIndex = -((typename Base::index)m_Length);
				// Compute normal index position that correspond to negative atIndex value
				const typename Base::index index = ((typename Base::index)m_Length) + atIndex;
				// Here index must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(m_Data + index + dataToInsertLen, m_Data + index, (-atIndex) * sizeof(typename Base::value_type));
				// Insert new data to it's place
				std::memcpy(m_Data + index, dataToInsert, dataToInsertLen * sizeof(typename Base::value_type));
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}
			/**
			 * @brief Inserts memory capable of holding dataToInsertLen data entries and copy data from dataToAppend to it.
			 * Case with negative atIndex is handled correctly.\n
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsert Buffer memory to be inserted to current data.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen) PCAPPP_OVERRIDE
			{
				// Inserting 0 bytes from any memory is always a success
				if (!dataToInsertLen)
					return true;
				// If memory to insert is not presented it is an error
				if (!dataToInsert)
				{
					// TODO: Add error msg
					return false;
				}
				// If object has no data insert is equal to append
				if (!m_Length)
					return append(dataToInsert, dataToInsertLen);
				// Handle cases when insertion must start from the back
				if (atIndex < 0)
					return insert_back(atIndex, dataToInsert, dataToInsertLen);
				// At this point atIndex must be in bound [0, m_Length]
				// where 0 correspond to "insert before first byte of current data"
				// and m_Length to "insert after last byte of current data" (a.e. append)
				if ((typename Base::size)atIndex >= m_Length)
					return append(dataToInsert, dataToInsertLen);
				// Here atIndex must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, (m_Length - atIndex) * sizeof(typename Base::value_type));
				// Insert new data to it's place
				std::memcpy(m_Data + atIndex, dataToInsert, dataToInsertLen * sizeof(typename Base::value_type));
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}
			/**
			 * @brief Handles negative values of atIndex for remove operation.
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 */
			bool remove_back(index atIndex, size numOfBytesToRemove)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "clear last byte of data"
				// and -m_Length to "clear from beginning of data"
				// Don't handle the cases when atIndex is in (-Inf,-m_Length)
				if (m_Length < (typename Base::size)(-atIndex))
					return true;
				// Compute normal index position that correspond to negative atIndex value
				const typename Base::index index = ((typename Base::index)m_Length) + atIndex;
				// Here index must be in bound [0, m_Length)
				// End of data to clear must be in (atIndex, m_Length]
				if ((typename Base::size)index + numOfBytesToRemove >= m_Length) // There is no tail memory to move
				{	// Just shrink current length to remaining data
					m_Length = index;
				}
				else // There is tail memory to move
				{	// Move tail memory to new place
					std::memmove(m_Data + index, m_Data + index + numOfBytesToRemove, (m_Length - index - numOfBytesToRemove) * sizeof(typename Base::value_type));
					m_Length -= numOfBytesToRemove;
				}
				return true;
			}
			/**
			 * @brief Removes memory capable of holding numOfBytesToRemove data entries starting from atIndex.
			 * Actually makes no reallocations just shrinks m_Length value (tail bytes are handled correctly).\n
			 * Case with negative atIndex is handled correctly.\n
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 * @param[in] atIndex Index from which removal take place.
			 * @param[in] numOfBytesToRemove Size of data to be removed.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool remove(index atIndex, size numOfBytesToRemove) PCAPPP_OVERRIDE
			{
				// Removing zero bytes or removing from empty data is always a success
				if (!(numOfBytesToRemove && m_Length))
					return true;
				// Handle cases when removal must start from the back
				if (atIndex < 0)
					return remove_back(atIndex, numOfBytesToRemove);
				// At this point atIndex must be in bound [0, m_Length)
				// where 0 correspond to "clear from beginning of data"
				// and m_Length-1 to "clear last byte of data"
				if (atIndex >= m_Length)
					return true;
				// End of data to clear must be in (atIndex, m_Length]
				if ((typename Base::size)atIndex + numOfBytesToRemove >= m_Length) // There is no tail memory to move
				{	// Just shrink current length to remaining data
					m_Length = atIndex;
				}
				else // There is tail memory to move
				{	// Move tail memory to new place
					std::memmove(m_Data + atIndex, m_Data + atIndex + numOfBytesToRemove, (m_Length - atIndex - numOfBytesToRemove) * sizeof(typename Base::value_type));
					m_Length -= numOfBytesToRemove;
				}
				return true;
			}
		protected:
			// Don't change the order of data members. 
			// It is optimized for 64 byte cache line access.
			// See: "Size comparison" note in MemoryProxy.h
			pointer m_Data;
			size m_Length, m_Capacity;
			mutable Adapter m_Allocator; // Don't move this data member. By default it may have size from 1 to 16 bytes (1,8,16) depending on defines.
			bool m_Ownership;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_CONTENTAWAREMEMORYPROXY */