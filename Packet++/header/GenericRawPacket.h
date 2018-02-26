#ifndef PCAPPP_GENERIC_RAW_PACKET
#define PCAPPP_GENERIC_RAW_PACKET

#include <CPP11.h>
#include <TypeUtils.h>
#include <MoveSemantics.h>
#include <MemoryProxy.h>
#include "RawPacket.h"

/// @file

/**
 * \namespace pcpp
 * \brief The main namespace for the PcapPlusPlus lib
 */
namespace pcpp
{
	/**
	 * @brief Represents a generic binding between RawPacket and MemoryProxy interfaces.
	 * Supports automatic deduction of MemoryProxy type.
	 * @tparam MemoryProxyTag One of the tag types defined in pcpp::memory::MemoryProxyTags namesapce. Used for auto deduction.
	 * @tparam MemoryProxyT Type of memory proxy to bind. By default may be deduced from MemoryProxyTag (except for CustomTag).
	 * @tparam unnamed Raises a compile-time error if provided MemoryProxyT do not implement memory::MemoryProxyInterface< Data_t > interface.
	 */
	template <
		typename MemoryProxyTag = ::pcpp::memory::MemoryProxyTags::OldMemoryModelTag,
		typename MemoryProxyT = typename ::pcpp::memory::MemoryProxyDispatcher< MemoryProxyTag >::memory_proxy_t,
		typename = typename ::pcpp::type_traits::enable_if<
			::pcpp::type_traits::is_base_of<
				::pcpp::memory::MemoryProxyInterface< ::pcpp::memory::Data_t >,
				MemoryProxyT
			>::value
		>::type
	>
	class GenericRawPacket PCAPPP_FINAL :
		public RawPacket,
		protected MemoryProxyT
	{
	public:

		/* MemoryProxy typedefs */

		/**
		 * Convenient alias typedef for base memory proxy class. 
		 */
		typedef MemoryProxyT MPBase;
		/**
		 * Defines type of tag used to deduce memory proxy type.
		 */
		typedef MemoryProxyTag tag_t;

		/* RawPacket typedefs */

		/**
		 * Convenient alias typedef for base raw packet class. 
		 */
		typedef RawPacket Base;
		/**
		 * Defines underlying memory type.
		 */
		typedef Base::value_type value_type;
		/**
		 * Defines type of pointer to underlying memory type.
		 */
		typedef Base::pointer pointer;
		/**
		 * Defines type of constant pointer to underlying memory type.
		 */
		typedef Base::const_pointer const_pointer;
		/**
		 * Defines type of reference to element of underlying memory type.
		 */
		typedef Base::reference reference;
		/**
		 * Defines type of constant reference to element of underlying memory type.
		 */
		typedef Base::const_reference const_reference;

		/**
		 * Defines type of variables that represents size values.
		 */
		typedef Base::size size;
		/**
		 * Defines type of variables that represents length values.
		 */
		typedef Base::length length;
		/**
		 * Defines type of variables that represents index values.
		 */
		typedef Base::index index;
		/**
		 * Defines type of variables that represents initial memory values for std::memset.
		 */
		typedef Base::memory_value memory_value;

		/**
		 * @brief A default constructor that initializes class'es attributes to default value.
		 * Internally calls initialize member functions of both base classes (via their constructors).
		 */
		GenericRawPacket() {}

		/**
		 * A constructor that receives a pointer to the raw data. 
		 * This constructor is usually used when packet is captured using a packet capturing engine (like libPcap. WinPcap, PF_RING, etc.). 
		 * The capturing engine allocates the raw data memory and give the user a pointer to it + a time-stamp it has arrived to the device.
		 * @param[in] pRawData A pointer to the raw data.
		 * @param[in] rawDataLen The raw data length in bytes.
		 * @param[in] timestamp The time-stamp packet was received by the NIC.
		 * @param[in] ownership An indicator whether raw data should be freed upon object destruction.
		 * @param[in] layerType The link layer type of this raw packet. The default is Ethernet
		 */
		GenericRawPacket(const_pointer pRawData, size rawDataLen, timeval timestamp, bool ownership, LinkLayerType layerType = LINKTYPE_ETHERNET) :
			Base(timestamp, layerType, rawDataLen), MPBase()
		{
			MPBase::reset(const_cast<pointer>(pRawData), rawDataLen, ownership);
		}
		
		/**
		 * @brief Default copy constructor.
		 * Internally calls copy constructors of base types.
		 * @param[in] other The instance to make copy of.
		 */
		GenericRawPacket(const GenericRawPacket& other) :
			Base(other), MPBase(other) {}

		/**
		 * @brief Default copy assignment operator.
		 * Internally calls copy assignment operators of base types.
		 * Correctly handles self assignment case.
		 * @param[in] other The instance to make copy of.
		 */
		GenericRawPacket& operator=(const GenericRawPacket& other)
		{
			if (this == &other)
				return *this;
			Base::operator=(other);
			MPBase::operator=(other);
			return *this;
		}

		/**
		 * @brief Default auto-generated move constructor.
		 * Internally calls move constructors of base types.
		 * Automatically switches between C++11 move semantics and its library implementation.
		 * In worst case works same as copy constructor.
		 * (the worst case is when memory proxy is only copyable or don't have any move constructors)
		 * @param[in] other The instance to move from.
		 */
		PCAPPP_MOVE_CONSTRUCTOR(GenericRawPacket) :
			Base(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER)),
			MPBase(PCAPPP_MOVE_WITH_CAST(MPBase&, PCAPPP_MOVE_OTHER))
		{}

