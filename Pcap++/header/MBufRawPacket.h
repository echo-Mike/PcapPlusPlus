#ifndef PCAPPP_MBUF_RAWPACKET
#define PCAPPP_MBUF_RAWPACKET

#include <cstdlib>
#include "RawPacket.h"

struct rte_mbuf;
struct rte_mempool;

/// @file

/**
* \namespace pcpp
* \brief The main namespace for the PcapPlusPlus lib
*/
namespace pcpp
{
	class DpdkDevice;

	/**
	 * @class MBufRawPacket
	 * A class that inherits RawPacket and wraps DPDK's mbuf object (see some info about mbuf in DpdkDevice.h) but is
	 * compatible with PcapPlusPlus framework. Using MBufRawPacket is be almost similar to using RawPacket, the implementation 
	 * differences are encapsulated in the class implementation. For example: user can create and manipulate a Packet object from 
	 * MBufRawPacket the same way it is done with RawPacket; User can use PcapFileWriterDevice to save MBufRawPacket to pcap the 
	 * same way it's used with RawPacket; etc.<BR>
	 * The main difference is that RawPacket contains a pointer to the data itself and MBufRawPacket is holding a pointer to an mbuf
	 * object which contains a pointer to the data. This implies that MBufRawPacket without an mbuf allocated to it is not usable.
	 * Getting instances of MBufRawPacket can be done in one to the following ways:
	 *    - Receiving packets from DpdkDevice. In this case DpdkDevice takes care of getting the mbuf from DPDK and wrapping it with
	 *      MBufRawPacket
	 *    - Creating MBufRawPacket from scratch (in order to send it with DpdkDevice, for example). In this case the user should call
	 *      the init() method after constructing the object in order to allocate a new mbuf from DPDK port pool (encapsulated by DpdkDevice)
	 * 
	 * Limitations of this class:
	 *    - Currently chained mbufs are not supported. An mbuf has the capability to be linked to another mbuf and create a linked list
	 *      of mbufs. This is good for Jumbo packets or other uses. MBufRawPacket doesn't support this capability so there is no way to
	 *      access the mbufs linked to the mbuf wrapped by MBufRawPacket instance. I hope I'll be able to add this support in the future
	 */
	class MBufRawPacket : public RawPacket
	{
		friend class DpdkDevice;
		struct rte_mbuf* m_MBuf;
		DpdkDevice* m_Device;

		/// Memory management API

		/**
		 * @brief Tries to provided mBuf as underlying to this object.
		 * If current mBuf is presented and provided mBuf is not same as current frees current mBuf first.\n
		 * Internally calls RawPacket::setRawData to set base class fields - may fail if mBuf contrent length is 0.
		 * @param[in] mBuf Pointer to new mBuf.
		 * @param[in] timestamp Packet timestamp to be set.
		 * @return true if set operation succeeded, false otherwise.
		 */
		bool setMBuf(struct rte_mbuf* mBuf, timeval timestamp);
		
		/**
		 * @brief Tries to allocate new mBuf in pool via call to rte_pktmbuf_alloc.
		 * Fails if provided pool is nullptr or if call to rte_pktmbuf_alloc returns NULL.
		 * @param[out] mBuf Reference to pointer, in which address of new mBuf will be written.
		 * @param[in] pool MBuf pool in which an allocation takes place.
		 * @return true if allocatoion succeeded, false otherwise.
		 */
		static bool allocate(rte_mbuf* &mBuf, rte_mempool* pool);

		/**
		 * @brief Tries to change underlying mBuf size.
		 * Adjustment is done via calls to rte_pktmbuf_append or rte_pktmbuf_adj.\n
		 * Fails if mBuf is not valid pointer, newSize is higher than MBUF_DATA_SIZE or if rte_pktmbuf_append or rte_pktmbuf_adj returns NULL.
		 * @param[out] mBuf Reference to pointer, in which lies address of mBuf to be adjusted.
		 * @param[in] oldSize Current size of mBuf.
		 * @param[in] newSize New size of mBuf.
		 * @return true if adjustment succeeded, false otherwise.
		 */
		static bool adjust(rte_mbuf* &mBuf, std::size_t oldSize, std::size_t newSize);

