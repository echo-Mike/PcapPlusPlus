#define LOG_MODULE PacketLogModuleRawPacket

#include "RawPacket.h"
#include "Logger.h"

namespace pcpp
{

RawPacket& RawPacket::operator=(const RawPacket& other)
{
	// Will not copy if assigned to itself
	if (this == &other) 
		return *this;
	m_TimeStamp = other.m_TimeStamp;
	m_FrameLength = other.m_FrameLength;
	m_linkLayerType = other.m_linkLayerType;
	return *this;
}

bool RawPacket::setRawData(const_pointer /* pRawData */, size rawDataLen, timeval timestamp, LinkLayerType layerType, length frameLength)
{
	if(frameLength == -1)
		frameLength = rawDataLen;
	m_FrameLength = frameLength;
	m_TimeStamp = timestamp;
	m_linkLayerType = layerType;
	return true;
}

} // namespace pcpp
