#ifndef PCAPPP_MBUF_RAWPACKET
#define PCAPPP_MBUF_RAWPACKET

#include <cstdlib>

#include "GenericRawPacket.h"

struct rte_mbuf;
struct rte_mempool;

#ifndef MBUF_DATA_SIZE
#	define MBUF_DATA_SIZE 2048
#endif

/// @file

/**
* \namespace pcpp
* \brief The main namespace for the PcapPlusPlus lib
*/
namespace pcpp
{
	// Forward declaration
	class DpdkDevice;

	/**
	 * \namespace memory
	 * \brief Namespace that contains memory manipulation features
	 */
	namespace memory
	{
		
		class DPDKMemoryProxy :
			public MemoryProxyInterface< ::pcpp::memory::Data_t >
		{
			friend class DpdkDevice;
		public:

			typedef struct rte_mbuf* mbuf_ptr;

			typedef MemoryProxyInterface< ::pcpp::memory::Data_t > Base;

			using Base::value_type;
			using Base::pointer;
			using Base::const_pointer;
			using Base::reference;
			using Base::const_reference;

			using Base::size;
			using Base::length;
			using Base::index;
			using Base::memory_value;

		protected:

			/**
			 * @brief Initialize an instance of this class. 
			 * Initialization includes allocating an mBuf from the pool that resides in DpdkDevice.\n
			 * The user should call this method only once per instance. Calling it more than once will result with an error.\n
			 * If fails provided DpdkDevice will not be set.
			 * @param[in] device The DpdkDevice which has the pool to allocate the mbuf from.
			 * @return True if initialization succeeded and false if this method was already called for this instance (and an mbuf is
			 * already attached) or if allocating an mbuf from the pool failed.
			 */
			inline bool initialize()
			{
				m_MBuf = PCAPPP_NULLPTR;
				m_Device = PCAPPP_NULLPTR;
			}

			/// Memory management API

			/**
			 * @brief Tries to provided mBuf as underlying to this object.
			 * If current mBuf is presented and provided mBuf is not same as current frees current mBuf first.\n
			 * Internally calls RawPacket::setRawData to set base class fields - may fail if mBuf content length is 0.
			 * @param[in] mBuf Pointer to new mBuf.
			 * @return true if set operation succeeded, false otherwise.
			 */
			bool setMBuf(mbuf_ptr mBuf);
		
			/**
			 * @brief Tries to allocate new mBuf in pool via call to rte_pktmbuf_alloc.
			 * Fails if provided pool is nullptr or if call to rte_pktmbuf_alloc returns NULL.
			 * @param[out] mBuf Reference to pointer, in which address of new mBuf will be written.
			 * @param[in] pool MBuf pool in which an allocation takes place.
			 * @return true if allocation succeeded, false otherwise.
			 */
			static bool allocate(mbuf_ptr& mBuf, rte_mempool* pool);

			/**
			 * @brief Tries to change underlying mBuf size.
			 * Adjustment is done via calls to rte_pktmbuf_append or rte_pktmbuf_adj.\n
			 * Fails if mBuf is not valid pointer, newSize is higher than MBUF_DATA_SIZE or if rte_pktmbuf_append or rte_pktmbuf_adj returns NULL.
			 * @param[out] mBuf Reference to pointer, in which lies address of mBuf to be adjusted.
			 * @param[in] oldSize Current size of mBuf.
			 * @param[in] newSize New size of mBuf.
			 * @return true if adjustment succeeded, false otherwise.
			 */
			static bool adjust(mbuf_ptr& mBuf, std::size_t oldSize, std::size_t newSize);

			/**
			 * @brief Composition of calls to allocate and adjust.
			 * Fails if one of the calls fails.
			 * @param[out] mBuf Reference to pointer, in which address of new mBuf will be written.
			 * @param[in] pool MBuf pool in which an allocation takes place.
			 * @param[in] mBufSize Size of mBuf to be set.
			 * @return true on success, false otherwise.
			 */
			static bool allocateAndResize(mbuf_ptr& mBuf, rte_mempool* pool, std::size_t mBufSize);

		public:

			/**
			 * @brief Default constructor.
			 * Sets object to a null-state by internally calling initialize method.
			 */
			DPDKMemoryProxy() { initialize(); }

			/**
			 * @brief A copy constructor for this class. 
			 * The copy constructor allocates a new mbuf from the same pool the original mbuf was allocated from, 
			 * attaches the new mbuf to this instance of MBufRawPacket 
			 * and copies the data from the original mbuf to the new mbuf.
			 * @param[in] other The MBufRawPacket instance to make copy of.
			 */
			DPDKMemoryProxy(const DPDKMemoryProxy& other);

			/**
			 * @brief An copy assignment operator for this class. 
			 * Copies the data from the mbuf attached to the other MBufRawPacket to the mbuf attached to this instance. 
			 * If instance is not initialized (meaning no mbuf is attached) nothing will be copied and
			 * instance will remain uninitialized (also, an error will be printed).
			 * @param[in] other The MBufRawPacket instance to make copy of.
			 */
			DPDKMemoryProxy& operator=(const DPDKMemoryProxy& other);

			/**
			 * @brief A move constructor for this class.
			 * Moves all data from another instance. Internaly calls RawPacket move c'tor to move base class data.\n
			 * other object is set in not-initialized state.
			 * other.m_MBuf is set to NULL but other.m_Device isn't touched.
			 * @param[in] other The MBufRawPacket instance to move from.
			 */
			PCAPPP_MOVE_CONSTRUCTOR(DPDKMemoryProxy);

			/**
			 * @brief An move assignment operator for this class.
			 * If current underlying mBuf is set frees it first. Moves the data from other without copying it.\n
			 * other object is set in not-initialized state.
			 * other.m_MBuf is set to NULL but other.m_Device isn't touched.
			 * @param[in] other The MBufRawPacket instance to move from.
			 */
			PCAPPP_MOVE_ASSIGNMENT(DPDKMemoryProxy);

			/**
			 * @brief Destructor for this class. 
			 * Once called it frees the mbuf attached to it (returning it back to the mbuf pool it was allocated from).
			 */
			~DPDKMemoryProxy();

			/**
			 * @brief Returns known underlying data length.
			 * @return Known underlying data length.
			 */
			inline size getLength() const PCAPPP_OVERRIDE;
			
			/**
			 * @brief Returns owning status of underlying data.
			 * @return true if object owns it's underlying data, false otherwise.
			 */
			inline bool isOwning() const PCAPPP_OVERRIDE { return true; }
			
			/**
			 * @brief Returns pointer to the beginning of underlying data.
			 * @return Pointer to the beginning of underlying data.
			 */
			inline pointer get() PCAPPP_OVERRIDE;
			
			/**
			 * @brief Returns pointer to the beginning of const-qualified underlying data.
			 * This overload is called in object is const-qualified.
			 * @return Pointer to the beginning of const-qualified underlying data.
			 */
			inline const_pointer get() const PCAPPP_OVERRIDE;
			
			/**
			 * @brief The ownership release method.
			 * Firstly saves current data pointer in temporary pointer.
			 * Next internally calls initialize method.
			 * Then returns saved pointer.
			 * @return Pointer to the beginning of underlying data.
			 */
			pointer release() PCAPPP_OVERRIDE { return PCAPPP_NULLPTR; }

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
			bool reset(pointer ptr, size length = 0, bool = true) PCAPPP_OVERRIDE;

			/**
			 * @brief Method to check if current object is in the null-state.
			 * @return true if object is NOT in the null-state, false otherwise.
			 */
			operator bool() const { return m_MBuf; }

			/**
			 * @brief Represents the reallocation operation of underlying data.
			 * Reallocation operation is defined as following set of steps:  
			 * * Allocate new memory with size newBufferLength via allocator;  
			 * * Set new memory content per byte as initialValue;  
			 * * Copy old memory content to new memory;  
			 * * Deallocate old memory via allocator.  
			 *  
			 * @param[in] newBufferLength New size of data.
			 * @param[in] initialValue Per-byte initial value of new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool reallocate(size newBufferLength, memory_value = 0);
			
			/**
			 * @brief Represents the clear operation.
			 * Call to clear operation signifies object to deallocate it's underlying data and set it self into a null-state.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool clear() = 0;
			
			/**
			 * @brief Represents the append operation over underlying data.
			 * Append operation must add dataToAppendLen count of data entries to the end of data.\n
			 * This overload sets memory for appended data to specified initialValue.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool append(size dataToAppendLen, memory_value initialValue = 0);
			
			/**
			 * @brief Represents the concatenation operation over underlying and provided data.
			 * Append operation must add dataToAppendLen count of data entries to the end of data.\n
			 * This overload sets memory for appended data same as in provided dataToAppend buffer.
			 * @param[in] dataToAppend Buffer memory to be appended to current data.
			 * @param[in] dataToAppendLen Size of data to be appended.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool append(const_pointer dataToAppend, size dataToAppendLen);
			
			/**
			 * @brief Handles negative values of atIndex for insert operation.
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 */
			bool insert_back(index atIndex, size dataToInsertLen, memory_value initialValue = 0);

			/**
			 * @brief Represents the insertion operation over underlying data.
			 * Insert operation must add dataToAppendLen count of data entries before the specified location in current data.\n
			 * This overload sets memory for inserted data to specified initialValue.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @param[in] initialValue Initial value for new memory.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool insert(index atIndex, size dataToInsertLen, memory_value initialValue = 0);
			
			/**
			 * @brief Handles negative values of atIndex for insert operation (with data).
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 */
			bool insert_back(index atIndex, const_pointer dataToInsert, size dataToInsertLen);

			/**
			 * @brief Represents the insertion operation over underlying and provided data.
			 * Insert operation must add dataToAppendLen count of data entries before the specified location in current data.\n
			 * This overload sets memory for inserted data same as in provided dataToAppend buffer.
			 * @param[in] atIndex Index before which insertion take place.
			 * @param[in] dataToInsert Buffer memory to be inserted to current data.
			 * @param[in] dataToInsertLen Size of data to be inserted.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen);
			
			/**
			 * @brief Handles negative values of atIndex for remove operation.
			 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
			 */
			bool remove_back(index atIndex, size numOfBytesToRemove);

			/**
			 * @brief Represents the remove operation over underlying data.
			 * Remove operation must remove numOfBytesToRemove count of data entries starting from the specified location in current data.
			 * @param[in] atIndex Index from which removal take place.
			 * @param[in] numOfBytesToRemove Size of data to be removed.
			 * @return true if operation finished successfully, false otherwise.
			 */
			bool remove(index atIndex, size numOfBytesToRemove);
		protected:
			mbuf_ptr m_MBuf;
			DpdkDevice* m_Device;
		};

	} // namespace pcpp::memory

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
	class MBufRawPacket :
		public GenericRawPacket< ::pcpp::MemoryProxyTags::DpdkTag >
	{
	public:

		typedef GenericRawPacket< ::pcpp::MemoryProxyTags::DpdkTag > Base;

		/**
		 * @brief Initialize an instance of this class. 
		 * Initialization includes allocating an mBuf from the pool that resides in DpdkDevice.\n
		 * The user should call this method only once per instance. Calling it more than once will result with an error.\n
		 * If fails provided DpdkDevice will not be set.
		 * @param[in] device The DpdkDevice which has the pool to allocate the mbuf from.
		 * @return True if initialization succeeded and false if this method was already called for this instance (and an mbuf is
		 * already attached) or if allocating an mbuf from the pool failed.
		 */
		bool initialize(DpdkDevice* device);

		/**
		 * @brief A default c'tor for this class. 
		 * Constructs an instance of this class without an mbuf attached to it. In order to allocate
		 * an mbuf the user should call the initialize() method. Without calling initialize() the instance of this class is not usable.\n
		 * This c'tor can be used for initializing an array of MBufRawPacket (which requires an empty c'tor).
		 */
		MBufRawPacket() {}

		/**
		 * @brief A copy c'tor for this class. 
		 * The copy c'tor allocates a new mbuf from the same pool the original mbuf was allocated from, 
		 * attaches the new mbuf to this instance of MBufRawPacket 
		 * and copies the data from the original mbuf to the new mbuf.
		 * @param[in] other The MBufRawPacket instance to make copy of.
		 */
		MBufRawPacket(const MBufRawPacket& other) : Base(other) {}

		/**
		 * @brief An copy assignment operator for this class. 
		 * Copies the data from the mbuf attached to the other MBufRawPacket to the mbuf attached to this instance. 
		 * If instance is not initialized (meaning no mbuf is attached) nothing will be copied and
		 * instance will remain uninitialized (also, an error will be printed).
		 * @param[in] other The MBufRawPacket instance to make copy of.
		 */
		MBufRawPacket& operator=(const MBufRawPacket& other)
		{
			if (this == &other)
				return *this;
			Base::operator=(other);
			return *this;
		}

		/**
		 * @brief A move constructor for this class.
		 * Moves all data from another instance. Internally calls RawPacket move c'tor to move base class data.\n
		 * other object is set in not-initialized state.
		 * other.m_MBuf is set to NULL but other.m_Device isn't touched.
		 * @param[in] other The MBufRawPacket instance to move from.
		 */
		PCAPPP_MOVE_CONSTRUCTOR(MBufRawPacket) :
			Base(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER)) {}

		/**
		 * @brief An move assignment operator for this class.
		 * If current underlying mBuf is set frees it first. Moves the data from other without copying it.\n
		 * other object is set in not-initialized state.
		 * other.m_MBuf is set to NULL but other.m_Device isn't touched.
		 * @param[in] other The MBufRawPacket instance to move from.
		 */
		PCAPPP_MOVE_ASSIGNMENT(MBufRawPacket)
		{
			if (this == &other)
				return *this;
			Base::operator=(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER));
			return *this;
		}
	};

} // namespace pcpp

#endif /* PCAPPP_MBUF_RAWPACKET */