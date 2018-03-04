#define LOG_MODULE PacketLogModulePacket

#include "Packet.h"
#include "EthLayer.h"
#include "SllLayer.h"
#include "NullLoopbackLayer.h"
#include "IPv4Layer.h"
#include "IPv6Layer.h"
#include "PayloadLayer.h"
#include "Logger.h"
#include <type_traits>
#include <typeinfo>
#include <cstring>
#include <sstream>
#include <utility>
#ifdef _MSC_VER
#include <time.h>
#include "SystemUtils.h"
#endif

namespace pcpp
{

void Packet::destructPacketData()
{
	// Clear layer storage
	Layer *curLayer = m_FirstLayer, *nextLayer = PCAPPP_NULLPTR;
	while (curLayer)
	{
		nextLayer = curLayer->getNextLayer();
		if (curLayer->m_IsAllocatedInPacket)
			delete curLayer;
		curLayer = nextLayer;
	}
	// Deallocate RawPacket
	if (m_RawPacket != PCAPPP_NULLPTR && m_FreeRawPacket)
		delete m_RawPacket;
}

Packet::Packet(size_t maxPacketLen)
{
	// Create this object in null-state
	initialize();
	// Allocate new RawPacket
	m_RawPacket = new DefaultRawPacket(maxPacketLen);
	m_FreeRawPacket = true;
	// Set time-stamp of packet
	timeval time;
	gettimeofday(&time, NULL);
	m_RawPacket->resetPacketTimeStamp(time);
}

Packet::Packet(RawPacket* rawPacket, bool freeRawPacket, ProtocolType parseUntil, OsiModelLayer parseUntilLayer)
{
	// Create this object in null-state
	initialize();
	setRawPacket(rawPacket, freeRawPacket, parseUntil, parseUntilLayer);
}

Packet::Packet(RawPacket* rawPacket, ProtocolType parseUntil)
{
	// Create this object in null-state
	initialize();
	setRawPacket(rawPacket, false, parseUntil, OsiModelLayerUnknown);
}

Packet::Packet(RawPacket* rawPacket, OsiModelLayer parseUntilLayer)
{
	// Create this object in null-state
	initialize();
	setRawPacket(rawPacket, false, UnknownProtocol, parseUntilLayer);
}

Packet::Packet(const Packet& other)
{
	// Create this object in null-state
	initialize();
	copyDataFrom(other);
}

Packet& Packet::operator=(const Packet& other)
{
	// Will not copy if assigned to itself
	if (this == &other) 
		return *this;
	copyDataFrom(other);
	return *this;
}

void Packet::copyDataFrom(const Packet& other)
{
	// This is an invalid case and it is very hard to handle
	if (other.m_RawPacket == PCAPPP_NULLPTR)
	{
		LOG_DEBUG("Attempt to make copy of packet without underlying RawPacket.");
		return;
	}
	// Destruct current packet data
	destructPacketData();
	// Create new copy of other raw packet object
	m_RawPacket = other.m_RawPacket->copy();
	// We definitely own an underlying RawPacket now
	m_FreeRawPacket = true;
	m_ProtocolTypes = other.m_ProtocolTypes;
	// @todo:
	// An error here may occur if first layer is not Ethernet
	// An error here may occur if m_RawPacket->getRawData() returns nullptr a.e. RawPacket is even but data is corrupted
	m_FirstLayer = new EthLayer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), this);
	m_LastLayer = m_FirstLayer;
	// Recreate layer structure
	// @todo:
	// Re-parsing is a bad decision, consider to use dynamic_cast based approach, benchmarks needed.
	Layer* curLayer = m_FirstLayer;
	while (curLayer)
	{
		curLayer->parseNextLayer();
		curLayer->m_IsAllocatedInPacket = true;
		curLayer = curLayer->getNextLayer();
		if (curLayer)
			m_LastLayer = curLayer;
	}
}