		/**
		 * @brief Composition of calls to allocate and adjust.
		 * Fails if one of the calls fails.
		 * @param[out] mBuf Reference to pointer, in which address of new mBuf will be written.
		 * @param[in] pool MBuf pool in which an allocation takes place.
		 * @param[in] mBufSize Size of mBuf to be set.
		 * @return true on success, false otherwise.
		 */
		static bool allocateAndResize(rte_mbuf* &mBuf, rte_mempool* pool, std::size_t mBufSize);

	public:

		/**
		 * @brief Initialize an instance of this class. 
		 * Initialization includes allocating an mBuf from the pool that resides in DpdkDevice.\n
		 * The user should call this method only once per instance. Calling it more than once will result with an error.\n
		 * If fails provided DpdkDevice will not be set.
		 * @param[in] device The DpdkDevice which has the pool to allocate the mbuf from.
		 * @return True if initialization succeeded and false if this method was already called for this instance (and an mbuf is
		 * already attched) or if allocating an mbuf from the pool failed.
		 */
		bool initialize(DpdkDevice* device);

		/**
		 * @brief A default c'tor for this class. 
		 * Constructs an instance of this class without an mbuf attached to it. In order to allocate
		 * an mbuf the user should call the initialize() method. Without calling initialize() the instance of this class is not usable.\n
		 * This c'tor can be used for initializing an array of MBufRawPacket (which requires an empty c'tor).
		 */
		MBufRawPacket() : RawPacket(), m_MBuf(NULL), m_Device(NULL) {}

		/**
		 * @brief A copy c'tor for this class. 
		 * The copy c'tor allocates a new mbuf from the same pool the original mbuf was allocated from, 
		 * attaches the new mbuf to this instance of MBufRawPacket 
		 * and copies the data from the original mbuf to the new mbuf.
		 * @param[in] other The MBufRawPacket instance to make copy of.
		 */
		MBufRawPacket(const MBufRawPacket& other);

		/**
		 * @brief An copy assignment operator for this class. 
		 * Copies the data from the mbuf attached to the other MBufRawPacket to the mbuf attached to this instance. 
		 * If instance is not initialized (meaning no mbuf is attached) nothing will be copied and
		 * instance will remain uninitialized (also, an error will be printed).
		 * @param[in] other The MBufRawPacket instance to make copy of.
		 */
		MBufRawPacket& operator=(const MBufRawPacket& other);

		/**
		 * @brief A move constructor for this class.
		 * Moves all data from another instance. Internaly calls RawPacket move c'tor to move base class data.\n
		 * other object is set in not-initialized state.
		 * other.m_MBuf is set to NULL but other.m_Device isn't touched.
		 * @param[in] other The MBufRawPacket instance to move from.
		 */
		MBufRawPacket(MBufRawPacket&& other);

		/**
		* @brief An move assignment operator for this class.
		* If current underlying mBuf is set frees it first. Moves the data from other without copying it.\n
		* other object is set in not-initialized state.
		* other.m_MBuf is set to NULL but other.m_Device isn't touched.
		* @param[in] other The MBufRawPacket instance to move from.
		*/
		MBufRawPacket& operator=(MBufRawPacket&& other);

		/**
		 * @brief A d'tor for this class. 
		 * Once called it frees the mbuf attached to it (returning it back to the mbuf pool it was allocated from).
		 */
		~MBufRawPacket();

