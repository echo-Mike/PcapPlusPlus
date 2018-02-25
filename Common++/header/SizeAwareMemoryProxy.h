#ifndef PCAPPP_SIZEAWAREMEMORYPROXY
#define PCAPPP_SIZEAWAREMEMORYPROXY

#include <cstdlib>
#include <exception>

#include "CPP11.h"
#include "MemoryUtils.h"
#include "MemoryProxyInterface.h"
#include "AllocatorPointerPair.h"

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
		 * @brief Size aware memory proxy class specialization. 
		 * This specialization held only one counter for internal data length.\n
		 * m_Length - represents the length of known data in underlying storage and the size of underlying storage.\n
		 * This is the safe implementation of old memory handling algorithm. But logic is not similar and therefore the test values.
		 * @tparam Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
		 */ 
		template < typename Allocator, typename traits = allocator_traits<Allocator> >
		class SizeAwareMemoryProxy :
			public MemoryProxyInterface< typename traits::value_type >
		{
		public:
			/**
			 * Base type of this class.
			 */
			typedef MemoryProxyInterface< typename traits::value_type > Base;
			/**
			 * Represents compressed pair of allocator adapter and data pointer.
			 */
			typedef Implementation::AllocatorPointerPair<Allocator> allocator_pointer_pair_t;
		protected:
			/**
			 * @brief Represents the read-write access facility to the underlying data pointer.
			 * @return Reference to underlying data pointer.
			 */
			inline typename allocator_pointer_pair_t::pointer& getPointer() { return m_Pair.get_pointer(); }
			/**
			 * @brief Represents the read access facility to the underlying data pointer.
			 * This overload is selected by compiler if object is const-qualified.
			 * @return Copy of underlying data pointer.
			 */
			inline typename allocator_pointer_pair_t::pointer getPointer() const { return m_Pair.get_pointer(); }
			/**
			 * @brief Checks the condition in which underlying data may be safely deallocated.
			 * @return true if condition is satisfied, false otherwise.
			 */
			inline bool SafeToDeleteCondition() { return m_Ownership && getPointer(); }
			/**
			 * @brief Checks the condition in which underlying data may be safely copied to another object.
			 * @return true if condition is satisfied, false otherwise.
			 */
			inline bool SafeToCopyCondition() { return getPointer() && m_Length > 0; }
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
						getAllocator().deallocate(getPointer());
				}
				catch (const std::exception&)
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
			 * @brief Setups object to special null-state.
			 * Basically zeroes all fields, no data is touched.\n
			 * initialize method of allocator object is called.
			 */
			inline void initialize()
			{
				// Set all fields to their initial values
				getPointer() = PCAPPP_NULLPTR;
				m_Length = 0;
				m_Ownership = false;
			}
			/**
			 * @brief Makes current object a copy of the other.
			 * Underlying data is deallocated first. Then other allocator object is copied.
			 * Next the SafeToCopyCondition is checked on other.\n
			 * If check is passed new data allocated by copied allocator with same size as other.m_Length and then data is copied from other.\n
			 * Else object will be set in null-state.
			 * If allocator can't allocate new memory object will be set in null-state.
			 * @param[in] other Object to make copy of.
			 * @return true if successful copy of other was made, false otherwise.
			 */
			inline bool copyDataFrom(const SizeAwareMemoryProxy& other)
			{
				// Deallocate current data
				deallocateData();
				// Check that copying of data is safe
				if (other.SafeToCopyCondition())
				{
					// Allocate new data
					// It is allocator's responsibility to handle memory allocation exceptions
					getPointer() = getAllocator().allocate(other.m_Length);
					// Check if new buffer was allocated
					if (m_Data == PCAPPP_NULLPTR) {// Expect nullptr/NULL returned when execption thrown on allocation
						zeroFields();
						return false;
					}
					// Copy whole data to this object memory
					std::memcpy(getPointer(), other.getPointer(), other.m_Length * sizeof(typename Base::value_type));
					// We definitely own data now
					m_Ownership = true;
					// Copy length value
					m_Length = other.m_Length;
					return true;
				}
				else // Other object must be in some null-state
				{	// Put our object in proper null-state
					initialize();
					return false;
				}
			}

			/**
			 * @brief Moves data from other.
			 * Underlying data is deallocated first. Then other allocator object is moved.
			 * Next all other data members are copied. In the end initialize member function is called on other.\n
			 * @param[in:out] other Object to move from.
			 */
			inline void moveDataFrom(SizeAwareMemoryProxy& other)
			{
				// Deallocate current data
				deallocateData();
				// Move stored pair
				m_Pair = PCAPPP_MOVE(other.m_Pair);
				// Copy other data members
				m_Ownership = other.m_Ownership;
				m_Length = other.m_Length;
				// Set other object to null-state
				other.initialize();
			}

		public:
			/**
			 * @brief Default constructor.
			 * Sets object to a null-state by internally calling initialize method.
			 */
			SizeAwareMemoryProxy() { initialize(); }