PCAPPP_MOVE_CONSTRUCTOR_IMPL(Packet)
{	// Create this object in null-state
	initialize();
	moveDataFrom(PCAPPP_MOVE_OTHER);
}

PCAPPP_MOVE_ASSIGNMENT_IMPL(Packet)
{
	// Will not move if assigned to itself
	if (this == &other) 
		return *this;
	moveDataFrom(PCAPPP_MOVE_OTHER);
	return *this;
}

void Packet::moveDataFrom(Packet& other)
{
	// Destruct current packet data
	destructPacketData();
	// Move data members
	m_RawPacket = other.m_RawPacket;
	m_LastLayer = other.m_LastLayer;
	m_FirstLayer = other.m_FirstLayer;
	// Reset pointers of packet layers to point to this object
	Layer* curLayer = m_FirstLayer;
	while (curLayer)
	{
		curLayer->m_Packet = this;
		curLayer = curLayer->getNextLayer();
	}
	// Copy non-movable data members
	m_FreeRawPacket = other.m_FreeRawPacket;
	m_ProtocolTypes = other.m_ProtocolTypes;
	// Return other to a null-state
	other.initialize();
}

Packet::~Packet()
{
	destructPacketData();
}

void Packet::parseLayers(ProtocolType parseUntil, OsiModelLayer parseUntilLayer)
{
	LinkLayerType linkType = m_RawPacket->getLinkLayerType();

	if (linkType == LINKTYPE_ETHERNET)
	{
		m_FirstLayer = new EthLayer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), this);
	}
	else if (linkType == LINKTYPE_LINUX_SLL)
	{
		m_FirstLayer = new SllLayer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), this);
	}
	else if (linkType == LINKTYPE_NULL)
	{
		m_FirstLayer = new NullLoopbackLayer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), this);
	}
	else if (linkType == LINKTYPE_RAW || linkType == LINKTYPE_DLT_RAW1 || linkType == LINKTYPE_DLT_RAW2)
	{
		uint8_t ipVer = m_RawPacket->getRawData()[0] & 0xf0;
		if (ipVer == 0x40)
			m_FirstLayer = new IPv4Layer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), NULL, this);
		else if (ipVer == 0x60)
			m_FirstLayer = new IPv6Layer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), NULL, this);
		else
			m_FirstLayer = new PayloadLayer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), NULL, this);
	}
	else // unknown link type
	{
		m_FirstLayer = new EthLayer(m_RawPacket->getRawData(), m_RawPacket->getRawDataLen(), this);
	}

	m_LastLayer = m_FirstLayer;
	Layer* curLayer = m_FirstLayer;
	while (curLayer != NULL && (curLayer->getProtocol() & parseUntil) == 0 && curLayer->getOsiModelLayer() <= parseUntilLayer)
	{
		m_ProtocolTypes |= curLayer->getProtocol();
		curLayer->parseNextLayer();
		curLayer->m_IsAllocatedInPacket = true;
		curLayer = curLayer->getNextLayer();
		if (curLayer != NULL)
			m_LastLayer = curLayer;
	}

	if (curLayer != NULL && (curLayer->getProtocol() & parseUntil) != 0)
	{
		m_ProtocolTypes |= curLayer->getProtocol();
		curLayer->m_IsAllocatedInPacket = true;
	}

	if (curLayer != NULL &&  curLayer->getOsiModelLayer() > parseUntilLayer)
	{
		m_LastLayer = curLayer->getPrevLayer();
		delete curLayer;
		m_LastLayer->m_NextLayer = NULL;
	}
}

void Packet::setRawPacket(RawPacket* rawPacket, bool freeRawPacket, ProtocolType parseUntil, OsiModelLayer parseUntilLayer)
{
	destructPacketData();
	initialize();
	m_FreeRawPacket = freeRawPacket;
	m_RawPacket = rawPacket;
	if (m_RawPacket == PCAPPP_NULLPTR)
		return;
	parseLayers(parseUntil, parseUntilLayer);
}

