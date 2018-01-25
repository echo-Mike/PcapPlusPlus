#ifndef PCAPPP_SIZEAWAREMEMORYPROXY
#define PCAPPP_SIZEAWAREMEMORYPROXY

#include <cstdint>
#include <exception>

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

		template < typename T, typename Allocator >
		class MemoryProxy< T, Allocator, MemoryProxyTags::SizeAwareTag > :
			public MemoryProxyInterface<T, Allocator, MemoryProxyTags::SizeAwareTag >
		{
		public:
			typedef MemoryProxyInterface<T, Allocator, MemoryProxyTags::SizeAwareTag > Base;
		protected:

			inline bool SafeToDeleteCondition() { return m_Ownership && m_Data; }

			inline bool SafeToCopyCondition() { return m_Data && m_Length > 0; }

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

			inline void zeroFields()
			{
				// Set all fields to their initial values
				m_Data = nullptr;
				m_Length = 0;
				m_Ownership = false;
			}

			inline void initialize()
			{
				m_Allocator.initialize();
				zeroFields();
			}

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
					m_Data = m_Allocator.allocate(other.m_Length);
					// Check if new buffer was allocated
					if (!m_Data) {// Expect nullptr/NULL returned when execption thrown on allocation
						zeroFields();
						return false;
					}
					// Copy whole data to this object memory
					std::memcpy(m_Data, other.m_Data, other.m_Length * sizeof(typename Base::value_type));
					// We definitely own data now
					m_Ownership = true;
					// Copy length value
					m_Length = other.m_Length;
					return true;
				}
				else // Other object must be in some null-state
				{	// Put our object in proper null-state
					zeroFields();
					return false;
				}
			}

#ifdef ENABLE_CPP11_MOVE_SEMANTICS

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
				// Set other object to null-state
				other.initialize();
			}
#endif
		public:

			MemoryProxy() { initialize(); }

// In case of unsupported std::nullptr_t nullptr will be a macro def (from CPP11.h)
#ifndef nullptr

			explicit MemoryProxy(std::nullptr_t) { initialize(); }
#endif

			explicit MemoryProxy(pointer p, size length = 0, bool ownership = true) :
				m_Allocator(), m_Data(p), m_Length(length), m_Ownership(ownership) {}

			MemoryProxy(const MemoryProxy& other) { copyDataFrom(other); }

			MemoryProxy& operator=(const MemoryProxy& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				copyDataFrom(other);
				return *this;
			}

#ifdef ENABLE_CPP11_MOVE_SEMANTICS

			MemoryProxy(MemoryProxy&& other) { moveDataFrom(std::move(other)); }

			MemoryProxy& operator=(MemoryProxy&& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				moveDataFrom(std::move(other));
				return *this;
			}
