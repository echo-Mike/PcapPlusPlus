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
		/**
		 * @brief Old memory handling model memory proxy class specialisation. 
		 * This specialisation represents the way memory was handled in old implementation of RawPacket.\n
		 * Old memory handling algorithm was carefully transferred to the new framework and slightly tweaked to accept custom allocators from users.\n
		 * The field correspondence of this class and original RawPacket implementation:  
		 * * m_pRawData -> m_Data  
		 * * m_RawDataLen -> m_Length  
		 * * m_RawPacketSet -> m_DataSet  
		 * * m_DeleteRawDataAtDestructor -> m_Ownership  
		 *   
		 * @tparam[in] Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
		 */ 
		template < typename Allocator >
		class MemoryProxy< Allocator, MemoryProxyTags::OldMemoryModelTag > :
			public MemoryProxyInterface< Allocator >
		{
		public:
			/**
			 * Base type of this class.
			 */
			typedef MemoryProxyInterface< Allocator > Base;
		protected:
			/**
			 * @brief Setups object to a null-state.
			 * Basically zeroes all fields, no data is touched.
			 */
			inline void zeroFields()
			{
				// Set all fields to their initial values
				m_Data = nullptr;
				m_Length = 0;
				m_Ownership = true;
				m_DataSet = false;
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
			 * @brief Sets provided data as new data to handle. 
			 * Frees the current raw data first.
			 * State of m_Ownership flag is not touched.\n
			 * This function is a partial implementation of old RawPacket::setRawData algorithm.
			 * @param[in] p A pointer to the new data.
			 * @param[in] length The new data length.
			 * @return true if raw data was set successfully, false otherwise.
			 */
			bool setData(const_pointer p, size length)
			{
				if (m_Data != nullptr && m_Ownership)
					m_Allocator.deallocate(m_Data);
				m_Data = (typename Base::pointer)p;
				m_Length = length;
				m_DataSet = true;
				return true;
			}
			/**
			 * @brief Makes current object a copy of the other.
			 * @param[in] other Object to make copy from.
			 * @param[in] allocateData If true new memory for data is allocated otherwise old memory is used.
			 * @return true if copy operation ended successfully, false otherwise.
			 */
			inline bool copyDataFrom(const MemoryProxy& other, bool allocateData = true)
			{
				if (!other.m_DataSet)
					return false;

				if (allocateData)
				{
					m_Ownership = true;
					m_Data = m_Allocator.allocate(other.m_Length);
					// This check does not performed in original code
					/* Put / before /* to uncomment (//*)
					if (!m_Data) {
						// Expect nullptr/NULL returned when execption thrown on allocation
						clear();
						return false;
					}
					//*/
					m_Length = other.m_Length;
				}
				std::memcpy(m_Data, other.m_Data, other.m_Length * sizeof(typename Base::value_type));
				m_DataSet = true;
				return true;
			}
		public:
			/**
			 * @brief Default constructor.
			 * Sets object to a null-state by internally calling initialize method.
			 */
			MemoryProxy() { initialize(); }
			/**
			 * @brief General object constructor.
			 * This function is a copy of old RawPacket::RawPacket general object constructor.
			 * Original description:\n
			 * A constructor that receives a pointer to the raw data (allocated elsewhere). This constructor is usually used when packet
			 * is captured using a packet capturing engine (like libPcap. WinPcap, PF_RING, etc.). The capturing engine allocates the raw data
			 * memory and give the user a pointer to it + a timestamp it has arrived to the device
			 * @param[in] p Pointer to memoty to take handle of.
			 * @param[in] length Size of provided memory (NOT in bytes).
			 * @param[in] ownership Indicator of ownership over provided memory.
			 */
			MemoryProxy(const_pointer p, size length, bool ownership) :
				m_Allocator() 
			{
				initialize();
				m_Ownership = ownership;
				setData(p, length);
			}
			/**
			 * @brief Copy constructor.
			 * This function is a copy of old RawPacket::RawPacket class copy constructor.
			 * Original description:\n
			 * A copy constructor that copies all data from another instance. Notice all raw data is copied (using memcpy), so when the original or
			 * the other instance are freed, the other won't be affected
			 * @param[in] other The instance to make copy of.
			 */
			MemoryProxy(const MemoryProxy& other) { copyDataFrom(other); }
			/**
			 * @brief Copy constructor.
			 * This function is a copy of old RawPacket::RawPacket class copy assignment operator.
			 * Original description:\n
			 * Assignment operator overload for this class. When using this operator on an already initialized RawPacket instance,
			 * the original raw data is freed first. Then the other instance is copied to this instance, the same way the copy constructor works
			 * @todo free raw data only if deleteRawDataAtDestructor was set to 'true'
			 * @param[in] other The instance to make copy of.
			 */
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
			/**
			 * @brief Destructor.
			 * This function is a copy of old RawPacket::~RawPacket destructor.
			 * Original description:\n
			 * A destructor for this class. Frees the raw data if deleteRawDataAtDestructor was set to 'true'
			 */
			~MemoryProxy() 
			{ 
				if (m_Ownership)
					m_Allocator.deallocate(m_Data);
			}
			/**
			 * @brief Returns known underlying data length.
			 * @return Known underlying data length.
			 */
			inline size getLength() const override { return m_Length; }
			/**
			 * @brief Returns owning status of underlying data.
			 * @return true if object owns it's underlying data, false otherwise.
			 */
			inline bool isOwning() const override { return m_Ownership; }
			/**
			 * @brief Returns pointer to the begining of underlying data.
			 * @return Pointer to the begining of underlying data.
			 */
			inline pointer get() override { return m_Data; }
			/**
			 * @brief Returns pointer to the begining of const qualified underlying data.
			 * This overload is called in object is const qualified.
			 * @return Pointer to the begining of const qualified underlying data.
			 */
			inline const_pointer get() const override { return m_Data; }
			/**
			 * @brief The ownership release method.
			 * This method was not presented in original RawPacket class code.
			 * @todo Add meaning to this method.
			 * @return Always returns nullptr.
			 */
			pointer relese() override
			{	// This function logic does not presented in original code
				/*typename Base::pointer old = m_Data;
				initialize();
				return old;*/
				return nullptr;
			}
			/**
			 * @brief Underlying data reset method.
			 * This method was not presented in original RawPacket class code.
			 * @param[in] ptr Pointer to memoty to take handle of.
			 * @param[in] length Size of provided memory (NOT in bytes).
			 * @param[in] ownership Indicator of ownership over provided memory.
			 * @todo Add meaning to this method.
			 * @return Always returns false.
			 */
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
			/**
			* @brief Method to check if current object is in the null-state.
			 * This method was not presented in original RawPacket class code.
			 * @todo Add meaning to this method.
			 * @return Always returns false.
			 */
			operator bool() const 
			{	// This function logic does not presented in original code
				// return m_Data || m_Ownership || m_Length > 0;
				return false;
			}
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
			 * This function is a copy of old RawPacket::reallocateData method.
			 * Original description:\n
			 * Re-allocate raw packet buffer meaning add size to it without losing the current packet data. This method allocates the required buffer size as instructed
			 * by the use and then copies the raw data from the current allocated buffer to the new one. This method can become useful if the user wants to insert or
			 * append data to the raw data, and the previous allocated buffer is too small, so the user wants to allocate a larger buffer and get RawPacket instance to
			 * point to it.
			 * @param[in] newBufferLength New size of data.
			 * @param[in] initialValue Perbyte initial value of new memory on allocation.
			 * @return true if operation ended successfully, false otherwise (you may expect that object is in null-state).
			 */
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
			/**
			 * @brief Clear underlying data and set object to a null-state.
			 * This function is a copy of old RawPacket::clear method.
			 * Original description:\n
			 * Clears all members of this instance, meaning setting raw data to NULL, raw data length to 0, etc. Currently raw data is always freed,
			 * even if deleteRawDataAtDestructor was set to 'false'.
			 * @todo deleteRawDataAtDestructor was set to 'true', don't free the raw data.
			 * @todo set timestamp to a default value as well.
			 * @return true if operation ended successfully, false otherwise.
			 */
			bool clear() override
			{
				if (m_Data != nullptr)
					m_Allocator.deallocate(m_Data);
				m_Data = nullptr;
				m_Length = 0;
				m_DataSet = false;
				return true;
			}
			/**
			 * @brief Append memory capable of holding dataToAppendLen data entries and set it per-byte to initialValue on allocation.
			 * This function is analogous to old RawPacket::appendData method but does not require to provide a temporary memory buffer.
			 * Original description:\n
			 * Append data to the end of current data. This method works without allocating more memory, it just uses memcpy() to copy dataToAppend at
			 * the end of the current data. This means that the method assumes this memory was already allocated by the user. If it isn't the case then
			 * this method will cause memory corruption.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation ended successfully, false otherwise.
			 */
			bool append(size dataToAppendLen, memory_value initialValue = 0) override
			{
				std::memset(m_Data + m_Length, initialValue, dataToAppendLen * sizeof(typename Base::value_type));
				m_Length += dataToAppendLen;
				return true;
			}
			/**
			 * @brief Append memory capable of holding dataToAppendLen data entries and copy data from dataToAppend to it (concatenate).
			 * This function is a copy of old RawPacket::appendData method.
			 * Original description:\n
			 * Append data to the end of current data. This method works without allocating more memory, it just uses memcpy() to copy dataToAppend at
			 * the end of the current data. This means that the method assumes this memory was already allocated by the user. If it isn't the case then
			 * this method will cause memory corruption.
			 * @param[in] dataToAppend Buffer memory to be appended to current data.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @return true if operation ended successfully, false otherwise.
			 */
			bool append(const_pointer dataToAppend, size dataToAppendLen) override
			{
				std::memcpy(m_Data + m_Length, dataToAppend, dataToAppendLen * sizeof(typename Base::value_type));
				m_Length += dataToAppendLen;
				return true;
			}
			/**
			 * @brief Inserts memory capable of holding dataToInsertLen data entries and set it per-byte to initialValue on allocation.
			 * This function is analogous to old RawPacket::insertData method but does not require to provide a temporary memory buffer.
			 * Original description:\n
			 * Insert new data at some index of the current data and shift the remaining old data to the end. This method works without allocating more memory,
			 * it just copies dataToAppend at the relevant index and shifts the remaining data to the end. This means that the method assumes this memory was
			 * already allocated by the user. If it isn't the case then this method will cause memory corruption.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
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
			/**
			 * @brief Inserts memory capable of holding dataToInsertLen data entries and copy data from dataToAppend to it.
			 * This function is a copy of old RawPacket::insertData method.
			 * Original description:\n
			 * Insert new data at some index of the current data and shift the remaining old data to the end. This method works without allocating more memory,
			 * it just copies dataToAppend at the relevant index and shifts the remaining data to the end. This means that the method assumes this memory was
			 * already allocated by the user. If it isn't the case then this method will cause memory corruption.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsert Buffer memory to be inserted to current data.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @return true if operation finished successfully, false otherwise.
			 */
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
			/**
			 * @brief Removes memory capable of holding numOfBytesToRemove data entries starting from atIndex.
			 * This function is a copy of old RawPacket::removeData method.
			 * Original description:\n
			 * Remove certain number of bytes from current raw data buffer. All data after the removed bytes will be shifted back.
			 * @param[in] atIndex Index from which removal take place.
			 * @param[in] numOfBytesToRemove Size of data to be removed.
			 * @return true if operation finished successfully, false otherwise.
			 */
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