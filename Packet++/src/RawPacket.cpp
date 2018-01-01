#define LOG_MODULE PacketLogModuleRawPacket

#include "RawPacket.h"
// For std::memcopy, std::memmove and std::memset
#include <cstring>
// For std::move
#include <utility>
#include "Logger.h"

namespace pcpp
{

void RawPacket::initialize()
{
	// Null object state
	m_RawDataLen = 0;
	m_FrameLength = 0;
	m_pRawData = nullptr;
	m_RawPacketSet = false;
	m_DeleteRawDataAtDestructor = false;
	m_linkLayerType = LINKTYPE_ETHERNET;
	// RawPacket object size on 64-bits systems is less than 64 bytes
	// so the whole object may be loaded in one cache line
	// and therefore the order of calls to assignment is not essential.
	// timeval is a system dependent structure and can't be clearly set to some null-state
}

RawPacket::RawPacket()
{
	// Produce an object at null-state
	initialize();
}

RawPacket::RawPacket(const uint8_t* pRawData, int rawDataLen, timeval timestamp, bool deleteRawDataAtDestructor, LinkLayerType layerType)
{
	initialize();
	// If data provided by pRawData and rawDataLen is corrupted but 
	// deleteRawDataAtDestructor is true it is UB
	if (setRawData(pRawData, rawDataLen, timestamp, layerType))
		m_DeleteRawDataAtDestructor = deleteRawDataAtDestructor;
}

RawPacket::RawPacket(const RawPacket& other)
{
	initialize();
	copyDataFrom(other);
}

RawPacket& RawPacket::operator=(const RawPacket& other)
{
	// Will not copy if assigned to itself
	if (this == &other) 
		return *this;
	copyDataFrom(other, true);
	return *this;
}

void RawPacket::copyDataFrom(const RawPacket& other, bool allocateData)
{
	// Take a look on SafeToCopyDataCondition function member description
	if (!other.SafeToCopyDataCondition())
		return;

	if (allocateData)
	{
		// Deallocate all data that object currently own
		if (SafeToDeleteDataCondition())
			delete[] m_pRawData;
		// Still all conditions in SafeToDeleteDataCondition() cannot guarantee that
		// memory leak here will not occur.
		m_DeleteRawDataAtDestructor = false;
		// There will be an error if other.m_RawDataLen is corrupted or set less or equal to 0 
		m_pRawData = new uint8_t[other.m_RawDataLen];
		// If exception thrown here object state is not invalid, data will not be touched on object destruction. 
		// Now we definitely own an underlying data
		m_DeleteRawDataAtDestructor = true;
		m_RawDataLen = other.m_RawDataLen;
	}
	// Copy memory from other.
	// This function may be called from derived classes
	// and it is undefined behavior to use std::memcpy on same memory.
	if (this != &other && static_cast<bool>(m_pRawData))
		std::memcpy(m_pRawData, other.m_pRawData, other.m_RawDataLen);
	// Copy-assing other data members
	m_linkLayerType = other.m_linkLayerType;
	m_FrameLength = other.m_FrameLength;
	m_TimeStamp = other.m_TimeStamp;
	m_RawPacketSet = true;
}

RawPacket::RawPacket(RawPacket&& other)
{
	initialize();
	moveDataFrom(std::move(other));
}

RawPacket& RawPacket::operator=(RawPacket&& other)
{
	// Will not move if assigned to itself
	if (this == &other) 
		return *this;
	moveDataFrom(std::move(other));
	return *this;
}

void RawPacket::moveDataFrom(RawPacket&& other)
{
	// Deallocate all data that object currently own
	if (SafeToDeleteDataCondition())
		delete[] m_pRawData;
	// Still all conditions in SafeToDeleteDataCondition() cannot guarantee that
	// memory leak here will not occur.
	// Raw data pointer is transfered to this object
	m_pRawData = other.m_pRawData;
	// All other internal data doesn't support move semantics
	m_TimeStamp = other.m_TimeStamp;
	m_RawDataLen = other.m_RawDataLen;
	m_FrameLength = other.m_FrameLength;
	m_RawPacketSet = other.m_RawPacketSet;
	m_linkLayerType = other.m_linkLayerType;
	m_DeleteRawDataAtDestructor = other.m_DeleteRawDataAtDestructor;
	// Clear other object to the null-object state
	other.initialize();
}

RawPacket::~RawPacket()
{
	if (SafeToDeleteDataCondition())
		delete[] m_pRawData;
	// Still all conditions in SafeToDeleteDataCondition() cannot guarantee that
	// memory leak here will not occur.
}

bool RawPacket::setRawData(const uint8_t* pRawData, int rawDataLen, timeval timestamp, LinkLayerType layerType, int frameLength)
{
	// This case is unsafe to handle
	if (!static_cast<bool>(pRawData) || rawDataLen < 1)
		return false;

	if(frameLength == -1)
		frameLength = rawDataLen;

	if (SafeToDeleteDataCondition())
		delete[] m_pRawData;
	// Still all conditions in SafeToDeleteDataCondition() cannot guarantee that
	// memory leak here will not occur.
	m_pRawData = const_cast<uint8_t*>(pRawData);
	m_RawDataLen = rawDataLen;
	m_FrameLength = frameLength;
	m_TimeStamp = timestamp;
	m_linkLayerType = layerType;
	m_RawPacketSet = true;
	return true;
}

/*
const uint8_t* RawPacket::getRawData()
{
	return m_pRawData;
}

const uint8_t* RawPacket::getRawDataReadOnly() const
{
	return m_pRawData;
}
		
LinkLayerType RawPacket::getLinkLayerType() const
{
	return m_linkLayerType;
}

int RawPacket::getRawDataLen() const
{
	return m_RawDataLen;
}

int RawPacket::getFrameLength() const
{
	return m_FrameLength;
}

timeval RawPacket::getPacketTimeStamp() const
{
	return m_TimeStamp;
}
*/

void RawPacket::clear()
{
	if (SafeToDeleteDataCondition())
		delete[] m_pRawData;
	// Still all conditions in SafeToDeleteDataCondition() cannot guarantee that
	// memory leak here will not occur.
	initialize();
}

void RawPacket::appendData(const uint8_t* dataToAppend, size_t dataToAppendLen)
{
	// std::memmove is used because there is no guarantee that dataToAppend	
	// is not pointing to some data in current packet
	std::memmove(m_pRawData+m_RawDataLen, dataToAppend, dataToAppendLen);
	m_RawDataLen += dataToAppendLen;
	m_FrameLength = m_RawDataLen;
}

void RawPacket::insertData(int atIndex, size_t dataToInsertLen)
{
	// atIndex must be some positive value or zero and dataToInsertLen must not be zero 
	if (atIndex < 0 || !dataToInsertLen)
		return;
	// Move (m_RawDataLen - atIndex) bytes from atIndex offset to atIndex + dataToInsertLen offset
	// Memory from [atIndex : atIndex + dataToInsertLen - 1] is now free to be used
	std::memmove(m_pRawData + atIndex + dataToInsertLen, m_pRawData + atIndex, m_RawDataLen - atIndex);
	// Set data in [atIndex : atIndex + dataToInsertLen - 1] space to 0
	std::memset(m_pRawData + atIndex, 0, dataToInsertLen);
	// Increment current data length counter by dataToInsertLen amount
	m_RawDataLen += dataToInsertLen;
	m_FrameLength = m_RawDataLen;
}

void RawPacket::insertData(int atIndex, const uint8_t* dataToInsert, size_t dataToInsertLen)
{
	// atIndex must be some positive value or zero and dataToInsertLen must not be zero 
	if (atIndex < 0 || !dataToInsertLen)
		return;
	// Move (m_RawDataLen - atIndex) bytes from atIndex offset to atIndex + dataToInsertLen offset
	// Memory from [atIndex : atIndex + dataToInsertLen - 1] is now free to be used
	std::memmove(m_pRawData + atIndex + dataToInsertLen, m_pRawData + atIndex, m_RawDataLen - atIndex);
	// Copy data to [atIndex : atIndex + dataToInsertLen - 1] space
	// UB here if dataToInsert points to some data in m_pRawData
	std::memcpy(m_pRawData + atIndex, dataToInsert, dataToInsertLen);
	// Increment current data length counter by dataToInsertLen amount
	m_RawDataLen += dataToInsertLen;
	m_FrameLength = m_RawDataLen;
}

bool RawPacket::reallocateData(size_t newBufferLength)
{
	if ((int)newBufferLength == m_RawDataLen)
		return true;

	if ((int)newBufferLength < m_RawDataLen)
	{
		LOG_ERROR("Cannot reallocate raw packet to a smaller size. Current data length: %d; requested length: %d", m_RawDataLen, (int)newBufferLength);
		return false;
	}

	// Allocate and clear a new buffer
	uint8_t* newBuffer = new uint8_t[newBufferLength];
	std::memset(newBuffer, 0, newBufferLength);
	// There may not be safe to copy data of current object to new buffer
	if (SafeToCopyDataCondition())
		std::memcpy(newBuffer, m_pRawData, m_RawDataLen);
	if (SafeToDeleteDataCondition())
		delete [] m_pRawData;
	m_pRawData = newBuffer;
	// We definitely own an underlying data now
	m_DeleteRawDataAtDestructor = true;
	m_RawPacketSet = true;
	return true;
}

bool RawPacket::removeData(int atIndex, size_t numOfBytesToRemove)
{
	if (atIndex < 0) 
	{
		LOG_ERROR("Data can't be removed from negative values of atIndex. atIndex value is %d.", atIndex);
		return false;
	}
	if (!numOfBytesToRemove)
	{
		LOG_DEBUG("Can't manipulate with 0 bytes.");
		// Manipulation with zero count of bytes is always a success
		return true;
	}
	if ((atIndex + (int)numOfBytesToRemove) > m_RawDataLen)
	{
		LOG_ERROR("Remove section is out of raw packet bound.");
		return false;
	}
	// Move data from (atIndex + numOfBytesToRemove) index to atIndex index
	// Count of tail bytes to be moved is equals current length minus index of this data: m_RawDataLen - data_index -> data_index = atIndex + numOfBytesToRemove
	std::memmove(m_pRawData + atIndex, m_pRawData + atIndex + numOfBytesToRemove, m_RawDataLen - atIndex - numOfBytesToRemove);
	// Set now unused tail data to zero: index of tail data -> (m_RawDataLen - numOfBytesToRemove) - pass-the-end pointer to current data
	std::memset(m_pRawData + (m_RawDataLen - numOfBytesToRemove), 0, numOfBytesToRemove);
	m_RawDataLen -= numOfBytesToRemove;
	m_FrameLength = m_RawDataLen;
	return true;
}

} // namespace pcpp