bool Packet::holdCopy(RawPacket* rawPacket)
{
	if (rawPacket == PCAPPP_NULLPTR)
	{
		// TODO: Add error message
		return false;
	}
	// Create copy
	RawPacket* temp = rawPacket->copy();
	if (temp == PCAPPP_NULLPTR)
	{
		// TODO: Add error message
		return false;
	}

	if (m_RawPacket != PCAPPP_NULLPTR && rawPacket == m_RawPacket) {
		// Reset pointers of packet layers to newly allocated object memory
		Layer* curLayer = m_FirstLayer;
		RawPacket::pointer oldBegin = m_RawPacket->getRawData();
		RawPacket::pointer newBegin = temp->getRawData();
		while (curLayer)
		{
			curLayer->m_Data = newBegin + (curLayer->m_Data - oldBegin);
			curLayer = curLayer->getNextLayer();
		}

		// Deallocate RawPacket
		if ( m_FreeRawPacket )
			delete m_RawPacket;

		m_RawPacket = temp;
	} else {
		destructPacketData();
		initialize();
		m_RawPacket = temp;
		parseLayers();
	}
	// We definitely own an underlying raw packet now
	m_FreeRawPacket = true;
	return true;
}

bool Packet::holdProvided(RawPacket* rawPacket)
{
	if (rawPacket == PCAPPP_NULLPTR)
	{
		// TODO: Add error message
		return false;
	}

	bool is_same = (rawPacket == m_RawPacket);

	// Create copy by move
	RawPacket* temp = rawPacket->move();
	if (temp == PCAPPP_NULLPTR)
	{
		// TODO: Add error message
		return false;
	}

	if (is_same) { 
		// data was moved not copied --> no pointer correction needed.
		// Deallocate RawPacket
		if (m_FreeRawPacket)
			delete m_RawPacket;

		m_RawPacket = temp;
	} else {
		// Re-parsing layers
		destructPacketData();
		initialize();
		m_RawPacket = temp;
		parseLayers();
	}

	// We definitely own an underlying raw packet now
	m_FreeRawPacket = true;
	return true;
}

bool Packet::addLayer(Layer* newLayer)
{
	return insertLayer(m_LastLayer, newLayer);
}

bool Packet::insertLayer(Layer* prevLayer, Layer* newLayer)
{
	if (!newLayer)
	{
		LOG_ERROR("Layer to add is NULL");
		return false;
	}

	if (newLayer->isAllocatedToPacket())
	{
		LOG_ERROR("Layer is already allocated to another packet. Cannot use layer in more than one packet");
		return false;
	}

	size_t appendDataLen = newLayer->getHeaderLen();

	// insert layer data to raw packet
	int indexToInsertData = 0;
	if (prevLayer != NULL)
		indexToInsertData = prevLayer->m_Data + prevLayer->getHeaderLen() - m_RawPacket->getRawData();
	m_RawPacket->insertData(indexToInsertData, newLayer->m_Data, appendDataLen);

	// delete previous layer data
	delete[] newLayer->m_Data; //<- @todo This is really bad, Layer must be a proper proxy class for data on it's own

	// add layer to layers linked list
	if (prevLayer != NULL)
	{
		newLayer->setNextLayer(prevLayer->getNextLayer());
		newLayer->setPrevLayer(prevLayer);
		if (prevLayer->getNextLayer())
			prevLayer->getNextLayer()->setPrevLayer(newLayer);
		prevLayer->setNextLayer(newLayer);
	}
	else // prevLayer == NULL
	{	// Add layer to head of packet
		newLayer->setNextLayer(m_FirstLayer);
		if (m_FirstLayer != NULL)
			m_FirstLayer->setPrevLayer(newLayer);
		m_FirstLayer = newLayer;
	}

	if (newLayer->getNextLayer() == NULL)
		m_LastLayer = newLayer;

	// assign layer with this packet only
	newLayer->m_Packet = this;
	
	// re-calculate all layers data ptr and data length
	uint8_t* dataPtr = m_RawPacket->getRawData();
	int dataLen = m_RawPacket->getRawDataLen();

	Layer* curLayer = m_FirstLayer;
	while (curLayer != NULL)
	{
		curLayer->m_Data = dataPtr;
		curLayer->m_DataLen = dataLen;
		dataPtr += curLayer->getHeaderLen();
		dataLen -= curLayer->getHeaderLen();
		curLayer = curLayer->getNextLayer();
	}

	// add layer protocol to protocol collection
	m_ProtocolTypes |= newLayer->getProtocol();
	return true;
}