		/**
		 * @brief Default auto-generated move assignment operator.
		 * Internally calls move assignment operators of base types.
		 * Automatically switches between C++11 move semantics and its library implementation.
		 * In worst case works same as copy assignment operator.
		 * (the worst case is when memory proxy is only copyable or don't have any move assignment operators)
		 * @param[in] other The instance to move from.
		 */
		PCAPPP_MOVE_ASSIGNMENT(GenericRawPacket)
		{
			if (this == &PCAPPP_MOVE_OTHER)
				return *this;
			Base::operator=(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER));
			MPBase::operator=(PCAPPP_MOVE_WITH_CAST(MPBase&, PCAPPP_MOVE_OTHER));
			return *this;
		}

		/**
		 * @brief Default destructor.
		 */
		~GenericRawPacket() {}

		/* Interface binding */

		/* Virtual API implementation */

		/**
		 * @brief Implements setRawData from RawPacket interface.
		 * Binds together effects of reset function from Memory Proxy interface
		 * and setRawData function from Raw Packet interface.
		 * @param[in] pRawData A pointer to the new raw data
		 * @param[in] rawDataLen The new raw data length in bytes
		 * @param[in] timestamp The time-stamp packet was received by the NIC
		 * @param[in] layerType The link layer type for this raw data
		 * @param[in] frameLength When reading from pcap files, sometimes the captured length is different from the actual packet length. This parameter represents the packet 
		 * length. This parameter is optional, if not set or set to -1 it is assumed both lengths are equal
		 * @return true if raw data was set successfully, false otherwise.
		 */
		bool setRawData(const_pointer pRawData, size rawDataLen, timeval timestamp, LinkLayerType layerType = LINKTYPE_ETHERNET, length frameLength = -1) PCAPPP_OVERRIDE
		{
			bool result = MPBase::reset(const_cast<pointer>(pRawData), rawDataLen, MPBase::isOwning());
			return result && Base::setRawData(pRawData, rawDataLen, timestamp, layerType, frameLength);
		}

		/**
		 * @brief Method to check if current object is in the null-state.
		 * @return true if object is NOT in the null-state, false otherwise.
		 */
		inline operator bool() { return Base::operator bool() || MPBase::operator bool(); }

		/* Abstract API implementation */

		/**
		 * @brief Special method that allocates new object on a heap.
		 * Object is constructed using default constructor.
		 * @return Pointer to the allocated object or PCAPPP_NULLPTR if allocation failed.
		 * @todo Add logger and error message x2
		 */
		inline RawPacket* newObject() PCAPPP_OVERRIDE
		{
			RawPacket* newObj = PCAPPP_NULLPTR;
			try {
				newObj = new GenericRawPacket;
			}
			catch (const std::exception&)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			catch (...)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			return newObj;
		}

		/**
		 * @brief Special method that allocates new object on a heap.
		 * Object is constructed using copy constructor to which this object is passed as other instance.
		 * @return Pointer to the allocated object or PCAPPP_NULLPTR if allocation failed.
		 * @todo Add logger and error message x2
		 */
		inline RawPacket* copy() PCAPPP_OVERRIDE
		{
			RawPacket* newObj = PCAPPP_NULLPTR;
			try {
				newObj = new GenericRawPacket(*this);
			}
			catch (const std::exception&)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			catch (...)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			return newObj;
		}

		/**
		 * @brief Special method that allocates new object on a heap.
		 * Object is constructed using move constructor to which this object is passed as other instance.
		 * @return Pointer to the allocated object or PCAPPP_NULLPTR if allocation failed.
		 * @todo Add logger and error message x2
		 */
		inline RawPacket* move() PCAPPP_OVERRIDE
		{
			RawPacket* newObj = PCAPPP_NULLPTR;
			try {
				newObj = new GenericRawPacket(PCAPPP_MOVE(*this));
			}
			catch (const std::exception&)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			catch (...)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			return newObj;
		}

		/**
		 * @brief Method to get raw data pointer.
		 * This overload is called if object is not const-qualified.
		 * Binds MemoryProxy::get to RawPacket::getRawData.
		 * @return A pointer to the raw data.
		 */
		inline pointer getRawData() PCAPPP_OVERRIDE { return MPBase::get(); }

		/**
		 * @brief Method to get raw data pointer.
		 * This overload is called if object is const-qualified.
		 * Binds MemoryProxy::get to RawPacket::getRawData.
		 * @return A pointer to the const-qualified raw data.
		 */
		inline const_pointer getRawData() const PCAPPP_OVERRIDE { return MPBase::get(); }

		/**
		 * @brief Method to get raw data pointer.
		 * Same as call to getRawData member function of const object.
		 * Binds MemoryProxy::get to RawPacket::getRawDataReadOnly.
		 * @return A pointer to the const-qualified raw data.
		 */
		inline const_pointer getRawDataReadOnly() const PCAPPP_OVERRIDE { return MPBase::get(); }

		/**
		 * @brief Method to get raw data length in bytes.
		 * Binds MemoryProxy::getLength to RawPacket::getRawDataLen.
		 * @return Raw data length in bytes.
		 */
		inline length getRawDataLen() const PCAPPP_OVERRIDE { return MPBase::getLength(); }

		/**
		 * @brief Method to get an indication whether raw data was already set for this instance.
		 * Binds MemoryProxy::operator bool to RawPacket::isPacketSet.
		 * @return True if raw data was set for this instance, false otherwise.
		 */
		inline bool isPacketSet() const PCAPPP_OVERRIDE { return MPBase::operator bool(); }

		/**
		 * @brief Returns owning status of underlying data.
		 * Binds MemoryProxy::isOwning to RawPacket::isOwning.
		 * @return true if object owns it's underlying data, false otherwise.
		 */
		inline bool isOwning() const PCAPPP_OVERRIDE { return MPBase::isOwning(); }

		/**
		 * @brief Represents the facility to release the ownership of underlying data.
		 * Binds MemoryProxy::release to RawPacket::releseData.
		 * @return Pointer to beginning of underlying data.
		 */
		inline pointer releseData() PCAPPP_OVERRIDE { return MPBase::release(); }

		/**
		 * @brief Reallocates underlying data.
		 * Binds MemoryProxy::reallocate to RawPacket::reallocateData.
		 * @todo RawPacket::m_FrameLength member may be invalidated here.
		 * @param[in] newBufferLength New size of data.
		 * @param[in] initialValue Per-byte initial value of new memory on allocation.
		 * @return true if operation ended successfully, false otherwise (you may expect that object is in null-state).
		 */
		inline bool reallocateData(size newBufferLength, memory_value initialValue = 0) PCAPPP_OVERRIDE { return MPBase::reallocate(newBufferLength, initialValue); }
		
		/**
		 * @brief Clear underlying data and set object to a null-state.
		 * Binds MemoryProxy::clear to RawPacket::clear.
		 * Internally calls RawPacket::initialize.
		 * @return true if operation ended successfully, false otherwise.
		 */
		inline bool clear() PCAPPP_OVERRIDE 
		{
			Base::initialize();
			return MPBase::clear();
		}

		/**
		 * @brief Append memory capable of holding dataToAppendLen data entries and set it per-byte to initialValue on allocation.
		 * Appending 0 bytes is always a success.
		 * Binds MemoryProxy::append to RawPacket::appendData.
		 * @todo RawPacket::m_FrameLength member may be invalidated here.
		 * @param[in] dataToAppendLen Size of data to be appended.
		 * @param[in] initialValue Initial value for new memory.
		 * @return true if operation ended successfully, false otherwise.
		 */
		inline bool appendData(size dataToAppendLen, memory_value initialValue = 0) PCAPPP_OVERRIDE { return MPBase::append(dataToAppendLen, initialValue); }
		
		/**
		 * @brief Append memory capable of holding dataToAppendLen data entries and copy data from dataToAppend to it (concatenate).
		 * Appending 0 bytes is always a success.
		 * Binds MemoryProxy::append to RawPacket::appendData.
		 * @todo RawPacket::m_FrameLength member may be invalidated here.
		 * @param[in] dataToAppend Buffer memory to be appended to current data.
		 * @param[in] dataToAppendLen Size of data to be appended.
		 * @return true if operation ended successfully, false otherwise.
		 */
		inline bool appendData(const_pointer dataToAppend, size dataToAppendLen) PCAPPP_OVERRIDE { return MPBase::append(dataToAppend, dataToAppendLen); }
		
		/**
		 * @brief Inserts memory capable of holding dataToInsertLen data entries and set it per-byte to initialValue on allocation.
		 * Binds MemoryProxy::insert to RawPacket::insertData.
		 * Depending on Memory Proxy type may handle case with negative atIndex.\n
		 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
		 * @todo RawPacket::m_FrameLength member may be invalidated here.
		 * @param[in] atIndex Index before which insertion take place.
		 * @param[in] dataToInsertLen Size of data to be inserted.
		 * @param[in] initialValue Initial value for new memory.
		 * @return true if operation finished successfully, false otherwise.
		 */
		inline bool insertData(index atIndex, size dataToInsertLen, memory_value initialValue = 0) PCAPPP_OVERRIDE { return MPBase::insert(atIndex, dataToInsertLen, initialValue); }
		
		/**
		 * @brief Inserts memory capable of holding dataToInsertLen data entries and copy data from dataToAppend to it.
		 * Binds MemoryProxy::insert to RawPacket::insertData.
		 * Depending on Memory Proxy type may handle case with negative atIndex.\n
		 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
		 * @todo RawPacket::m_FrameLength member may be invalidated here.
		 * @param[in] atIndex Index before which insertion take place.
		 * @param[in] dataToInsert Buffer memory to be inserted to current data.
		 * @param[in] dataToInsertLen Size of data to be inserted.
		 * @return true if operation finished successfully, false otherwise.
		 */
		inline bool insertData(index atIndex, const_pointer dataToInsert, size dataToInsertLen) PCAPPP_OVERRIDE { return MPBase::insert(atIndex, dataToInsert, dataToInsertLen); }
		
		/**
		 * @brief Removes memory capable of holding numOfBytesToRemove data entries starting from atIndex.
		 * Binds MemoryProxy::remove to RawPacket::removeData.
		 * Depending on Memory Proxy type may handle case with negative atIndex is handled correctly.\n
		 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
		 * @todo RawPacket::m_FrameLength member may be invalidated here.
		 * @param[in] atIndex Index from which removal take place.
		 * @param[in] numOfBytesToRemove Size of data to be removed.
		 * @return true if operation finished successfully, false otherwise.
		 */
		inline bool removeData(index atIndex, size numOfBytesToRemove) PCAPPP_OVERRIDE { return MPBase::remove(atIndex, numOfBytesToRemove); }
	};

	/**
	 * @brief Specialization for DefaultOldMemoryModelMemoryProxy.
	 */
	template <>
	class GenericRawPacket< ::pcpp::memory::MemoryProxyTags::OldMemoryModelTag, ::pcpp::memory::DefaultOldMemoryModelMemoryProxy, void > PCAPPP_FINAL :
		public RawPacket,
		protected ::pcpp::memory::DefaultOldMemoryModelMemoryProxy
	{
	public:

		/* MemoryProxy typedefs */

		/**
		 * Convenient alias typedef for base memory proxy class. 
		 */
		typedef ::pcpp::memory::DefaultOldMemoryModelMemoryProxy MPBase;
		/**
		 * Defines type of tag used to deduce memory proxy type.
		 */
		typedef ::pcpp::memory::MemoryProxyTags::OldMemoryModelTag tag_t;

		/* RawPacket typedefs */

		/**
		 * Convenient alias typedef for base raw packet class. 
		 */
		typedef RawPacket Base;
		/**
		 * Defines underlying memory type.
		 */
		typedef Base::value_type value_type;
		/**
		 * Defines type of pointer to underlying memory type.
		 */
		typedef Base::pointer pointer;
		/**
		 * Defines type of constant pointer to underlying memory type.
		 */
		typedef Base::const_pointer const_pointer;
		/**
		 * Defines type of reference to element of underlying memory type.
		 */
		typedef Base::reference reference;
		/**
		 * Defines type of constant reference to element of underlying memory type.
		 */
		typedef Base::const_reference const_reference;

		/**
		 * Defines type of variables that represents size values.
		 */
		typedef Base::size size;
		/**
		 * Defines type of variables that represents length values.
		 */
		typedef Base::length length;
		/**
		 * Defines type of variables that represents index values.
		 */
		typedef Base::index index;
		/**
		 * Defines type of variables that represents initial memory values for std::memset.
		 */
		typedef Base::memory_value memory_value;

		GenericRawPacket() : m_MaxLength(0) {}

		GenericRawPacket(const_pointer pRawData, size rawDataLen, timeval timestamp, bool ownership, LinkLayerType layerType = LINKTYPE_ETHERNET) :
			Base(timestamp, layerType, rawDataLen), MPBase()
		{
			m_MaxLength = rawDataLen;
			MPBase::reset(const_cast<pointer>(pRawData), rawDataLen, ownership);
		}
		
		GenericRawPacket(const GenericRawPacket& other) :
			Base(other), MPBase(other), m_MaxLength(other.m_MaxLength) {}

		GenericRawPacket& operator=(const GenericRawPacket& other)
		{
			if (this == &other)
				return *this;
			Base::operator=(other);
			MPBase::operator=(other);
			m_MaxLength = other.m_MaxLength;
			return *this;
		}

		PCAPPP_MOVE_CONSTRUCTOR(GenericRawPacket) :
			Base(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER)),
			MPBase(PCAPPP_MOVE_WITH_CAST(MPBase&, PCAPPP_MOVE_OTHER)),
			m_MaxLength(PCAPPP_MOVE_OTHER.m_MaxLength)
		{}

		PCAPPP_MOVE_ASSIGNMENT(GenericRawPacket)
		{
			if (this == &PCAPPP_MOVE_OTHER)
				return *this;
			Base::operator=(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER));
			MPBase::operator=(PCAPPP_MOVE_WITH_CAST(MPBase&, PCAPPP_MOVE_OTHER));
			m_MaxLength = PCAPPP_MOVE_OTHER.m_MaxLength;
			return *this;
		}

		~GenericRawPacket() {}

		/* Interface binding */

		/* Virtual API implementation */

		bool setRawData(const_pointer pRawData, size rawDataLen, timeval timestamp, LinkLayerType layerType = LINKTYPE_ETHERNET, length frameLength = -1) PCAPPP_OVERRIDE
		{
			m_MaxLength = rawDataLen;
			bool result = MPBase::reset(const_cast<pointer>(pRawData), rawDataLen, MPBase::isOwning());
			return result && Base::setRawData(pRawData, rawDataLen, timestamp, layerType, frameLength);
		}

		inline operator bool() { return Base::operator bool() || MPBase::operator bool(); }

		/* Abstract API implementation */

		inline RawPacket* newObject() PCAPPP_OVERRIDE
		{
			RawPacket* newObj = PCAPPP_NULLPTR;
			try {
				newObj = new GenericRawPacket;
			}
			catch (const std::exception&)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			catch (...)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			return newObj;
		}

		inline RawPacket* copy() PCAPPP_OVERRIDE
		{
			RawPacket* newObj = PCAPPP_NULLPTR;
			try {
				newObj = new GenericRawPacket(*this);
			}
			catch (const std::exception&)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			catch (...)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			return newObj;
		}

		inline RawPacket* move() PCAPPP_OVERRIDE
		{
			RawPacket* newObj = PCAPPP_NULLPTR;
			try {
				newObj = new GenericRawPacket(PCAPPP_MOVE(*this));
			}
			catch (const std::exception&)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			catch (...)
			{
				// TODO: Add logger and error message
				return PCAPPP_NULLPTR;
			}
			return newObj;
		}

		inline pointer getRawData() PCAPPP_OVERRIDE { return MPBase::get(); }

		inline const_pointer getRawData() const PCAPPP_OVERRIDE { return MPBase::get(); }

		inline const_pointer getRawDataReadOnly() const PCAPPP_OVERRIDE { return MPBase::get(); }

		inline length getRawDataLen() const PCAPPP_OVERRIDE { return MPBase::getLength(); }

		inline bool isPacketSet() const PCAPPP_OVERRIDE { return MPBase::operator bool(); }

		inline bool isOwning() const PCAPPP_OVERRIDE { return MPBase::isOwning(); }

		inline pointer releseData() PCAPPP_OVERRIDE { return MPBase::release(); }

		inline bool reallocateData(size newBufferLength, memory_value initialValue = 0) PCAPPP_OVERRIDE 
		{
			m_MaxLength = newBufferLength;
			return MPBase::reallocate(newBufferLength, initialValue); 
		}
		
		inline bool clear() PCAPPP_OVERRIDE 
		{
			m_MaxLength = 0;
			Base::initialize();
			return MPBase::clear();
		}

		inline bool appendData(size dataToAppendLen, memory_value initialValue = 0) PCAPPP_OVERRIDE 
		{
			realloc_impl(dataToAppendLen);
			return MPBase::append(dataToAppendLen, initialValue); 
		}
		
		inline bool appendData(const_pointer dataToAppend, size dataToAppendLen) PCAPPP_OVERRIDE 
		{
			realloc_impl(dataToAppendLen);
			return MPBase::append(dataToAppend, dataToAppendLen);
		}
		
		inline bool insertData(index atIndex, size dataToInsertLen, memory_value initialValue = 0) PCAPPP_OVERRIDE 
		{ 
			realloc_impl(dataToInsertLen);
			return MPBase::insert(atIndex, dataToInsertLen, initialValue);
		}
		
		inline bool insertData(index atIndex, const_pointer dataToInsert, size dataToInsertLen) PCAPPP_OVERRIDE
		{
			realloc_impl(dataToInsertLen);
			return MPBase::insert(atIndex, dataToInsert, dataToInsertLen);
		}
		
		inline bool removeData(index atIndex, size numOfBytesToRemove) PCAPPP_OVERRIDE { return MPBase::remove(atIndex, numOfBytesToRemove); }
		
	private:

		inline void realloc_impl(size new_size)
		{
			if (!Base::isPacketSet()) {
				reallocateData(new_size > m_MaxLength ? new_size : m_MaxLength );
			} else if ( MPBase::getLength() + new_size > m_MaxLength ) {
				// reallocate to maximum value of: twice the max size of the packet or max size + new required length
				if ( MPBase::getLength() + new_size > m_MaxLength * 2 ) {
					reallocateData(MPBase::getLength() + new_size + m_MaxLength);
				} else {
					reallocateData(m_MaxLength * 2);
				}
			}
		}

		size m_MaxLength; //<! Was previously a data member of Packet class called m_MaxPacketLen.
	};

	/**
	 * @brief Represents a type of default library-wide RawPacket implementation.
	 */
	typedef GenericRawPacket<> DefaultRawPacket;

} // namespace pcpp

#endif
