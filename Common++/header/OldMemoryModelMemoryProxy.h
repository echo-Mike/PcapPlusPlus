#ifndef PCAPPP_OLDMEMORYMODELMEMORYPROXY
#define PCAPPP_OLDMEMORYMODELMEMORYPROXY

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

		template < typename Allocator >
		class MemoryProxy< Allocator, MemoryProxyTags::OldMemoryModelTag > :
			public MemoryProxyInterface< Allocator >
		{
		public:
			typedef MemoryProxyInterface< Allocator > Base;
		protected:

			inline void zeroFields()
			{
				// Set all fields to their initial values
				m_Data = nullptr;
				m_Length = 0;
				m_Ownership = true;
				m_DataSet = false;
			}

			inline void initialize()
			{
				m_Allocator.initialize();
				zeroFields();
			}

			bool setData(const_pointer p, size length)
			{
				if (m_Data != nullptr && m_Ownership)
					m_Allocator.deallocate(m_Data);
				m_Data = (typename Base::pointer)p;
				m_Length = length;
				m_DataSet = true;
				return true;
			}

			inline bool copyDataFrom(const MemoryProxy& other, bool allocateData = true)
			{
				if (!other.m_DataSet)
					return false;

				if (allocateData)
				{
					m_Ownership = true;
					m_Data = m_Allocator.allocate(other.m_Length);
					// This check does not done in original code
					/*if (!m_Data) {
						// Expect nullptr/NULL returned when execption thrown on allocation
						clear();
						return false;
					}*/
					m_Length = other.m_Length;
				}
				std::memcpy(m_Data, other.m_Data, other.m_Length * sizeof(typename Base::value_type));
				m_DataSet = true;
			}

		public:

			MemoryProxy() { initialize(); }

			MemoryProxy(const_pointer p, size length, bool ownership) :
				m_Allocator() 
			{
				initialize();
				m_Ownership = ownership;
				setData(p, length);
			}

			MemoryProxy(const MemoryProxy& other) { copyDataFrom(other); }

			MemoryProxy& operator=(const MemoryProxy& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				if (m_Data != nullptr)
					m_Allocator.deallocate(m_Data);
				m_DataSet = false;
				copyDataFrom(other);
				return *this;
			}

			~MemoryProxy() 
			{ 
				if (m_Ownership)
					m_Allocator.deallocate(m_Data);
			}

			inline size getLength() const override { return m_Length; }

			inline bool isOwning() const override { return m_Ownership; }

			inline pointer get() override { return m_Data; }

			inline const_pointer get() const override { return m_Data; }

			pointer relese() override
			{	// This function logic does not presented in original code
				/*typename Base::pointer old = m_Data;
				initialize();
				return old;*/
				return nullptr;
			}

			bool reset(pointer ptr, size length = 0, bool ownership = true) override
			{	// This function logic does not presented in original code
				/*
				if (deallocateData())
				{
					m_Data = ptr;
					m_Length = length;
					m_Ownership = ownership;
					return true;
				}
				initialize();
				*/
				return false;
			}

			operator bool() const 
			{	// This function logic does not presented in original code
				// return m_Data || m_Ownership || m_Length > 0;
				return false;
			}

			inline typename Adapter::allocator_traits::allocator_type& getAllocator() const { return m_Allocator.getAllocator(); }

			inline void setAllocator(typename Adapter::allocator_traits::allocator_type& allocator) const { return m_Allocator.setAllocator(allocator); }

			bool reallocate(size newBufferLength, memory_value initialValue = 0) override
			{
				if (newBufferLength == m_Length)
					return true;

				if (newBufferLength < m_Length)
				{
					// TODO: Add error msg
					//LOG_ERROR("Cannot reallocate raw packet to a smaller size. Current data length: %d; requested length: %d", m_RawDataLen, (int)newBufferLength);
					return false;
				}

				typename Base::pointer newBuffer = m_Allocator.allocate(newBufferLength);
				std::memset(newBuffer, initialValue, newBufferLength * sizeof(typename Base::value_type));
				std::memcpy(newBuffer, m_Data, m_Length * sizeof(typename Base::value_type));
				if (m_Ownership)
					m_Allocator.deallocate(m_Data);

				m_Ownership = true;
				m_Data = newBuffer;
				return true;
			}

			bool clear() override
			{
				if (m_Data != nullptr)
					m_Allocator.deallocate(m_Data);
				m_Data = nullptr;
				m_Length = 0;
				m_DataSet = false;
				return true;
			}

			bool append(size dataToAppendLen, memory_value initialValue = 0) override
			{
				std::memset(m_Data + m_Length, initialValue, dataToAppendLen * sizeof(typename Base::value_type));
				m_Length += dataToAppendLen;
				return true;
			}

			bool append(const_pointer dataToAppend, size dataToAppendLen) override
			{
				std::memcpy(m_Data + m_Length, dataToAppend, dataToAppendLen * sizeof(typename Base::value_type));
				m_Length += dataToAppendLen;
				return true;
			}

			bool insert(index atIndex, size dataToInsertLen, memory_value initialValue = 0) override
			{
				typename Base::index index = (typename Base::index)m_Length - 1;
				while (index >= atIndex)
				{
					m_Data[index + dataToInsertLen] = m_Data[index];
					--index;
				}

				std::memset(m_Data + atIndex, initialValue, dataToInsertLen * sizeof(typename Base::value_type));
				m_Length += dataToInsertLen;
				return true;
			}

			bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen) override
			{
				typename Base::index index = (typename Base::index)m_Length - 1;
				while (index >= atIndex)
				{
					m_Data[index + dataToInsertLen] = m_Data[index];
					--index;
				}

				std::memcpy(m_Data + atIndex, dataToInsert, dataToInsertLen * sizeof(typename Base::value_type));
				m_Length += dataToInsertLen;
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

				if ((atIndex + (typename Base::index)numOfBytesToRemove) > m_Length)
				{
					// TODO: Add error msg
					//LOG_ERROR("Remove section is out of raw packet bound");
					return false;
				}

				typename Base::index index = atIndex;
				while (index < (m_Length - numOfBytesToRemove))
				{
					m_Data[index] = m_Data[index + numOfBytesToRemove];
					++index;
				}

				m_Length -= numOfBytesToRemove;
				return true;
			}

		protected:
			// Don't change the order of data members. 
			// It is optimised for 64 byte cache line access.
			pointer m_Data;
			size m_Length;
			mutable Adapter m_Allocator; // Don't move this data member. By default it may have size from 1 to 16 bytes (1,8,16) depending on defines.
			bool m_Ownership, m_DataSet;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_OLDMEMORYMODELMEMORYPROXY */