bool Packet::removeLayer(Layer* layer)
{
	if (!layer)
	{
		LOG_ERROR("Layer is NULL");
		return false;
	}

	// verify layer is allocated to a packet
	if (!layer->isAllocatedToPacket())
	{
		LOG_ERROR("Layer isn't allocated to any packet");
		return false;
	}

	// verify layer is allocated to *this* packet
	Layer* curLayer = layer;
	while (curLayer->m_PrevLayer != NULL)
		curLayer = curLayer->m_PrevLayer;
	if (curLayer != m_FirstLayer)
	{
		LOG_ERROR("Layer isn't allocated to this packet");
		return false;
	}

	// remove data from raw packet
	size_t numOfBytesToRemove = layer->getHeaderLen();
	int indexOfDataToRemove = layer->m_Data - m_RawPacket->getRawData();
	if (!m_RawPacket->removeData(indexOfDataToRemove, numOfBytesToRemove))
	{
		LOG_ERROR("Couldn't remove data from packet");
		return false;
	}

	// remove layer from layers linked list
	if (layer->m_PrevLayer != NULL)
		layer->m_PrevLayer->setNextLayer(layer->m_NextLayer);
	if (layer->m_NextLayer != NULL)
		layer->m_NextLayer->setPrevLayer(layer->m_PrevLayer);

	// take care of head and tail ptrs
	if (m_FirstLayer == layer)
		m_FirstLayer = layer->m_NextLayer;
	if (m_LastLayer == layer)
		m_LastLayer = layer->m_PrevLayer;
	layer->setNextLayer(NULL);
	layer->setPrevLayer(NULL);

	// re-calculate all layers data ptr and data length
	uint8_t* dataPtr = m_RawPacket->getRawData();
	int dataLen = m_RawPacket->getRawDataLen();

	curLayer = m_FirstLayer;
	bool anotherLayerWithSameProtocolExists = false;
	while (curLayer != NULL)
	{
		curLayer->m_Data = dataPtr;
		curLayer->m_DataLen = dataLen;
		if (curLayer->getProtocol() == layer->getProtocol())
			anotherLayerWithSameProtocolExists = true;
		dataPtr += curLayer->getHeaderLen();
		dataLen -= curLayer->getHeaderLen();
		curLayer = curLayer->getNextLayer();
	}

	// remove layer protocol from protocol list if necessary
	if (!anotherLayerWithSameProtocolExists)
		m_ProtocolTypes &= ~((uint64_t)layer->getProtocol());

	// if layer was allocated by this packet, delete it
	if (layer->m_IsAllocatedInPacket)
		delete layer;

	return true;
}

bool Packet::extendLayer(Layer* layer, int offsetInLayer, size_t numOfBytesToExtend)
{
	if (!layer)
	{
		LOG_ERROR("Layer is NULL");
		return false;
	}

	// verify layer is allocated to this packet
	if (!(layer->m_Packet == this))
	{
		LOG_ERROR("Layer isn't allocated to this packet");
		return false;
	}

	// insert layer data to raw packet
	int indexToInsertData = layer->m_Data + offsetInLayer - m_RawPacket->getRawData();
	m_RawPacket->insertData(indexToInsertData, numOfBytesToExtend);

	// re-calculate all layers data ptr and data length
	uint8_t* dataPtr = m_RawPacket->getRawData();
	int dataLen = m_RawPacket->getRawDataLen();

	Layer* curLayer = m_FirstLayer;
	while (curLayer != NULL)
	{
		curLayer->m_Data = dataPtr;
		curLayer->m_DataLen = dataLen;
		// assuming header length of the layer that requested to be extended hasn't been enlarged yet
		size_t headerLen = curLayer->getHeaderLen() + (curLayer == layer ? numOfBytesToExtend : 0);
		dataPtr += headerLen;
		dataLen -= headerLen;
		curLayer = curLayer->getNextLayer();
	}

	return true;
}

