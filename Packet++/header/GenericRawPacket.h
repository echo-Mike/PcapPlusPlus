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
		typename MemoryProxyT::Base& (MemoryProxyT::* expose)() = &MemoryProxyT::expose,
		typename = typename ::pcpp::type_traits::enable_if< 
			::pcpp::type_traits::is_same<
				::pcpp::memory::MemoryProxyInterface< ::pcpp::memory::Data_t >, 
				typename MemoryProxyT::Base >::value, 
			void >::type
	>
	class GenericRawPacket PCAPPP_FINAL :
		public RawPacket
	{
	public:

		/* MemoryProxy typedefs */

		typedef MemoryProxyT memory_proxy_t;
		
		typedef ::pcpp::memory::MemoryProxyInterface< ::pcpp::memory::Data_t >& MPInterface_t;
		
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
		/**
		 * @brief Replacement for expression: dynamic_cast<MPInterface_t>(*&memory_proxy).
		 * We already knew that memory_proxy base class must be ::pcpp::memory::MemoryProxyInterface< ::pcpp::memory::Data_t >
		 * so no dynamic checks are needed.
		 * @return Polymorphic reference to memory_proxy.
		 */
		inline MPInterface_t MPIterface() { return (memory_proxy.*expose)(); }

		inline void initialize()
		{
			Base::initialize();
			MPIterface().clear();
		}

	public:

		GenericRawPacket() { initialize(); }

		GenericRawPacket(const_pointer pRawData, size rawDataLen, timeval timestamp, bool ownership, LinkLayerType layerType = LINKTYPE_ETHERNET) :
			Base(timestamp, layerType, rawDataLen), memory_proxy()
		{
			MPIterface().reset(pRawData, rawDataLen, ownership);
		}

		GenericRawPacket(const GenericRawPacket& other) :
			Base(other), memory_proxy(other.memory_proxy) {}

		GenericRawPacket& operator=(const GenericRawPacket& other)
		{
			initialize();
			memory_proxy = other.memory_proxy;
			Base::operator=(other);
		}

		PCAPPP_MOVE_CONSTRUCTOR(GenericRawPacket) :
			Base(PCAPPP_MOVE_OTHER), 
			memory_proxy(PCAPPP_MOVE(PCAPPP_MOVE_OTHER.memory_proxy))
		{}

		PCAPPP_MOVE_ASSIGNMENT(GenericRawPacket)
		{
			initialize();
			memory_proxy = PCAPPP_MOVE(PCAPPP_MOVE_OTHER.memory_proxy);
			Base::operator=(PCAPPP_MOVE_OTHER);
		}

		~GenericRawPacket() {}

		bool setRawData(const_pointer pRawData, size rawDataLen, timeval timestamp, LinkLayerType layerType = LINKTYPE_ETHERNET, length frameLength = -1) PCAPPP_OVERRIDE
		{
			MPIterface().reset(pRawData, rawDataLen, MPIterface().isOwning());
			Base::setRawData(pRawData, rawDataLen, timestamp, layerType, frameLength);
		}

		inline operator bool() { return Base::operator bool() || MPIterface().operator bool(); }

		inline pointer getRawData() PCAPPP_OVERRIDE { return MPIterface().get(); }

		inline const_pointer getRawData() const PCAPPP_OVERRIDE { return MPIterface().get(); }

		inline const_pointer getRawDataReadOnly() const PCAPPP_OVERRIDE { return MPIterface().get(); }

		inline length getRawDataLen() const PCAPPP_OVERRIDE { return MPIterface().getLength(); }

		inline bool isPacketSet() const PCAPPP_OVERRIDE { return MPIterface().operator bool(); }

		inline bool isOwning() const PCAPPP_OVERRIDE { return MPIterface().isOwning(); }

		pointer releseData() PCAPPP_OVERRIDE { return MPIterface().release(); }

		bool reallocateData(size newBufferLength, memory_value initialValue = 0) PCAPPP_OVERRIDE { MPIterface().realocate(newBufferLength, initialValue); }

		bool clear() PCAPPP_OVERRIDE { MPIterface().clear(); }

		void appendData(size dataToAppendLen, memory_value initialValue = 0) PCAPPP_OVERRIDE { MPIterface().append(dataToAppendLen, initialValue); }

		void appendData(const_pointer dataToAppend, size dataToAppendLen) PCAPPP_OVERRIDE { MPIterface().append(dataToAppend, dataToAppendLen); }

		void insertData(index atIndex, size dataToInsertLen, memory_value initialValue = 0) PCAPPP_OVERRIDE { MPIterface().insert(atIndex, dataToInsertLen, initialValue); }
		
		void insertData(index atIndex, const_pointer dataToInsert, size dataToInsertLen) PCAPPP_OVERRIDE { MPIterface().insert(atIndex, dataToInsert, dataToInsertLen); }

		bool removeData(index atIndex, size numOfBytesToRemove) PCAPPP_OVERRIDE { MPIterface().remove(atIndex, numOfBytesToRemove); }

	protected:
		memory_proxy_t memory_proxy;
	};

} // namespace pcpp

#endif