#ifdef PCAPPP_HAVE_NULLPTR_T
			/**
			 * @brief Special case constructor for nullptr.
			 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
			 */
			explicit SizeAwareMemoryProxy(std::nullptr_t) { initialize(); }
#endif
			/**
			 * @brief General object constructor.
			 * Simply sets data field to provided values.
			 * @param[in] p Pointer to memoty to take handle of.
			 * @param[in] length Size of provided memory (NOT in bytes).
			 * @param[in] ownership Indicator of ownership over provided memory.
			 */
			explicit SizeAwareMemoryProxy(const_pointer p, size length = 0, bool ownership = true, Allocator alloc = Allocator()) :
				m_Pair(alloc, p), m_Length(length), m_Ownership(ownership) {}
			/**
			 * @brief Copy constructor.
			 * Object is set to a null-state first. Then other allocator object is copied.
			 * Next the SafeToCopyCondition is checked on other.\n
			 * If check is passed new data allocated by copied allocator with same size as other.m_Length and then data is copied from other.\n
			 * Else object will be set in null-state.
			 * If allocator can't allocate new memory object will be set in null-state.
			 * @param[in] other The instance to make copy of.
			 */
			SizeAwareMemoryProxy(const SizeAwareMemoryProxy& other)
			{
				initialize();
				copyDataFrom(other);
			}
			/**
			 * @brief Copy assignment operator.
			 * Don't allows self assignment.\n
			 * Underlying data is deallocated first. Then other allocator object is copied.
			 * Next the SafeToCopyCondition is checked on other.\n
			 * If check is passed new data allocated by copied allocator with same size as other.m_Length and then data is copied from other.\n
			 * Else object will be set in null-state.
			 * If allocator can't allocate new memory object will be set in null-state.
			 * @param[in] other The instance to make copy of.
			 */
			SizeAwareMemoryProxy& operator=(const SizeAwareMemoryProxy& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				copyDataFrom(other);
				return *this;
			}
			/**
			 * @brief Move constructor.
			 * Object is set to a null-state first. Then other allocator object is moved.
			 * Next all other data members are copied. In the end the initialize member function is called on other.\n
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_CONSTRUCTOR(SizeAwareMemoryProxy)
			{
				initialize();
				moveDataFrom(PCAPPP_MOVE_OTHER);
			}
			/**
			 * @brief Move assignment operator.
			 * Don't allows self assignment.\n
			 * Underlying data is deallocated first. Then other allocator object is moved.
			 * Next all other data members are copied. In the end the initialize member function is called on other.\n
			 * @param[in:out] other The instance to move from.
			 */
			PCAPPP_MOVE_ASSIGNMENT(SizeAwareMemoryProxy)
			{
				// Handle self assignment case
				if (this == &PCAPPP_MOVE_OTHER)
					return *this;
				moveDataFrom(PCAPPP_MOVE_OTHER);
				return *this;
			}
			/**
			 * @brief Destructor.
			 * Deallocates underlying data if SafeToDeleteCondition is satisfied.
			 */
			~SizeAwareMemoryProxy() { deallocateData(); }

			/**
			 * @brief Returns known underlying data length.
			 * @return Known underlying data length.
			 */
			inline size getLength() const PCAPPP_OVERRIDE { return m_Length; }
			/**
			 * @brief Returns owning status of underlying data.
			 * @return true if object owns it's underlying data, false otherwise.
			 */
			inline bool isOwning() const PCAPPP_OVERRIDE { return m_Ownership; }
			/**
			 * @brief Returns pointer to the beginning of underlying data.
			 * @return Pointer to the beginning of underlying data.
			 */
			inline pointer get() PCAPPP_OVERRIDE { return m_Pair.get_pointer(); }
			/**
			 * @brief Returns pointer to the beginning of const-qualified underlying data.
			 * This overload is called in object is const-qualified.
			 * @return Pointer to the beginning of const-qualified underlying data.
			 */
			inline const_pointer get() const PCAPPP_OVERRIDE { return m_Pair.get_pointer(); }
			/**
			 * @brief The ownership release method.
			 * Firstly saves current data pointer in temporary pointer.
			 * Next internally calls initialize method.
			 * Then returns saved pointer.
			 * @return Pointer to the beginning of underlying data.
			 */
			pointer release() PCAPPP_OVERRIDE
			{
				typename Base::pointer old = getPointer();
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
					getPointer() = ptr;
					m_Length = length;
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
			operator bool() const { return getPointer() || m_Ownership || m_Length > 0; }
			/**
			 * @brief Represents the read access facility to the underlying allocator object.
			 * @return Reference to underlying allocator object.
			 */
			inline typename allocator_pointer_pair_t::Adapter& getAllocator() { return m_Pair.get_allocator(); }
			/**
			 * @brief Represents the read access facility to the underlying allocator object.
			 * This overload is selected by compiler if object is const-qualified.
			 * @return Reference to const-qualified underlying allocator object.
			 */
			inline const typename allocator_pointer_pair_t::Adapter& getAllocator() const { return m_Pair.get_allocator(); }
			/**
			 * @brief Reallocates underlying data.
			 * If m_Length is equal to newBufferLength - Immediately returns true.\n
			 * newBufferLength set to 0 interpreted as clear operation a.e. deallocate data and call zeroFields member function.\n
			 * In any other case firstly allocates memory for newBufferLength data entries. 
			 * Then sets per-byte value of that memory to initialValue. After that copies old data to beginig of new memory.
			 * At the end deallocates old data and sets data member fields to their corresponding values.\n
			 * Correctly handles the case when newBufferLength is less than m_Length -> only data that fits new memory will be copied.\n
			 * Will perform a deallocation attempt only if m_Length is greater than zero.\n
			 * If old data can't be deallocated sets object to a null-state.
			 * @param[in] newBufferLength New size of data.
			 * @param[in] initialValue Per-byte initial value of new memory on allocation.
			 * @return true if operation ended successfully, false otherwise (you may expect that object is in null-state).
			 */
			bool reallocate(size newBufferLength, memory_value initialValue = 0) PCAPPP_OVERRIDE
			{
				// Immediately return if there is enough memory
				if (newBufferLength == m_Length)
					return true;
				// Provided zero length is interpreted as clear operation
				if (!newBufferLength)
				{
					deallocateData();
					initialize();
					return true;
				}
				// Allocate new buffer of requested 
				// It is allocator's responsibility to handle memory allocation exceptions
				typename Base::pointer newBuffer = getAllocator().allocate(newBufferLength);
				if (!newBuffer) // Expect nullptr/NULL returned when execption thrown on allocation
					return false;
				// Clear new buffer
				std::memset(newBuffer, initialValue, newBufferLength * sizeof(typename Base::value_type));
				// Copy data from previous storage
				// If new storage length is smaller than previous -> copy only data that fits new storage size
				// else -> copy whole old data
				if (SafeToCopyCondition())
					std::memcpy(newBuffer, getPointer(), (newBufferLength < m_Length ? newBufferLength : m_Length) * sizeof(typename Base::value_type));
				// We will deallocate memory only if we have some
				if (m_Length > 0)
				{
					// Deallocate old data
					if (!deallocateData())
					{
						initialize(); // Clear object data
						getAllocator().deallocate(newBuffer); // Avoid memory leak
						return false;
					}
				}
				// Set new data as current handled
				getPointer() = newBuffer;
				// Set curent known length of buffer
				m_Length = newBufferLength;
				// We definitely own data now
				m_Ownership = true;
				return true;
			}
			/**
			 * @brief Clear underlying data and set object to a null-state.
			 * Internally calls reallocate(0).
			 * @return true if operation ended successfully, false otherwise.
			 */
			bool clear() PCAPPP_OVERRIDE { return reallocate(0); }
			/**
			 * @brief Append memory capable of holding dataToAppendLen data entries and set it per-byte to initialValue on allocation.
			 * Appending 0 bytes is always a success.
			 * m_Length is corrected.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation ended successfully, false otherwise.
			 */
			bool append(size dataToAppendLen, memory_value initialValue = 0) PCAPPP_OVERRIDE
			{
				// Append of 0 bytes is always a success
				if (!dataToAppendLen)
					return true;
				// Save old length
				const typename Base::size oldLength = m_Length;
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				// Set new data space to zero
				std::memset(getPointer() + oldLength, initialValue, dataToAppendLen * sizeof(typename Base::value_type));
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
			bool append(const_pointer dataToAppend, size dataToAppendLen) PCAPPP_OVERRIDE
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
				// Save old length
				const typename Base::size oldLength = m_Length;
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				// There is no way that reallocate call doesn't reallocate current storage
				// therefore we can safely use memcpy instead of memove
				std::memcpy(getPointer() + oldLength, dataToAppend, dataToAppendLen * sizeof(typename Base::value_type));
				return true;
			}
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
				std::memmove(getPointer() + index + dataToInsertLen, getPointer() + index, (-atIndex) * sizeof(typename Base::value_type));
				// Fill new data with zeros
				std::memset(getPointer() + index, initialValue, dataToInsertLen * sizeof(typename Base::value_type));
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
				// Save old length
				const typename Base::size oldLength = m_Length;
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(getPointer() + atIndex + dataToInsertLen, getPointer() + atIndex, (oldLength - atIndex) * sizeof(typename Base::value_type));
				// Fill new data with zeros
				std::memset(getPointer() + atIndex, initialValue, dataToInsertLen * sizeof(typename Base::value_type));
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
				std::memmove(getPointer() + index + dataToInsertLen, getPointer() + index, (-atIndex) * sizeof(typename Base::value_type));
				// Insert new data to it's place
				std::memcpy(getPointer() + index, dataToInsert, dataToInsertLen * sizeof(typename Base::value_type));
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
				// where 0 correspond to "insert before fisrt byte of current data"
				// and m_Length to "insert after last byte of current data" (a.e. append)
				if ((typename Base::size)atIndex >= m_Length)
					return append(dataToInsert, dataToInsertLen);
				// Here atIndex must be in bound [0, m_Length)
				// Save old length
				const typename Base::size oldLength = m_Length;
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(getPointer() + atIndex + dataToInsertLen, getPointer() + atIndex, (oldLength - atIndex) * sizeof(typename Base::value_type));
				// Insert new data to it's place
				std::memcpy(getPointer() + atIndex, dataToInsert, dataToInsertLen * sizeof(typename Base::value_type));
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
				{	// Just shrink current buffer length to remaining data
					return reallocate(index);
				}
				else // There is tail memory to move
				{	// Move tail memory to new place
					std::memmove(getPointer() + index, getPointer() + index + numOfBytesToRemove, (m_Length - index - numOfBytesToRemove) * sizeof(typename Base::value_type));
					// Shrink current buffer
					return reallocate(m_Length - numOfBytesToRemove);
				}
			}
			/**
			 * @brief Removes memory capable of holding numOfBytesToRemove data entries starting from atIndex.
			 * Reallocates memory (tail bytes are handled correctly).\n
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
					return reallocate(atIndex);
				}
				else // There is tail memory to move
				{	// Move tail memory to new place
					std::memmove(getPointer() + atIndex, getPointer() + atIndex + numOfBytesToRemove, (m_Length - atIndex - numOfBytesToRemove) * sizeof(typename Base::value_type));
					// Shrink current buffer
					return reallocate(m_Length - numOfBytesToRemove);
				}
			}
		protected:
			allocator_pointer_pair_t m_Pair;
			size m_Length;
			bool m_Ownership;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_SIZEAWAREMEMORYPROXY */