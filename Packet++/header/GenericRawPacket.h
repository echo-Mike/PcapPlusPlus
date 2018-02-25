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

	template <
		typename MemoryProxyTag = ::pcpp::memory::MemoryProxyTags::OldMemoryModelTag,
		typename MemoryProxyT = typename ::pcpp::memory::MemoryProxyDispatcher< MemoryProxyTag >::memory_proxy_t,
		typename = typename ::pcpp::type_traits::enable_if<
			::pcpp::type_traits::is_base_of<
				::pcpp::memory::MemoryProxyInterface< ::pcpp::memory::Data_t >,
				MemoryProxyT
			>::value,
			void 
		>::type
	>
		class GenericRawPacket PCAPPP_FINAL :
		public RawPacket,
		protected MemoryProxyT
	{
	public:

		/* MemoryProxy typedefs */

		typedef MemoryProxyT MPBase;
		
		typedef MemoryProxyTag tag_t;

		/* RawPacket typedefs */

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

	protected:

		inline void initialize()
		{
			Base::initialize();
			MPBase::initialize();
		}

	public:

		GenericRawPacket() { initialize(); }

		GenericRawPacket(const_pointer pRawData, size rawDataLen, timeval timestamp, bool ownership, LinkLayerType layerType = LINKTYPE_ETHERNET) :
			Base(timestamp, layerType, rawDataLen), MPBase()
		{
			MPBase::reset(pRawData, rawDataLen, ownership);
		}

		GenericRawPacket(const GenericRawPacket& other) :
			Base(other), MPBase(other) {}

		GenericRawPacket& operator=(const GenericRawPacket& other)
		{
			initialize();
			Base::operator=(other);
			MPBase::operator=(other);
		}

		PCAPPP_MOVE_CONSTRUCTOR(GenericRawPacket) :
			Base(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER)),
			MPBase(PCAPPP_MOVE_WITH_CAST(MPBase&, PCAPPP_MOVE_OTHER))
		{}

		PCAPPP_MOVE_ASSIGNMENT(GenericRawPacket)
		{
			initialize();
			Base::operator=(PCAPPP_MOVE_WITH_CAST(Base&, PCAPPP_MOVE_OTHER));
			MPBase::operator=(PCAPPP_MOVE_WITH_CAST(MPBase&, PCAPPP_MOVE_OTHER));
		}

		~GenericRawPacket() {}

		bool setRawData(const_pointer pRawData, size rawDataLen, timeval timestamp, LinkLayerType layerType = LINKTYPE_ETHERNET, length frameLength = -1) PCAPPP_OVERRIDE
		{
			MPBase::reset(pRawData, rawDataLen, MPBase::isOwning());
			Base::setRawData(pRawData, rawDataLen, timestamp, layerType, frameLength);
		}

		inline operator bool() { return Base::operator bool() || MPBase::operator bool(); }

		inline pointer getRawData() PCAPPP_OVERRIDE { return MPBase::get(); }

		inline const_pointer getRawData() const PCAPPP_OVERRIDE { return MPBase::get(); }

		inline const_pointer getRawDataReadOnly() const PCAPPP_OVERRIDE { return MPBase::get(); }

		inline length getRawDataLen() const PCAPPP_OVERRIDE { return MPBase::getLength(); }

		inline bool isPacketSet() const PCAPPP_OVERRIDE { return MPBase::operator bool(); }

		inline bool isOwning() const PCAPPP_OVERRIDE { return MPBase::isOwning(); }

		inline pointer releseData() PCAPPP_OVERRIDE { return MPBase::release(); }

		inline bool reallocateData(size newBufferLength, memory_value initialValue = 0) PCAPPP_OVERRIDE { return MPBase::realocate(newBufferLength, initialValue); }

		inline bool clear() PCAPPP_OVERRIDE { return MPBase::clear(); }

		inline bool appendData(size dataToAppendLen, memory_value initialValue = 0) PCAPPP_OVERRIDE { return MPBase::append(dataToAppendLen, initialValue); }

		inline bool appendData(const_pointer dataToAppend, size dataToAppendLen) PCAPPP_OVERRIDE { return MPBase::append(dataToAppend, dataToAppendLen); }

		inline bool insertData(index atIndex, size dataToInsertLen, memory_value initialValue = 0) PCAPPP_OVERRIDE { return MPBase::insert(atIndex, dataToInsertLen, initialValue); }
		
		inline bool insertData(index atIndex, const_pointer dataToInsert, size dataToInsertLen) PCAPPP_OVERRIDE { return MPBase::insert(atIndex, dataToInsert, dataToInsertLen); }

		inline bool removeData(index atIndex, size numOfBytesToRemove) PCAPPP_OVERRIDE { return MPBase::remove(atIndex, numOfBytesToRemove); }
	};

} // namespace pcpp

#endif