#endif

			~MemoryProxy() { deallocateData(); }

			inline size getLength() const override { return m_Length; }

			inline bool isOwning() const override { return m_Ownership; }

			inline pointer get() override { return m_Data; }

			inline const_pointer get() const override { return m_Data; }

			pointer relese() override
			{
				typename Base::pointer old = m_Data;
				initialize();
				return old;
			}

			bool reset(pointer ptr, size length = 0, bool ownership = true) override
			{
				if (deallocateData())
				{
					m_Data = ptr;
					m_Length = length;
					m_Ownership = ownership;
					return true;
				}
				initialize();
				return false;
			}

			operator bool() const { return m_Data || m_Ownership || m_Length > 0; }

			inline typename Adapter::allocator_traits::allocator_type& getAllocator() const { return m_Allocator.getAllocator(); }

			inline void setAllocator(typename Adapter::allocator_traits::allocator_type& allocator) const { return m_Allocator.setAllocator(allocator); }

			bool reallocate(size newBufferLength, memory_value initialValue = 0) override
			{
				// Immediately return if there is enough memory
				if (newBufferLength == m_Length)
					return true;
				// Provided zero length is interpreted as clear operation
				if (!newBufferLength)
				{
					deallocateData();
					zeroFields();
					return true;
				}
				// Allocate new buffer of requested 
				// It is allocator's responsibility to handle memory allocation exceptions
				typename Base::pointer newBuffer = m_Allocator.allocate(newBufferLength);
				if (!newBuffer) // Expect nullptr/NULL returned when execption thrown on allocation
					return false;
				// Clear new buffer
				std::memset(newBuffer, initialValue, newBufferLength * sizeof(typename Base::value_type));
				// Copy data from previous storage
				// If new storage length is smaller than previous -> copy only data that fits new storage size
				// else -> copy whole old data
				if (SafeToCopyCondition())
					std::memcpy(newBuffer, m_Data, (newBufferLength < m_Length ? newBufferLength : m_Length) * sizeof(typename Base::value_type));
				// We will deallocate memory only if we have some
				if (m_Length > 0)
				{
					// Deallocate old data
					if (!deallocateData())
					{
						zeroFields(); // Clear object data
						m_Allocator.deallocate(newBuffer); // Avoid memory leak
						return false;
					}
				}
				// Set new data as current handled
				m_Data = newBuffer;
				// Set curent known length of buffer
				m_Length = newBufferLength;
				// We definitely own data now
				m_Ownership = true;
				return true;
			}

			bool clear() override { return reallocate(0); }

			bool append(size dataToAppendLen, memory_value initialValue = 0) override
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
				std::memset(m_Data + oldLength, initialValue, dataToAppendLen * sizeof(typename Base::value_type));
				return true;
			}

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
				// Save old length
				const typename Base::size oldLength = m_Length;
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				// There is no way that reallocate call doesn't reallocate current storage
				// therefore we can safly use memcpy instead of memove
				std::memcpy(m_Data + oldLength, dataToAppend, dataToAppendLen * sizeof(typename Base::value_type));
				return true;
			}

			bool insert_back(index atIndex, size dataToInsertLen, memory_value initialValue = 0)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "insert before last byte of current data"
				// and -m_Length to "insert before first byte of current data" (a.e. at the begining)
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
				return true;
			}

			bool insert(index atIndex, size dataToInsertLen, memory_value initialValue = 0) override
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
				// where 0 correspond to "insert before fisrt byte of current data"
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
				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, (oldLength - atIndex) * sizeof(typename Base::value_type));
				// Fill new data with zeros
				std::memset(m_Data + atIndex, initialValue, dataToInsertLen * sizeof(typename Base::value_type));
				return true;
			}

			bool insert_back(index atIndex, const_pointer dataToInsert, size dataToInsertLen)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "insert before last byte of current data"
				// and -m_Length to "insert before first byte of current data" (a.e. at the begining)
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
				return true;
			}

			bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen) override
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
				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, (oldLength - atIndex) * sizeof(typename Base::value_type));
				// Insert new data to it's place
				std::memcpy(m_Data + atIndex, dataToInsert, dataToInsertLen * sizeof(typename Base::value_type));
				return true;
			}

			bool remove_back(index atIndex, size numOfBytesToRemove)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "clear last byte of data"
				// and -m_Length to "clear from begining of data"
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
					std::memmove(m_Data + index, m_Data + index + numOfBytesToRemove, (m_Length - index - numOfBytesToRemove) * sizeof(typename Base::value_type));
					// Shrink current buffer
					return reallocate(m_Length - numOfBytesToRemove);
				}
			}

			bool remove(index atIndex, size numOfBytesToRemove) override
			{
				// Removing zero bytes or removing from empty data is always a success
				if (!(numOfBytesToRemove && m_Length))
					return true;
				// Handle cases when removal must start from the back
				if (atIndex < 0)
					return remove_back(atIndex, numOfBytesToRemove);
				// At this point atIndex must be in bound [0, m_Length)
				// where 0 correspond to "clear from begining of data"
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
					std::memmove(m_Data + atIndex, m_Data + atIndex + numOfBytesToRemove, (m_Length - atIndex - numOfBytesToRemove) * sizeof(typename Base::value_type));
					// Shrink current buffer
					return reallocate(m_Length - numOfBytesToRemove);
				}
			}

		protected:
			mutable Adapter m_Allocator;
			pointer m_Data;
			size m_Length;
			bool m_Ownership;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_SIZEAWAREMEMORYPROXY */