bool Packet::shortenLayer(Layer* layer, int offsetInLayer, size_t numOfBytesToShorten)
{
	if (!layer)
	{
		LOG_ERROR("Layer is NULL");
		return false;
	}

	// verify layer is allocated to this packet
	if (!(layer->m_Packet == this))
	{
		LOG_ERROR("Layer isn't allocated to this packet");
		return false;
	}

	// remove data from raw packet
	int indexOfDataToRemove = layer->m_Data + offsetInLayer - m_RawPacket->getRawData();
	if (!m_RawPacket->removeData(indexOfDataToRemove, numOfBytesToShorten))
	{
		LOG_ERROR("Couldn't remove data from packet");
		return false;
	}

	// re-calculate all layers data ptr and data length
	uint8_t* dataPtr = m_RawPacket->getRawData();
	int dataLen = m_RawPacket->getRawDataLen();

	Layer* curLayer = m_FirstLayer;
	while (curLayer != NULL)
	{
		curLayer->m_Data = dataPtr;
		curLayer->m_DataLen = dataLen;
		// assuming header length of the layer that requested to be extended hasn't been enlarged yet
		size_t headerLen = curLayer->getHeaderLen() - (curLayer == layer ? numOfBytesToShorten : 0);
		dataPtr += headerLen;
		dataLen -= headerLen;
		curLayer = curLayer->getNextLayer();
	}

	return true;
}

void Packet::computeCalculateFields()
{
	// calculated fields should be calculated from top layer to bottom layer

	Layer* curLayer = m_LastLayer;
	while (curLayer != NULL)
	{
		curLayer->computeCalculateFields();
		curLayer = curLayer->getPrevLayer();
	}
}

std::string Packet::printPacketInfo(bool timeAsLocalTime)
{
	std::ostringstream dataLenStream;
	dataLenStream << m_RawPacket->getRawDataLen();

	// convert raw packet timestamp to printable format
	timeval timestamp = m_RawPacket->getPacketTimeStamp();
	time_t nowtime = timestamp.tv_sec;
	struct tm *nowtm = NULL;
	if (timeAsLocalTime)
		nowtm = localtime(&nowtime);
	else
		nowtm = gmtime(&nowtime);

	char tmbuf[64], buf[64];
	if (nowtm != NULL)
	{
		strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
		snprintf(buf, sizeof(buf), "%s.%06lu", tmbuf, timestamp.tv_usec);
	}
	else
		snprintf(buf, sizeof(buf), "0000-00-00 00:00:00.000000");
	
	return "Packet length: " + dataLenStream.str() + " [Bytes], Arrival time: " + std::string(buf);
}

std::string Packet::printToString(bool timeAsLocalTime)
{
	std::vector<std::string> stringList;
	std::string result;
	printToStringList(stringList, timeAsLocalTime);
	for (std::vector<std::string>::iterator iter = stringList.begin(); iter != stringList.end(); ++iter)
	{
		result += *iter + "\n";
	}

	return result;
}

void Packet::printToStringList(std::vector<std::string>& result, bool timeAsLocalTime)
{
	result.clear();
	result.push_back(printPacketInfo(timeAsLocalTime));
	Layer* curLayer = m_FirstLayer;
	while (curLayer != NULL)
	{
		result.push_back(curLayer->toString());
		curLayer = curLayer->getNextLayer();
	}
}

} // namespace pcpp