		/**
		 * @brief Set raw data to the mbuf by copying the data to it. 
		 * In order to stay compatible with the ancestor method which takes control of the data pointer 
		 * and frees it when RawPacket is destroyed, this method frees this pointer right away after
		 * data is copied to the mbuf. So when using this method please notice that after it's called pRawData memory is free, don't
		 * use this pointer again. In addition, if raw packet isn't initialized (mbuf is NULL), this method will call the initialize() method.
		 * @param[in] pRawData A pointer to the new raw data.
		 * @param[in] rawDataLen The new raw data length in bytes.
		 * @param[in] timestamp The timestamp packet was received by the NIC.
		 * @param[in] layerType The link layer type for this raw data.
		 * @param[in] frameLength When reading from pcap files, sometimes the captured length is different from the actual packet length.
		 * @return True if raw data was copied to the mbuf successfully, false if rawDataLen is larger than mbuf max size, if initialization
		 * failed or if copying the data to the mbuf failed. In all of these cases an error will be printed to log
		 */
		bool setRawData(const uint8_t* pRawData, int rawDataLen, timeval timestamp, LinkLayerType layerType = LINKTYPE_ETHERNET, int frameLength = -1);

		/**
		 * @brief Clears the object and frees the mbuf.
		 * Internally calls RawPacket::initialize to clear base class fields.
		 */
		void clear();

		/**
		 * @brief Append packet data at the end of current data. 
		 * This method uses the same mbuf already allocated and tries to append more space and copy the data to it.\n
		 * If MBufRawPacket is not initialize (mbuf is NULL) or mbuf append failed an error is printed to log.\n
		 * Internally calls RawPacket::appendData to handle data movement operations.
		 * @param[in] dataToAppend A pointer to the data to append.
		 * @param[in] dataToAppendLen Length in bytes of dataToAppend.
		 */
		void appendData(const uint8_t* dataToAppend, size_t dataToAppendLen);

		/**
		 * @brief Insert raw data at some index of the current data and shift the remaining data to the end. 
		 * This method uses the same mbuf already allocated and tries to append more space to it. 
		 * Then it just copies dataToAppend at the relevant index and shifts the remaining data to the end. 
		 * If MBufRawPacket is not initialize (mbuf is NULL) or mbuf append failed an error is printed to log.\n
		 * Internally calls RawPacket::insertData to handle data movement operations.
		 * @param[in] atIndex The index to insert the new data to
		 * @param[in] dataToInsert A pointer to the new data to insert
		 * @param[in] dataToInsertLen Length in bytes of dataToInsert
		 */
		void insertData(int atIndex, const uint8_t* dataToInsert, size_t dataToInsertLen);

		/**
		 * @brief Remove certain number of bytes from current raw data buffer. 
		 * All data after the removed bytes will be shifted back. 
		 * This method uses the mbuf already allocated and tries to trim space from it.\n
		 * Internally calls RawPacket::removeData to handle data movement operations.\n
		 * Trim operation is taken place only after RawPacket::removeData was successful.
		 * @param[in] atIndex The index to start removing bytes from.
		 * @param[in] numOfBytesToRemove Number of bytes to remove.
		 * @return True if all bytes were removed successfully, or false if MBufRawPacket is not initialize (mbuf is NULL), mbuf trim
		 * failed or logatIndex+numOfBytesToRemove is out-of-bounds of the raw data buffer. In all of these cases an error is printed to log.
		 */
		bool removeData(int atIndex, size_t numOfBytesToRemove);

		/**
		 * This overridden method,in contrast to its ancestor RawPacket#reallocateData() doesn't need to do anything because mbuf is already
		 * allocated to its maximum extent. So it only performs a check to verify the size after re-allocation doesn't exceed mbuf max size
		 * @param[in] newBufferLength The new buffer length as required by the user
		 * @return True if new size is larger than current size but smaller than mbuf max size, false otherwise
		 */
		bool reallocateData(size_t newBufferLength);
	};
} // namespace pcpp

#endif /* PCAPPP_MBUF_RAWPACKET */