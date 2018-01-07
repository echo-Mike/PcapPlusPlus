#ifdef USE_DPDK

#define LOG_MODULE PcapLogModuleDpdkDevice

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS

#include "DpdkDevice.h"
#include "DpdkDeviceList.h"
#include "Logger.h"
#include "rte_config.h"
#include "rte_ethdev.h"
#include "rte_errno.h"
#include <cstring>
#include <utility>
#include <stdint.h>
#include <unistd.h>

#define MBUF_DATA_SIZE 2048

#define MBUF_SIZE (MBUF_DATA_SIZE + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)

#define RX_BURST_SIZE 64

namespace pcpp
{

/**
 * ===================
 * Class MBufRawPacket
 * ===================
 */

bool MBufRawPacket::setMBuf(struct rte_mbuf* mBuf, timeval timestamp)
{
	if (mBuf == NULL)
	{
		LOG_ERROR("mbuf to set is NULL");
		return false;
	}
	
	// Free mBuf only if it is present and not same as provided (self assignment case)
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
	if (m_MBuf != NULL && m_MBuf != mBuf)
		rte_pktmbuf_free(m_MBuf);
	
	// Set current mBuf to be the provided one
	m_MBuf = mBuf;

	// Disable deallocation mechanism in next function call
	m_RawPacketSet = false;
	// Only set base class fields, don't do anything with data
	// rte_pktmbuf_mtod: 
	//		A macro that points to the start of the data in the mbuf
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a2a8b10263496c7b580e9d0c7f2a1f073
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a2a8b10263496c7b580e9d0c7f2a1f073
	// rte_pktmbuf_pkt_len: 
	//		A macro that returns the length of the carried data
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#ab3d5990c3ca14784ed3e0135844282a3
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a7a7ea614c79d5b9ed790a56c1c79189d
	if (!RawPacket::setRawData(rte_pktmbuf_mtod(mBuf, const uint8_t*), rte_pktmbuf_pkt_len(mBuf), timestamp, LINKTYPE_ETHERNET))
	{ 
		// This function may return false if rte_pktmbuf_pkt_len(mBuf) < 1
		// Here we are in some undetermined state -> UB after this line
		LOG_ERROR("Can't set mBuf. mBuf length is 0.");
		// Possible solutions (uncomment appropriate combination):
		// rte_pktmbuf_free(m_MBuf);
		/*
		m_MBuf = NULL;
		m_Device = NULL;
		*/
		// RawPacket::initialize(); // Call to clear will be unsafe
		return false;
	}
	return true;
}

bool MBufRawPacket::allocate(rte_mbuf* &mBuf, rte_mempool* pool)
{
	// Check for pool to not to be nullptr
	if (!pool)
	{
		LOG_ERROR("Invalid mBuf pool pointer. Can't allocate mBuf.");
		return false;
	}
	// Call dpdk mBuf allocation function for specified pool
	// rte_pktmbuf_alloc:
	//		Allocate a new mbuf from a mempool
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#ad4d1c289d8cffc831dfb77c64f52447b
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#ad4d1c289d8cffc831dfb77c64f52447b
	mBuf = rte_pktmbuf_alloc(pool);
	if (mBuf == NULL)
	{
		LOG_ERROR("Couldn't allocate mbuf");
		return false;
	}
	return true;
}

bool MBufRawPacket::adjust(rte_mbuf* &mBuf, std::size_t oldSize, std::size_t newSize)
{
	// We can't work with unknown object
	if (mBuf == NULL)
	{
		LOG_ERROR("Can't adjust NULL mbuf");
		return false;
	}

	if (newSize > MBUF_DATA_SIZE)
	{
		LOG_ERROR("Can't adjust mBuf. Provided size is gereater than maximum possible size. mBuf max length: %d; requested length: %d", MBUF_DATA_SIZE, newSize);
		return false;
	}

	// Adjust the size of the mbuf to the new data
	if (oldSize < newSize)
	{
		// Append bytes to an mbuf
		// rte_pktmbuf_append:
		//		Append len bytes to an mbuf
		//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#ad5b0cd686ad3bcbb83416ca8395a080b
		//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#ad5b0cd686ad3bcbb83416ca8395a080b
		if (rte_pktmbuf_append(mBuf, newSize - oldSize) == NULL)
		{
			LOG_ERROR("Couldn't append %d bytes to mbuf", newSize - oldSize);
			return false;
		}
	}
	else if (oldSize > newSize) // May use (oldSize != newSize) - it must be faster
	{
		// Remove bytes at the beginning of an mbuf
		// rte_pktmbuf_adj:
		//		Remove len bytes at the beginning of an mbuf
		//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a1bbd752194759ce7b419c4998f2e8651
		//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a1bbd752194759ce7b419c4998f2e8651
		if (rte_pktmbuf_adj(mBuf, oldSize - newSize) == NULL)
		{
			LOG_ERROR("Couldn't remove %d bytes from mbuf", oldSize - newSize);
			return false;
		}
	}
	return true;
}

bool MBufRawPacket::allocateAndResize(rte_mbuf* &mBuf, rte_mempool* pool, std::size_t mBufSize)
{
	// mBuf is allocated with length of 0
	if (!allocate(mBuf, pool))
		return false;
	// Need to adjust it to the correct size
	if (!adjust(mBuf, 0, mBufSize))
		return false;
	return true;
}

bool MBufRawPacket::initialize(DpdkDevice* device)
{
	// Check if there is already a mBuf associated with this object
	if (m_MBuf != NULL)
	{
		LOG_ERROR("MBufRawPacket already initialized");
		return false;
	}
	// Try to allocate new mBuf in provided dpdkDevice
	if (!allocate(m_MBuf, device->m_MBufMempool))
		return false;
	// Set dpdkDevice 
	m_Device = device;
	return true;
}

MBufRawPacket::MBufRawPacket(const MBufRawPacket& other)
{
	// Clear data of base class
	RawPacket::initialize();
	// Clear this class data
	m_MBuf = NULL;
	m_Device = other.m_Device;
	// Check that other have a mBuf
	if (other.m_MBuf == NULL)
	{
		LOG_DEBUG("Copy construction form object in the not-initialized state.");
		// This object now is in proper not-initialized state - base class instance is in null-state
		return;
	}
	// Try to make a "clone" of underlying mBuf object in same pool as oroginal
	// allocateAndResize approach may be taken here but rte_pktmbuf_clone is faster
	// Clone is a packet of same dpdk structure not a packet data
	// rte_pktmbuf_clone:
	//		Creates a "clone" of the given packet mbuf
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a1d26cc982f6363cd7492dd70cc5c287c
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a1d26cc982f6363cd7492dd70cc5c287c
	rte_mbuf* newMbuf = rte_pktmbuf_clone(other.m_MBuf, other.m_MBuf->pool);
	if (newMbuf == NULL)
		return; // If exit here then object is in not-initialized state
	// Correctly set all fields that handles dpdk data 
	setMBuf(newMbuf, other.m_TimeStamp);
	// Copy data members of base class and copy packet data
	copyDataFrom(other, false);
}

MBufRawPacket& MBufRawPacket::operator=(const MBufRawPacket& other)
{
	// Do nothing in case of self assignment
	if (this == &other) 
		return *this;
	// Clear all base class fields
	RawPacket::initialize();

	if (m_MBuf == NULL)
	{
		// This must not be an error: initialize and copy state of other a call to copy constructor
		LOG_ERROR("MBufRawPacket isn't initialized");
		return *this;
	}

	if (!adjust(m_MBuf, m_RawDataLen, other.m_RawDataLen))
		return *this;
	// Correctly set all fields that handles dpdk data 
	setMBuf(m_MBuf, other.m_TimeStamp);
	// Copy data members of base class and copy packet data
	copyDataFrom(other, false);
	return *this;
}

MBufRawPacket::MBufRawPacket(MBufRawPacket&& other) :
	RawPacket(std::move(other)),
	m_MBuf(other.m_MBuf),
	m_Device(other.m_Device)
{
	// Lave other in not-initialized state
	other.m_MBuf = NULL;
}

MBufRawPacket& MBufRawPacket::operator=(MBufRawPacket&& other)
{
	// Do nothing in case of self assignment
	if (this == &other) 
		return *this;
	// Free underlying packet if it is present
	// rte_pktmbuf_free:
	//		Free a packet mbuf back into its original mempool			
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
	if (m_MBuf != NULL)
		rte_pktmbuf_free(m_MBuf);
	// Move data from other
	m_MBuf = other.m_MBuf;
	m_Device = other.m_Device;
	// Desable deallocation of raw data in next call
	m_RawPacketSet = false;
	// Call move-assignment operator of base class
	RawPacket::operator=(std::move(other));
	// Lave other in not-initialized state
	other.m_MBuf = NULL;
	return *this;
}

MBufRawPacket::~MBufRawPacket()
{
	// If mBuf is presented in current packet call dpdk mBuf deallocation function
	// rte_pktmbuf_free:
	//		Free a packet mbuf back into its original mempool	
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
	if (m_MBuf != NULL)
		rte_pktmbuf_free(m_MBuf);
}

bool MBufRawPacket::setRawData(const uint8_t* pRawData, int rawDataLen, timeval timestamp, LinkLayerType layerType, int frameLength)
{
	if (!pRawData)
	{
		LOG_ERROR("Provided pointer to data is nullptr.");
		return false;
	}
	if (rawDataLen > MBUF_DATA_SIZE)
	{
		LOG_ERROR("Cannot set raw data which length is larger than mBuf max size. mBuf max length: %d; requested length: %d.", MBUF_DATA_SIZE, rawDataLen);
		return false;
	}
	if (m_MBuf == NULL)
	{
		if (!allocateAndResize(m_MBuf, m_Device->m_MBufMempool, rawDataLen))
		{
			LOG_ERROR("Error occurred during allocation or resize operation.");
			return false;
		}
	}
	// rte_pktmbuf_mtod:
	//		A macro that points to the start of the data in the mbuf
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a2a8b10263496c7b580e9d0c7f2a1f073
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a2a8b10263496c7b580e9d0c7f2a1f073
	m_pRawData = rte_pktmbuf_mtod(m_MBuf, uint8_t*);
	// rte_pktmbuf_pkt_len: 
	//		A macro that returns the length of the carried data
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#ab3d5990c3ca14784ed3e0135844282a3
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a7a7ea614c79d5b9ed790a56c1c79189d
	m_RawDataLen = rte_pktmbuf_pkt_len(m_MBuf);
	// Copy memory content from other storage to dpdk storage
	std::memcpy(m_pRawData, pRawData, m_RawDataLen);
	// Free provided memory
	delete[] pRawData;
	// Setup other data members
	m_TimeStamp = timestamp;
	if (frameLength == -1)
		frameLength = m_RawDataLen;
	m_FrameLength = frameLength;
	m_linkLayerType = layerType;
	m_RawPacketSet = true;
	return true;
}

void MBufRawPacket::clear()
{
	if (m_MBuf != NULL)
	{
		// rte_pktmbuf_free:
		//		Free a packet mbuf back into its original mempool	
		//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
		//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#a1215458932900b7cd5192326fa4a6902
		rte_pktmbuf_free(m_MBuf);
		m_MBuf = NULL;
	}
	// Clear fields of base class
	RawPacket::initialize();
}

void MBufRawPacket::appendData(const uint8_t* dataToAppend, size_t dataToAppendLen)
{
	// Add memory to end of packet
	if (!adjust(m_MBuf, m_RawDataLen, m_RawDataLen + dataToAppendLen))
		return;
	// Call base class method to handle data
	RawPacket::appendData(dataToAppend, dataToAppendLen);
	LOG_DEBUG("Appended %d bytes to MBufRawPacket", (int)dataToAppendLen);
}

void MBufRawPacket::insertData(int atIndex, const uint8_t* dataToInsert, size_t dataToInsertLen)
{
	// Add memory to end of packet
	if (!adjust(m_MBuf, m_RawDataLen, m_RawDataLen + dataToInsertLen))
		return;
	// Call base class method to handle data
	RawPacket::insertData(atIndex, dataToInsert, dataToInsertLen);
	LOG_DEBUG("Inserted %d bytes to MBufRawPacket", (int)dataToInsertLen);
}

bool MBufRawPacket::removeData(int atIndex, size_t numOfBytesToRemove)
{
	// Trim method was not separated in the memory management API
	// So this is full implementation of it:
	if (m_MBuf == NULL)
	{
		LOG_ERROR("MBufRawPacket not initialized. Please call the init() method");
		return false;
	}
	// Call base class method to handle data
	if (!RawPacket::removeData(atIndex, numOfBytesToRemove))
		return false;
	// rte_pktmbuf_trim:
	//		Remove len bytes of data at the end of the mbuf
	//		http://dpdk.org/doc/api-17.02/rte__mbuf_8h.html#abe1bed439015acec170d1342e8dabb3b
	//		http://dpdk.org/doc/api-16.04/rte__mbuf_8h.html#abe1bed439015acec170d1342e8dabb3b
	if (rte_pktmbuf_trim(m_MBuf, numOfBytesToRemove) != 0)
	{
		LOG_ERROR("Couldn't trim the mBuf");
		return false;
	}
	LOG_DEBUG("Trimmed %d bytes from MBufRawPacket", (int)numOfBytesToRemove);
	return true;
}

bool MBufRawPacket::reallocateData(size_t newBufferLength)
{
	if ((int)newBufferLength < m_RawDataLen)
	{
		LOG_ERROR("Cannot reallocate mBuf raw packet to a smaller size. Current data length: %d; requested length: %d", m_RawDataLen, (int)newBufferLength);
		return false;
	}
	if (newBufferLength > MBUF_DATA_SIZE)
	{
		LOG_ERROR("Cannot reallocate mBuf raw packet to a size larger than mBuf data. mBuf max length: %d; requested length: %d", MBUF_DATA_SIZE, (int)newBufferLength);
		return false;
	}
	// no need to do any memory allocation because mbuf is already allocated
	return true;
}

/**
 * ================
 * Class DpdkDevice
 * ================
 */

#define DPDK_COFIG_HEADER_SPLIT			0 /**< Header Split disabled */
#define DPDK_COFIG_SPLIT_HEADER_SIZE	0
#define DPDK_COFIG_HW_IP_CHECKSUM		0 /**< IP checksum offload disabled */
#define DPDK_COFIG_HW_VLAN_FILTER		0 /**< VLAN filtering disabled */
#define DPDK_COFIG_JUMBO_FRAME			0 /**< Jumbo Frame Support disabled */
#define DPDK_COFIG_HW_STRIP_CRC			0 /**< CRC stripped by hardware disabled */
#define DPDK_CONFIG_MQ_MODE				ETH_RSS

//RSS random key:
uint8_t DpdkDevice::m_RSSKey[40] = {
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
	};

DpdkDevice::DpdkDevice(int port, uint32_t mBufPoolSize)
	: m_Id(port), m_MacAddress(MacAddress::Zero)
{
	snprintf((char*)m_DeviceName, 30, "DPDK_%d", m_Id);

	struct ether_addr etherAddr;
	rte_eth_macaddr_get((uint8_t) m_Id, &etherAddr);
	m_MacAddress = MacAddress(etherAddr.addr_bytes[0], etherAddr.addr_bytes[1],
			etherAddr.addr_bytes[2], etherAddr.addr_bytes[3],
			etherAddr.addr_bytes[4], etherAddr.addr_bytes[5]);

	rte_eth_dev_get_mtu((uint8_t) m_Id, &m_DeviceMtu);

	char mBufMemPoolName[32];
	sprintf(mBufMemPoolName, "MBufMemPool%d", m_Id);
	if (!initMemPool(m_MBufMempool, mBufMemPoolName, mBufPoolSize))
	{
		LOG_ERROR("Could not initialize mBuf mempool. Device not initialized");
		return;
	}

	m_NumOfRxQueuesOpened = 0;
	m_NumOfTxQueuesOpened = 0;

	setDeviceInfo();

	m_DeviceOpened = false;
	m_WasOpened = false;
	m_StopThread = true;
}

uint32_t DpdkDevice::getCurrentCoreId()
{
	return rte_lcore_id();
}

bool DpdkDevice::setMtu(uint16_t newMtu)
{
	int res = rte_eth_dev_set_mtu(m_Id, newMtu);
	if (res != 0)
	{
		LOG_ERROR("Couldn't set device MTU. DPDK error: %d", res);
		return false;
	}

	LOG_DEBUG("Managed to set MTU from %d to %d", m_DeviceMtu, newMtu);
	m_DeviceMtu = newMtu;
	return true;
}

bool DpdkDevice::openMultiQueues(uint16_t numOfRxQueuesToOpen, uint16_t numOfTxQueuesToOpen, const DpdkDeviceConfiguration& config)
{
	if (m_DeviceOpened)
	{
		LOG_ERROR("Device already opened");
		return false;
	}

	// There is a VMXNET3 limitation that when opening a device with a certain number of RX+TX queues
	// it's impossible to close it and open it again with a larger number of RX+TX queues. So for this
	// PMD I made a patch to open the device in the first time with maximum RX & TX queue, close it
	// immediately and open it again with number of queues the user wanted to
	if (!m_WasOpened && m_PMDType == PMD_VMXNET3)
	{
		m_WasOpened = true;
		openMultiQueues(getTotalNumOfRxQueues(), getTotalNumOfTxQueues(), config);
		close();
	}

	if (!configurePort(numOfRxQueuesToOpen, numOfTxQueuesToOpen))
	{
		m_DeviceOpened = false;
		return false;
	}

	m_Config = config;

	clearCoreConfiguration();

	if (!initQueues(numOfRxQueuesToOpen, numOfTxQueuesToOpen))
		return false;

	if (!startDevice())
	{
		LOG_ERROR("failed to start device %d\n", m_Id);
		m_DeviceOpened = false;
		return false;
	}

	m_NumOfRxQueuesOpened = numOfRxQueuesToOpen;
	m_NumOfTxQueuesOpened = numOfTxQueuesToOpen;

	rte_eth_stats_reset(m_Id);

	m_DeviceOpened = true;
	return m_DeviceOpened;
}

void DpdkDevice::close()
{
	if (!m_DeviceOpened)
	{
		LOG_DEBUG("Trying to close device [%s] but device is already closed", m_DeviceName);
		return;
	}
	stopCapture();
	clearCoreConfiguration();
	m_NumOfRxQueuesOpened = 0;
	m_NumOfTxQueuesOpened = 0;
	rte_eth_dev_stop(m_Id);
	LOG_DEBUG("Called rte_eth_dev_stop for device [%s]", m_DeviceName);
	m_DeviceOpened = false;
}

bool DpdkDevice::configurePort(uint8_t numOfRxQueues, uint8_t numOfTxQueues)
{
	if (numOfRxQueues > getTotalNumOfRxQueues())
	{
		LOG_ERROR("Could not open more than %d RX queues", getTotalNumOfRxQueues());
		return false;
	}

	if (numOfTxQueues > getTotalNumOfTxQueues())
	{
		LOG_ERROR("Could not open more than %d TX queues", getTotalNumOfTxQueues());
		return false;
	}


	// verify num of RX queues is power of 2
	bool isRxQueuePowerOfTwo = !(numOfRxQueues == 0) && !(numOfRxQueues & (numOfRxQueues - 1));
	if (!isRxQueuePowerOfTwo)
	{
		LOG_ERROR("Num of RX queues must be power of 2 (because of DPDK limitation). Attempetd to open device with %d RX queues", numOfRxQueues);
		return false;
	}

	struct rte_eth_conf portConf;
	std::memset(&portConf,0,sizeof(rte_eth_conf));
	portConf.rxmode.split_hdr_size = DPDK_COFIG_SPLIT_HEADER_SIZE;
	portConf.rxmode.header_split = DPDK_COFIG_HEADER_SPLIT;
	portConf.rxmode.hw_ip_checksum = DPDK_COFIG_HW_IP_CHECKSUM;
	portConf.rxmode.hw_vlan_filter = DPDK_COFIG_HW_VLAN_FILTER;
	portConf.rxmode.jumbo_frame = DPDK_COFIG_JUMBO_FRAME;
	portConf.rxmode.hw_strip_crc = DPDK_COFIG_HW_STRIP_CRC;
	portConf.rxmode.mq_mode = DPDK_CONFIG_MQ_MODE;
	portConf.rx_adv_conf.rss_conf.rss_key= DpdkDevice::m_RSSKey;
	portConf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IPV4 | ETH_RSS_IPV6;

	int res = rte_eth_dev_configure((uint8_t) m_Id, numOfRxQueues, numOfTxQueues, &portConf);
	if (res < 0)
	{
		LOG_ERROR("Failed to configure device [%s]. error is: '%s' [Error code: %d]\n", m_DeviceName, rte_strerror(res), res);
		return false;
	}

	LOG_DEBUG("Successfully called rte_eth_dev_configure for device [%s] with %d RX queues and %d TX queues", m_DeviceName, numOfRxQueues, numOfTxQueues);

	return true;
}

bool DpdkDevice::initQueues(uint8_t numOfRxQueuesToInit, uint8_t numOfTxQueuesToInit)
{
	rte_eth_dev_info devInfo;
	rte_eth_dev_info_get(m_Id, &devInfo);
	if (numOfRxQueuesToInit > devInfo.max_rx_queues)
	{
		LOG_ERROR("Num of RX queues requested for open [%d] is larger than RX queues available in NIC [%d]", numOfRxQueuesToInit, devInfo.max_rx_queues);
		return false;
	}

	if (numOfTxQueuesToInit > devInfo.max_tx_queues)
	{
		LOG_ERROR("Num of TX queues requested for open [%d] is larger than TX queues available in NIC [%d]", numOfTxQueuesToInit, devInfo.max_tx_queues);
		return false;
	}

	for (uint8_t i = 0; i < numOfRxQueuesToInit; i++)
	{
		int ret = rte_eth_rx_queue_setup((uint8_t) m_Id, i,
				m_Config.receiveDescriptorsNumber, 0,
				NULL, m_MBufMempool);

		if (ret < 0)
		{
			LOG_ERROR("Failed to init RX queue for device [%s]. Error was: '%s' [Error code: %d]", m_DeviceName, rte_strerror(ret), ret);
			return false;
		}
	}

	LOG_DEBUG("Successfully initialized %d RX queues for device [%s]", numOfRxQueuesToInit, m_DeviceName);

	for (uint8_t i = 0; i < numOfTxQueuesToInit; i++)
	{
		int ret = rte_eth_tx_queue_setup((uint8_t) m_Id, i,
				m_Config.transmitDescriptorsNumber,
					0, NULL);
		if (ret < 0)
		{
			LOG_ERROR("Failed to init TX queue #%d for port %d. Error was: '%s' [Error code: %d]", i, m_Id, rte_strerror(ret), ret);
			return false;
		}
	}

	LOG_DEBUG("Successfully initialized %d TX queues for device [%s]", numOfTxQueuesToInit, m_DeviceName);

	return true;
}

bool DpdkDevice::initMemPool(struct rte_mempool*& memPool, const char* mempoolName, uint32_t mBufPoolSize)
{
    bool ret = false;

    // Create transmission memory pool
    memPool = rte_mempool_create(mempoolName, // The name of the mempool
    		mBufPoolSize, // The number of elements in the mempool
            MBUF_SIZE, // The size of each element
            32, // cache_size
            sizeof(struct rte_pktmbuf_pool_private),// The size of the private data appended after the mempool structure
            rte_pktmbuf_pool_init, // A function pointer that is called for initialization of the pool
            NULL, // An opaque pointer to data that can be used in the mempool
            rte_pktmbuf_init, // A function pointer that is called for each object at initialization of the pool
            NULL, // An opaque pointer to data that can be used as an argument
            0, // socket identifier in the case of NUMA
            MEMPOOL_F_SC_GET); // Flags

    if (memPool == NULL)
	{
		LOG_ERROR("Failed to create packets memory pool for port %d, pool name: %s", m_Id, mempoolName);
	}
	else
	{
		LOG_DEBUG("Successfully initialized packets pool of size [%d] for device [%s]", mBufPoolSize, m_DeviceName);
		ret = true;
	}
    return ret;
}

bool DpdkDevice::startDevice()
{
	int ret = rte_eth_dev_start((uint8_t) m_Id);
	if (ret < 0)
	{
	    LOG_ERROR("Failed to start device %d. Error is %d", m_Id, ret);
		return false;
	}

	LinkStatus status;
	getLinkStatus(status);
	LOG_DEBUG("Device [%s] : Link %s; Speed: %d Mbps; %s",
			m_DeviceName,
			(status.linkUp ? "up" : "down"),
			status.linkSpeedMbps,
			(status.linkDuplex == LinkStatus::FULL_DUPLEX ? "full-duplex" : "half-duplex"));


	rte_eth_promiscuous_enable((uint8_t) m_Id);
	LOG_DEBUG("Started device [%s]", m_DeviceName);

	return true;
}

void DpdkDevice::clearCoreConfiguration()
{
	for (int i = 0; i < MAX_NUM_OF_CORES; i++)
	{
		m_CoreConfiguration[i].IsCoreInUse = false;
	}
}

int DpdkDevice::getCoresInUseCount()
{
	int res = 0;
	for (int i = 0; i < MAX_NUM_OF_CORES; i++)
		if (m_CoreConfiguration[i].IsCoreInUse)
			res++;

	return res;
}

void DpdkDevice::setDeviceInfo()
{
	rte_eth_dev_info portInfo;
	rte_eth_dev_info_get(m_Id, &portInfo);
	m_PMDName = std::string(portInfo.driver_name);

	if (m_PMDName == "eth_bond")
		m_PMDType = PMD_BOND;
	else if (m_PMDName ==  "rte_em_pmd")
		m_PMDType = PMD_E1000EM;
	else if (m_PMDName ==  "rte_igb_pmd")
		m_PMDType = PMD_IGB;
	else if (m_PMDName ==  "rte_igbvf_pmd")
		m_PMDType = PMD_IGBVF;
	else if (m_PMDName ==  "rte_enic_pmd")
		m_PMDType = PMD_ENIC;
	else if (m_PMDName == "rte_pmd_fm10k")
		m_PMDType = PMD_FM10K;
	else if (m_PMDName == "rte_i40e_pmd")
		m_PMDType = PMD_I40E;
	else if (m_PMDName == "rte_i40evf_pmd")
		m_PMDType = PMD_I40EVF;
	else if (m_PMDName == "rte_ixgbe_pmd")
		m_PMDType = PMD_IXGBE;
	else if (m_PMDName == "rte_ixgbevf_pmd")
		m_PMDType = PMD_IXGBEVF;
	else if (m_PMDName == "librte_pmd_mlx4")
		m_PMDType = PMD_MLX4;
	else if (m_PMDName == "eth_null")
		m_PMDType = PMD_NULL;
	else if (m_PMDName == "eth_pcap")
		m_PMDType = PMD_PCAP;
	else if (m_PMDName == "eth_ring")
		m_PMDType = PMD_RING;
	else if (m_PMDName == "rte_virtio_pmd")
		m_PMDType = PMD_VIRTIO;
	else if (m_PMDName == "rte_vmxnet3_pmd")
		m_PMDType = PMD_VMXNET3;
	else if (m_PMDName == "eth_xenvirt")
		m_PMDType = PMD_XENVIRT;
	else
		m_PMDType = PMD_UNKNOWN;

	m_PciAddress = PciAddress(
			portInfo.pci_dev->addr.domain,
			portInfo.pci_dev->addr.bus,
			portInfo.pci_dev->addr.devid,
			portInfo.pci_dev->addr.function);


	LOG_DEBUG("Device [%s] has %d RX queues", m_DeviceName, portInfo.max_rx_queues);
	LOG_DEBUG("Device [%s] has %d TX queues", m_DeviceName, portInfo.max_tx_queues);

	m_TotalAvailableRxQueues = portInfo.max_rx_queues;
	m_TotalAvailableTxQueues = portInfo.max_tx_queues;
}

bool DpdkDevice::isVirtual()
{
	switch (m_PMDType)
	{
	case PMD_IGBVF:
	case PMD_I40EVF:
	case PMD_IXGBEVF:
	case PMD_PCAP:
	case PMD_RING:
	case PMD_VIRTIO:
	case PMD_VMXNET3:
	case PMD_XENVIRT:
		return true;
	default:
		return false;
	}
}

void DpdkDevice::getLinkStatus(LinkStatus& linkStatus)
{
	struct rte_eth_link link;
	rte_eth_link_get((uint8_t) m_Id, &link);
	linkStatus.linkUp = link.link_status;
	linkStatus.linkSpeedMbps = (unsigned) link.link_speed;
	linkStatus.linkDuplex = (link.link_duplex == ETH_LINK_FULL_DUPLEX) ? LinkStatus::FULL_DUPLEX : LinkStatus::HALF_DUPLEX;
}

bool DpdkDevice::initCoreConfigurationByCoreMask(CoreMask coreMask)
{
	int i = 0;
	int numOfCores = getNumOfCores();
	clearCoreConfiguration();
	while ((coreMask != 0) && (i < numOfCores))
	{
		if (coreMask & 1)
		{
			if (i == DpdkDeviceList::getInstance().getDpdkMasterCore().Id)
			{
				LOG_ERROR("Core %d is the master core, you can't use it for capturing threads", i);
				clearCoreConfiguration();
				return false;
			}

			if (!rte_lcore_is_enabled(i))
			{
				LOG_ERROR("Trying to use core #%d which isn't initialized by DPDK", i);
				clearCoreConfiguration();
				return false;
			}
			m_CoreConfiguration[i].IsCoreInUse = true;
		}

		coreMask = coreMask >> 1;
		i++;
	}

	if (coreMask != 0) // this mean coreMask contains a core that doesn't exist
	{
		LOG_ERROR("Trying to use a core [%d] that doesn't exist while machine has %d cores", i, numOfCores);
		clearCoreConfiguration();
		return false;
	}

	return true;
}

bool DpdkDevice::startCaptureSingleThread(OnDpdkPacketsArriveCallback onPacketsArrive, void* onPacketsArriveUserCookie)
{
	if (!m_StopThread)
	{
		LOG_ERROR("Device already capturing. Cannot start 2 capture sessions at the same time");
		return false;
	}

	if (m_NumOfRxQueuesOpened != 1)
	{
		LOG_ERROR("Cannot start capturing on a single thread when more than 1 RX queue is opened");
		return false;
	}

	LOG_DEBUG("Trying to start capturing on a single thread for device [%s]", m_DeviceName);

	clearCoreConfiguration();

	m_OnPacketsArriveCallback = onPacketsArrive;
	m_OnPacketsArriveUserCookie = onPacketsArriveUserCookie;

	m_StopThread = false;

	for (int coreId = 0; coreId < MAX_NUM_OF_CORES; coreId++)
	{
    	if (coreId == (int)rte_get_master_lcore() || !rte_lcore_is_enabled(coreId))
    		continue;

    	m_CoreConfiguration[coreId].IsCoreInUse = true;
    	m_CoreConfiguration[coreId].RxQueueId = 0;

    	LOG_DEBUG("Trying to start capturing on core %d", coreId);
    	int err = rte_eal_remote_launch(dpdkCaptureThreadMain, (void*)this, coreId);
    	if (err != 0)
    	{
    		LOG_ERROR("Cannot create capture thread for device '%s'", m_DeviceName);
        	m_CoreConfiguration[coreId].IsCoreInUse = false;
    		return false;
    	}

    	LOG_DEBUG("Capturing started for device [%s]", m_DeviceName);
    	return true;
	}

	LOG_ERROR("Could not find initialized core so capturing thread cannot be initialized");
	return false;
}

bool DpdkDevice::startCaptureMultiThreads(OnDpdkPacketsArriveCallback onPacketsArrive, void* onPacketsArriveUserCookie, CoreMask coreMask)
{
	if (!m_DeviceOpened)
	{
		LOG_ERROR("Device not opened");
		return false;
	}

	if (!initCoreConfigurationByCoreMask(coreMask))
		return false;

	if (m_NumOfRxQueuesOpened != getCoresInUseCount())
	{
		LOG_ERROR("Cannot use a different number of queues and cores. Opened %d queues but set %d cores in core mask", m_NumOfRxQueuesOpened, getCoresInUseCount());
		clearCoreConfiguration();
		return false;
	}

	m_StopThread = false;
	int rxQueue = 0;
	for (int coreId = 0; coreId < MAX_NUM_OF_CORES; coreId++)
	{
		if (!m_CoreConfiguration[coreId].IsCoreInUse)
			continue;

		// create a new thread
		m_CoreConfiguration[coreId].RxQueueId = rxQueue++;
		int err = rte_eal_remote_launch(dpdkCaptureThreadMain, (void*)this, coreId);
		if (err != 0)
		{
			LOG_ERROR("Cannot create capture thread #%d for device '%s': [%s]", coreId, m_DeviceName, strerror(err));
			m_CoreConfiguration[coreId].clear();
			return false;
		}
	}

	m_OnPacketsArriveCallback = onPacketsArrive;
	m_OnPacketsArriveUserCookie = onPacketsArriveUserCookie;

	return true;
}

void DpdkDevice::stopCapture()
{
	LOG_DEBUG("Trying to stop capturing on device [%s]", m_DeviceName);
	m_StopThread = true;
	for (int coreId = 0; coreId < MAX_NUM_OF_CORES; coreId++)
	{
		if (!m_CoreConfiguration[coreId].IsCoreInUse)
			continue;
		rte_eal_wait_lcore(coreId);
		LOG_DEBUG("Thread on core [%d] stopped", coreId);
	}

	LOG_DEBUG("All capturing threads stopped");
}

int DpdkDevice::dpdkCaptureThreadMain(void *ptr)
{
	DpdkDevice* pThis = (DpdkDevice*)ptr;

	if (pThis == NULL)
	{
		LOG_ERROR("Failed to retrieve DPDK device in capture thread main loop");
		return 1;
	}

	uint32_t coreId = pThis->getCurrentCoreId();
	LOG_DEBUG("Starting capture thread %d", coreId);

	int queueId = pThis->m_CoreConfiguration[coreId].RxQueueId;

	while (likely(!pThis->m_StopThread))
	{
		uint32_t numOfPktsReceived = rte_eth_rx_burst(pThis->m_Id, queueId, pThis->m_mBufArray, RX_BURST_SIZE);

		if (unlikely(numOfPktsReceived == 0))
			continue;

		timeval time;
		gettimeofday(&time, NULL);

		if (likely(pThis->m_OnPacketsArriveCallback != NULL))
		{
			// THIS IS A PURE C DECLARATION THAT IS INVALID IN C++ (AND INVALID IN C STARTING WITH C11): see VLA http://en.cppreference.com/w/c/language/array
			MBufRawPacket rawPackets[numOfPktsReceived]; // <-- WTF??? THIS IS UB IN C++ AND MUST NOT COMPILE - the size of an array is a compile-time type property
			// Array here is created on stack and must be set to constant size. The best solution is to use RX_BURST_SIZE as an array length and allocate it 
			// only once outside of while(){} scope (right before the start of loop)
			for (uint32_t index = 0; index < numOfPktsReceived; ++index)
			{
				struct rte_mbuf* mBuf = pThis->m_mBufArray[index];
				rawPackets[index].setMBuf(mBuf, time);
			}

			pThis->m_OnPacketsArriveCallback(rawPackets, numOfPktsReceived, coreId, pThis, pThis->m_OnPacketsArriveUserCookie);
		}
	}

	LOG_DEBUG("Exiting capture thread %d", coreId);

	return 0;
}

void DpdkDevice::getStatistics(pcap_stat& stats)
{
	struct rte_eth_stats rteStats;
	rte_eth_stats_get(m_Id, &rteStats);
	stats.ps_recv = rteStats.ipackets;
	stats.ps_drop = rteStats.ierrors + rteStats.rx_nombuf;
	stats.ps_ifdrop = rteStats.rx_nombuf;
}

bool DpdkDevice::setFilter(GeneralFilter& filter)
{
	//TODO: I think DPDK supports filters
	LOG_ERROR("Filters aren't supported in DPDK device");
	return false;
}

bool DpdkDevice::setFilter(std::string filterAsString)
{
	//TODO: I think DPDK supports filters
	LOG_ERROR("Filters aren't supported in DPDK device");
	return false;
}

int DpdkDevice::sendPacketsInner(uint16_t txQueueId, void* packetStorage, packetIterator iter, int arrLength)
{
	if (!m_DeviceOpened)
	{
		LOG_ERROR("Device '%s' not opened!", m_DeviceName);
		return 0;
	}

	if (txQueueId < 0)
	{
		LOG_ERROR("txQueueId must be >= 0");
		return 0;
	}

	if (txQueueId >= m_NumOfTxQueuesOpened)
	{
		LOG_ERROR("TX queue %d isn't opened in device", txQueueId);
		return 0;
	}

	int totalPacketsToSend = arrLength;
	int packetIndex = 0;

	#define PACKET_TRANSMITION_THRESHOLD 0.8
	#define PACKET_TX_TRIES 1.5

	int mBufArraySize = (int)(m_Config.transmitDescriptorsNumber*PACKET_TRANSMITION_THRESHOLD);
	// THIS IS A PURE C DECLARATION THAT IS INVALID IN C++ (AND INVALID IN C STARTING WITH C11): see VLA http://en.cppreference.com/w/c/language/array
	rte_mbuf* mBufArr[mBufArraySize]; // <-- WTF??? THIS IS UB IN C++ AND MUST NOT COMPILE - the size of an array is a compile-time type property
	// Array here is created on stack and must be set to constant size.
	// The best way here is to allocate it via new and make this array part of DpdkDevice class and allocate it only once (a.e. if m_Config will not change in 
	// DpdkDevice object lifetime) when we first time come to this function.
	// If m_Config may change in DpdkDevice object lifetime there an cache mechanism needed to reallocate this array every time m_Config changes.
	// Best solution here is to use the gsl::span class object
	int packetsToSendInThisIteration = 0;
	int numOfSendFailures = 0;

	while (packetIndex < totalPacketsToSend && numOfSendFailures < 3)
	{
		RawPacket* rawPacket = iter(packetStorage, packetIndex);

		if (rawPacket->getRawDataLen() == 0)
		{
			LOG_ERROR("Cannot send a packet with size of 0");
			packetIndex++;
			continue;
		}

		rte_mbuf* newMBuf = rte_pktmbuf_alloc(m_MBufMempool);
		// out of mBuf resources, or
		// aggregated number of mBufs reached threshold (which is total number of DPDK TX descriptors * PACKET_TRANSMITION_THRESHOLD, or
		// only packetsToSendInThisIteration are left
		if (newMBuf == NULL || packetsToSendInThisIteration >= (m_Config.transmitDescriptorsNumber*PACKET_TRANSMITION_THRESHOLD) || packetIndex + packetsToSendInThisIteration == totalPacketsToSend)
		{
			// try to send packets currently in mBufArr
			LOG_DEBUG("Ready to send %d packets (packet indices: %d - %d) with rte_eth_tx_burst", packetsToSendInThisIteration, packetIndex, packetIndex+packetsToSendInThisIteration);
			int packetsSentInThisIteration = rte_eth_tx_burst(m_Id, txQueueId,
					mBufArr,
					packetsToSendInThisIteration);
			packetIndex += packetsSentInThisIteration;
			LOG_DEBUG("rte_eth_tx_burst sent %d out of %d. Packet index: %d", packetsSentInThisIteration, packetsToSendInThisIteration, packetIndex);

			// free all mBufs we allocated
			for (int i = 0; i < packetsToSendInThisIteration; i++)
				rte_pktmbuf_free(mBufArr[i]);

			if (packetsSentInThisIteration < packetsToSendInThisIteration)
			{
				LOG_DEBUG("Freed %d mBufs that weren't freed by rte_eth_tx_burst", packetsToSendInThisIteration - packetsSentInThisIteration);
				LOG_DEBUG("Since NIC couldn't send all packet in this iteration, waiting for 0.2 second for H/W descriptors to get free");
				usleep(200000);
			}

			// else - rte_eth_tx_burst succeeded
			packetsToSendInThisIteration = 0; // start new iteration
			if (newMBuf == NULL)
			{
				numOfSendFailures++;
				LOG_DEBUG("Couldn't allocate mBuf for transmitting, number of failures: %d", numOfSendFailures);
			}

			continue;
		}

		// else - rte_pktmbuf_alloc succeeded

		// mbuf is allocated with length of 0, need to adjust it to the size of the raw packet
		if (rte_pktmbuf_append(newMBuf, rawPacket->getRawDataLen()) == NULL)
		{
			LOG_ERROR("Couldn't set new allocated mBuf size to %d bytes", rawPacket->getRawDataLen());
			packetIndex++;
			continue;
		}

		if (rawPacket->getRawDataLen() > (int)rte_pktmbuf_pkt_len(newMBuf))
		{
			LOG_ERROR("Trying to send data with length larger than mBuf size. Requested length: %d; mBuf size: %d. Skipping RawPacket", rawPacket->getRawDataLen(), rte_pktmbuf_pkt_len(newMBuf));
			packetIndex++;
			continue;
		}


		uint8_t* mBufData = (uint8_t*)rte_pktmbuf_mtod(newMBuf, uint8_t*);
		if (std::memcpy(mBufData, rawPacket->getRawData(), rawPacket->getRawDataLen()) == NULL)
		{
			LOG_ERROR("Failed to copy RawPacket data to mBuf. Skipping RawPacket");
			packetIndex++;
			continue;
		}

		newMBuf->data_len = rawPacket->getRawDataLen();

		mBufArr[packetsToSendInThisIteration] = newMBuf;
		packetsToSendInThisIteration++;
		numOfSendFailures = 0;
	}

	LOG_DEBUG("All %d packets were sent successfully", packetIndex);
	return packetIndex;
}

bool DpdkDevice::receivePackets(RawPacketVector& rawPacketsArr, uint16_t rxQueueId)
{
	if (!m_DeviceOpened)
	{
		LOG_ERROR("Device not opened");
		return false;
	}

	if (!m_StopThread)
	{
		LOG_ERROR("DpdkDevice capture mode is currently running. Cannot recieve packets in parallel");
		return false;
	}

	if (rxQueueId >= m_TotalAvailableRxQueues)
	{
		LOG_ERROR("RX queue ID #%d not available for this device", rxQueueId);
		return false;
	}

	struct rte_mbuf* mBufArray[RX_BURST_SIZE];
	uint32_t numOfPktsReceived  = rte_eth_rx_burst(m_Id, rxQueueId, mBufArray, RX_BURST_SIZE);

	//the following line trashes the log with many messages. Uncomment only if necessary
	//LOG_DEBUG("Captured %d packets", numOfPktsReceived);

	if (unlikely(numOfPktsReceived <= 0))
	{
		return true;
	}

	timeval time;
	gettimeofday(&time, NULL);

	for (uint32_t index = 0; index < numOfPktsReceived; ++index)
	{
		struct rte_mbuf* mBuf = mBufArray[index];
		MBufRawPacket* newRawPacket = new MBufRawPacket();
		newRawPacket->setMBuf(mBuf, time);
		rawPacketsArr.pushBack(newRawPacket);
	}

	return true;
}

bool DpdkDevice::receivePackets(MBufRawPacket** rawPacketsArr, int& rawPacketArrLength, uint16_t rxQueueId)
{
	if (!m_DeviceOpened)
	{
		LOG_ERROR("Device not opened");
		return false;
	}

	if (!m_StopThread)
	{
		LOG_ERROR("DpdkDevice capture mode is currently running. Cannot recieve packets in parallel");
		return false;
	}

	if (rxQueueId >= m_TotalAvailableRxQueues)
	{
		LOG_ERROR("RX queue ID #%d not available for this device", rxQueueId);
		return false;
	}

	struct rte_mbuf* mBufArray[RX_BURST_SIZE];
	rawPacketArrLength = rte_eth_rx_burst(m_Id, rxQueueId, mBufArray, RX_BURST_SIZE);
	LOG_DEBUG("Captured %d packets", rawPacketArrLength);

	if (unlikely(rawPacketArrLength <= 0))
	{
		rawPacketArrLength = 0;
		rawPacketsArr = NULL;
		return true;
	}

	timeval time;
	gettimeofday(&time, NULL);

	MBufRawPacket* mBufArr = new MBufRawPacket[rawPacketArrLength];
	for (int index = 0; index < rawPacketArrLength; ++index)
	{
		struct rte_mbuf* mBuf = mBufArray[index];
		mBufArr[index].setMBuf(mBuf, time);
	}

	*rawPacketsArr = mBufArr;
	return true;
}

bool DpdkDevice::receivePackets(Packet** packetsArr, int& packetsArrLength, uint16_t rxQueueId)
{
	if (!m_DeviceOpened)
	{
		LOG_ERROR("Device not opened");
		return false;
	}

	if (!m_StopThread)
	{
		LOG_ERROR("DpdkDevice capture mode is currently running. Cannot recieve packets in parallel");
		return false;
	}

	if (rxQueueId >= m_TotalAvailableRxQueues)
	{
		LOG_ERROR("RX queue ID #%d not available for this device", rxQueueId);
		return false;
	}

	struct rte_mbuf* mBufArray[RX_BURST_SIZE];
	packetsArrLength = rte_eth_rx_burst(m_Id, rxQueueId, mBufArray, RX_BURST_SIZE);
	LOG_DEBUG("Captured %d packets", packetsArrLength);

	if (unlikely(packetsArrLength <= 0))
	{
		packetsArrLength = 0;
		packetsArr = NULL;
		return true;
	}

	timeval time;
	gettimeofday(&time, NULL);

    *packetsArr = new Packet[packetsArrLength];
	for (int index = 0; index < packetsArrLength; ++index)
	{
		struct rte_mbuf* mBuf = mBufArray[index];
		MBufRawPacket* newRawPacket = new MBufRawPacket();
		newRawPacket->setMBuf(mBuf, time);
		((*packetsArr)[index]).setRawPacket(newRawPacket, true);
	}

	return true;
}

RawPacket* getNextPacketFromRawPacketArray(void* packetStorage, int index)
{
	RawPacket* packetsArr = (RawPacket*)packetStorage;
	return &packetsArr[index];
}

RawPacket* getNextPacketFromPacketArray(void* packetStorage, int index)
{
	Packet** packetsArr = (Packet**)packetStorage;
	return packetsArr[index]->getRawPacket();
}

RawPacket* getNextPacketFromRawPacketVec(void* packetStorage, int index)
{
	RawPacketVector* packetVec = (RawPacketVector*)packetStorage;
	return packetVec->at(index);
}

int DpdkDevice::sendPackets(const RawPacket* rawPacketsArr, int arrLength, uint16_t txQueueId)
{
	return sendPacketsInner(txQueueId, (void*)rawPacketsArr, getNextPacketFromRawPacketArray, arrLength);
}

int DpdkDevice::sendPackets(const Packet** packetsArr, int arrLength, uint16_t txQueueId)
{
	return sendPacketsInner(txQueueId, (void*)packetsArr, getNextPacketFromPacketArray, arrLength);
}

int DpdkDevice::sendPackets(const RawPacketVector& rawPacketsVec, uint16_t txQueueId)
{
	return sendPacketsInner(txQueueId, (void*)(&rawPacketsVec), getNextPacketFromRawPacketVec, rawPacketsVec.size());
}

bool DpdkDevice::sendPacket(const uint8_t* packetData, int packetDataLength, uint16_t txQueueId)
{
	if (packetDataLength == 0)
	{
		LOG_ERROR("Trying to send a packet with length 0");
		return false;
	}

	timeval timestamp;
	RawPacket tempRawPacket(packetData, packetDataLength, timestamp, false);
	return (sendPackets(&tempRawPacket, 1, txQueueId) == 1);
}

bool DpdkDevice::sendPacket(const RawPacket& rawPacket, uint16_t txQueueId)
{
	return (sendPackets(&rawPacket, 1, txQueueId) == 1);
}

bool DpdkDevice::sendPacket(const Packet& packet, uint16_t txQueueId)
{
	const Packet* tempArr[1] = { &packet };
	return (sendPackets(tempArr, 1, txQueueId) == 1);
}

int DpdkDevice::getAmountOfFreeMbufs()
{
	return (int)rte_mempool_avail_count(m_MBufMempool);
}

int DpdkDevice::getAmountOfMbufsInUse()
{
	return (int)rte_mempool_in_use_count(m_MBufMempool);
}

} // namespace pcpp

#endif /* USE_DPDK */
