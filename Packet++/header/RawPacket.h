#ifndef PCAPPP_RAW_PACKET
#define PCAPPP_RAW_PACKET

#include <stdint.h>
#ifdef _MSC_VER
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif
#include <stddef.h>

#include "MemoryProxy.h"

/// @file

/**
 * \namespace pcpp
 * \brief The main namespace for the PcapPlusPlus lib
 */
namespace pcpp
{

	/**
	 * An enum describing all known link layer type. Taken from: http://www.tcpdump.org/linktypes.html .
	 * Currently only Ethernet (1) and SLL (113) are supported
	 */
	enum LinkLayerType
	{
		/** BSD loopback encapsulation */
		LINKTYPE_NULL = 0,
		/** IEEE 802.3 Ethernet */
		LINKTYPE_ETHERNET = 1,
		/** AX.25 packet */
		LINKTYPE_AX25 = 3,
		/** IEEE 802.5 Token Ring */
		LINKTYPE_IEEE802_5 = 6,
		/** ARCNET Data Packets */
		LINKTYPE_ARCNET_BSD = 7,
		/** SLIP, encapsulated with a LINKTYPE_SLIP header */
		LINKTYPE_SLIP = 8,
		/** PPP, as per RFC 1661 and RFC 1662 */
		LINKTYPE_PPP = 9,
		/** FDDI, as specified by ANSI INCITS 239-1994 */
		LINKTYPE_FDDI = 10,
		/** Raw IP */
		LINKTYPE_DLT_RAW1 = 12,
		/** Raw IP (OpenBSD) */
		LINKTYPE_DLT_RAW2 = 14,
		/** PPP in HDLC-like framing, as per RFC 1662, or Cisco PPP with HDLC framing, as per section 4.3.1 of RFC 1547 */
		LINKTYPE_PPP_HDLC = 50,
		/** PPPoE */
		LINKTYPE_PPP_ETHER = 51,
		/** RFC 1483 LLC/SNAP-encapsulated ATM */
		LINKTYPE_ATM_RFC1483 = 100,
		/** Raw IP */
		LINKTYPE_RAW = 101,
		/** Cisco PPP with HDLC framing */
		LINKTYPE_C_HDLC = 104,
		/** IEEE 802.11 wireless LAN */
		LINKTYPE_IEEE802_11 = 105,
		/** Frame Relay */
		LINKTYPE_FRELAY = 107,
		/** OpenBSD loopback encapsulation */
		LINKTYPE_LOOP = 108,
		/** Linux "cooked" capture encapsulation */
		LINKTYPE_LINUX_SLL = 113,
		/** Apple LocalTalk */
		LINKTYPE_LTALK = 114,
		/** OpenBSD pflog */
		LINKTYPE_PFLOG = 117,
		/** Prism monitor mode information followed by an 802.11 header */
		LINKTYPE_IEEE802_11_PRISM = 119,
		/** RFC 2625 IP-over-Fibre Channel */
		LINKTYPE_IP_OVER_FC = 122,
		/** ATM traffic, encapsulated as per the scheme used by SunATM devices */
		LINKTYPE_SUNATM = 123,
		/** Radiotap link-layer information followed by an 802.11 header */
		LINKTYPE_IEEE802_11_RADIOTAP = 127,
		/** ARCNET Data Packets, as described by the ARCNET Trade Association standard ATA 878.1-1999 */
		LINKTYPE_ARCNET_LINUX = 129,
		/** Apple IP-over-IEEE 1394 cooked header */
		LINKTYPE_APPLE_IP_OVER_IEEE1394 = 138,
		/** Signaling System 7 Message Transfer Part Level 2 */
		LINKTYPE_MTP2_WITH_PHDR = 139,
		/** Signaling System 7 Message Transfer Part Level 2 */
		LINKTYPE_MTP2 = 140,
		/** Signaling System 7 Message Transfer Part Level 3 */
		LINKTYPE_MTP3 = 141,
		/** Signaling System 7 Signalling Connection Control Part */
		LINKTYPE_SCCP = 142,
		/** Signaling System 7 Signalling Connection Control Part */
		LINKTYPE_DOCSIS = 143,
		/** Linux-IrDA packets */
		LINKTYPE_LINUX_IRDA = 144,
		/** AVS monitor mode information followed by an 802.11 header */
		LINKTYPE_IEEE802_11_AVS = 163,
		/** BACnet MS/TP frames */
		LINKTYPE_BACNET_MS_TP = 165,
		/** PPP in HDLC-like encapsulation, like LINKTYPE_PPP_HDLC, but with the 0xff address byte replaced by a direction indication - 0x00 for incoming and 0x01 for outgoing */
		LINKTYPE_PPP_PPPD = 166,
		/** General Packet Radio Service Logical Link Control */
		LINKTYPE_GPRS_LLC = 169,
		/** Transparent-mapped generic framing procedure */
		LINKTYPE_GPF_T = 170,
		/** Frame-mapped generic framing procedure */
		LINKTYPE_GPF_F = 171,
		/** Link Access Procedures on the D Channel (LAPD) frames */
		LINKTYPE_LINUX_LAPD = 177,
		/** Bluetooth HCI UART transport layer */
		LINKTYPE_BLUETOOTH_HCI_H4 = 187,
		/** USB packets, beginning with a Linux USB header */
		LINKTYPE_USB_LINUX = 189,
		/** Per-Packet Information information */
		LINKTYPE_PPI = 192,
		/** IEEE 802.15.4 wireless Personal Area Network */
		LINKTYPE_IEEE802_15_4 = 195,
		/** Various link-layer types, with a pseudo-header, for SITA */
		LINKTYPE_SITA = 196,
		/** Various link-layer types, with a pseudo-header, for Endace DAG cards; encapsulates Endace ERF record */
		LINKTYPE_ERF = 197,
		/** Bluetooth HCI UART transport layer */
		LINKTYPE_BLUETOOTH_HCI_H4_WITH_PHDR = 201,
		/** AX.25 packet, with a 1-byte KISS header containing a type indicator */
		LINKTYPE_AX25_KISS = 202,
		/** Link Access Procedures on the D Channel (LAPD) frames */
		LINKTYPE_LAPD = 203,
		/** PPP, as per RFC 1661 and RFC 1662, preceded with a one-byte pseudo-header with a zero value meaning "received by this host" and a non-zero value meaning  "sent by this host" */
		LINKTYPE_PPP_WITH_DIR = 204,
		/** Cisco PPP with HDLC framing */
		LINKTYPE_C_HDLC_WITH_DIR = 205,
		/** Frame Relay */
		LINKTYPE_FRELAY_WITH_DIR = 206,
		/** IPMB over an I2C circuit */
		LINKTYPE_IPMB_LINUX = 209,
		/** IEEE 802.15.4 wireless Personal Area Network */
		LINKTYPE_IEEE802_15_4_NONASK_PHY = 215,
		/** USB packets, beginning with a Linux USB header */
		LINKTYPE_USB_LINUX_MMAPPED = 220,
		/** Fibre Channel FC-2 frames, beginning with a Frame_Header */
		LINKTYPE_FC_2 = 224,
		/** Fibre Channel FC-2 frames */
		LINKTYPE_FC_2_WITH_FRAME_DELIMS = 225,
		/** Solaris ipnet pseudo-header */
		LINKTYPE_IPNET = 226,
		/** CAN (Controller Area Network) frames, with a pseudo-header as supplied by Linux SocketCAN */
		LINKTYPE_CAN_SOCKETCAN = 227,
		/** Raw IPv4; the packet begins with an IPv4 header */
		LINKTYPE_IPV4 = 228,
		/** Raw IPv6; the packet begins with an IPv6 header */
		LINKTYPE_IPV6 = 229,
		/** IEEE 802.15.4 wireless Personal Area Network, without the FCS at the end of the frame */
		LINKTYPE_IEEE802_15_4_NOFCS = 230,
		/** Raw D-Bus messages, starting with the endianness flag, followed by the message type, etc., but without the authentication handshake before the message sequence */
		LINKTYPE_DBUS = 231,
		/** DVB-CI (DVB Common Interface for communication between a PC Card module and a DVB receiver), with the message format specified by the PCAP format for DVB-CI specification */
		LINKTYPE_DVB_CI = 235,
		/** Variant of 3GPP TS 27.010 multiplexing protocol (similar to, but not the same as, 27.010) */
		LINKTYPE_MUX27010 = 236,
		/** D_PDUs as described by NATO standard STANAG 5066, starting with the synchronization sequence, and including both header and data CRCs */
		LINKTYPE_STANAG_5066_D_PDU = 237,
		/** Linux netlink NETLINK NFLOG socket log messages */
		LINKTYPE_NFLOG = 239,
		/** Pseudo-header for Hilscher Gesellschaft für Systemautomation mbH netANALYZER devices, followed by an Ethernet frame, beginning with the MAC header and ending with the FCS */
		LINKTYPE_NETANALYZER = 240,
		/** Pseudo-header for Hilscher Gesellschaft für Systemautomation mbH netANALYZER devices, followed by an Ethernet frame, beginning with the preamble, SFD, and MAC header, and ending with the FCS */
		LINKTYPE_NETANALYZER_TRANSPARENT = 241,
		/** IP-over-InfiniBand, as specified by RFC 4391 section 6 */
		LINKTYPE_IPOIB = 242,
		/** MPEG-2 Transport Stream transport packets, as specified by ISO 13818-1/ITU-T Recommendation H.222.0 */
		LINKTYPE_MPEG_2_TS = 243,
		/** Pseudo-header for ng4T GmbH's UMTS Iub/Iur-over-ATM and Iub/Iur-over-IP format as used by their ng40 protocol tester */
		LINKTYPE_NG40 = 244,
		/** Pseudo-header for NFC LLCP packet captures, followed by frame data for the LLCP Protocol as specified by NFCForum-TS-LLCP_1.1 */
		LINKTYPE_NFC_LLCP = 245,
		/** Raw InfiniBand frames, starting with the Local Routing Header */
		LINKTYPE_INFINIBAND = 247,
		/** SCTP packets, as defined by RFC 4960, with no lower-level protocols such as IPv4 or IPv6 */
		LINKTYPE_SCTP = 248,
		/** USB packets, beginning with a USBPcap header */
		LINKTYPE_USBPCAP = 249,
		/** Serial-line packet header for the Schweitzer Engineering Laboratories "RTAC" product */
		LINKTYPE_RTAC_SERIAL = 250,
		/** Bluetooth Low Energy air interface Link Layer packets */
		LINKTYPE_BLUETOOTH_LE_LL = 251,
		/** Linux Netlink capture encapsulation */
		LINKTYPE_NETLINK = 253,
		/** Bluetooth Linux Monitor encapsulation of traffic for the BlueZ stack */
		LINKTYPE_BLUETOOTH_LINUX_MONITOR = 254,
		/** Bluetooth Basic Rate and Enhanced Data Rate baseband packets */
		LINKTYPE_BLUETOOTH_BREDR_BB = 255,
		/** Bluetooth Low Energy link-layer packets */
		LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR = 256,
		/** PROFIBUS data link layer packets, as specified by IEC standard 61158-6-3 */
		LINKTYPE_PROFIBUS_DL = 257,
		/** Apple PKTAP capture encapsulation */
		LINKTYPE_PKTAP = 258,
		/** Ethernet-over-passive-optical-network packets */
		LINKTYPE_EPON = 259,
		/** IPMI trace packets, as specified by Table 3-20 "Trace Data Block Format" in the PICMG HPM.2 specification */
		LINKTYPE_IPMI_HPM_2 = 260,
		/** Per Joshua Wright <jwright@hasborg.com>, formats for Z-Wave RF profiles R1 and R2 captures */
		LINKTYPE_ZWAVE_R1_R2 = 261,
		/** Per Joshua Wright <jwright@hasborg.com>, formats for Z-Wave RF profile R3 captures */
		LINKTYPE_ZWAVE_R3 = 262,
		/** Formats for WattStopper Digital Lighting Management (DLM) and Legrand Nitoo Open protocol common packet structure captures */
		LINKTYPE_WATTSTOPPER_DLM = 263,
		/** Messages between ISO 14443 contactless smartcards (Proximity Integrated Circuit Card, PICC) and card readers (Proximity Coupling Device, PCD), with the message format specified by the PCAP format for ISO14443 specification */
		LINKTYPE_ISO_14443 = 264
	};

	/**
	 * Max packet size supported
	 */
	#define PCPP_MAX_PACKET_SIZE 65536

	/**
	 * @class RawPacket
	 * RawPacket is an abstract class that describes the interface which is used to work with raw (not parsed) packet data.
	 * Additionally holds a time-stamp representing the time the packet was received by the NIC.
	 * This class has the null-object state in which any object can be set to by call to clear member function.
	 */
	class RawPacket
	{
		typedef ::pcpp::memory::DefaultMemoryProxyInterface TypeHelper;
	public:
		/**
		 * Defines underlying memory type.
		 */
		typedef TypeHelper::value_type value_type;
		/**
		 * Defines type of pointer to underlying memory type.
		 */
		typedef TypeHelper::pointer pointer;
		/**
		 * Defines type of constant pointer to underlying memory type.
		 */
		typedef TypeHelper::const_pointer const_pointer;
		/**
		 * Defines type of reference to element of underlying memory type.
		 */
		typedef TypeHelper::reference reference;
		/**
		 * Defines type of constant reference to element of underlying memory type.
		 */
		typedef TypeHelper::const_reference const_reference;

		/**
		 * Defines type of variables that represents size values.
		 */
		typedef TypeHelper::size size;
		/**
		 * Defines type of variables that represents length values.
		 */
		typedef TypeHelper::length length;
		/**
		 * Defines type of variables that represents index values.
		 */
		typedef TypeHelper::index index;
		/**
		 * Defines type of variables that represents initial memory values for std::memset.
		 */
		typedef TypeHelper::memory_value memory_value;

		/**
		 * Represents type of structure that holds time-stamp.
		 */
		typedef timeval time_t;

	protected:

		/**
		 * @brief Setups RawPacket object to a null-state.
		 * Basically zeroes all fields.
		 * m_TimeStamp field isn't touched.
		 */
		inline void initialize()
		{
			// Null object state
			m_FrameLength = 0;
			m_linkLayerType = LINKTYPE_ETHERNET;
		}

	public:

		/* General functions */

		/**
		 * @brief Default constructor that initializes class'es attributes to default value.
		 * Internally calls initialize member function.
		 * @todo time-stamp isn't set here to a default value
		 */
		RawPacket() { initialize(); }

		/**
		 * @brief Basic constructor that initializes class'es attributes to provided values.
		 * @param[in] timestamp The time-stamp packet was received by the NIC.
		 * @param[in] layerType The link layer type of this raw packet. The default is Ethernet
		 * @param[in] frameLength Frame length parameter used by file devices.
		 */
		RawPacket(time_t timestamp, LinkLayerType layerType, length frameLength = -1) :
			m_TimeStamp(timestamp), m_FrameLength(frameLength), m_linkLayerType(layerType) {}

		/**
		 * @brief Default copy constructor.
		 * Copies data members from other instance.
		 * @param[in] other The instance to make copy of.
		 */
		RawPacket(const RawPacket& other) :
			m_TimeStamp(other.m_TimeStamp), m_FrameLength(other.m_FrameLength), m_linkLayerType(other.m_linkLayerType) {}

		/**
		 * @brief Default copy assignment operator.
		 * Copies data members from other instance.
		 * Correctly handles self assignment case.
		 * @param[in] other The instance to make copy of.
		 */
		RawPacket& operator=(const RawPacket& other);

		/* Class API */

		/**
		 * @brief Method to get the link layer type.
		 * @return The type of the link layer.
		 */
		inline LinkLayerType getLinkLayerType() const { return m_linkLayerType; }

		/**
		 * @brief Method to get frame length in bytes.
		 * @return Frame length in bytes.
		 */
		inline length getFrameLength() const { return m_FrameLength; }

		/**
		 * @brief Method to get raw data time-stamp.
		 * @return Raw data time-stamp.
		 */
		inline time_t getPacketTimeStamp() const { return m_TimeStamp; }

		/**
		 * @brief Method to reset raw data time-stamp.
		 * Works by swapping provided and current values.
		 * @return Previous raw data time-stamp.
		 */
		inline time_t& resetPacketTimeStamp(time_t& timestamp) { std::swap(m_TimeStamp, timestamp); return timestamp; }

#ifdef ENABLE_CPP11_MOVE_SEMANTICS
		/**
		 * @brief Method to reset raw data time-stamp.
		 * Compiler calls this overload if time-stamp directly returned from function call or explicitly moved
		 */
		inline void resetPacketTimeStamp(time_t&& timestamp) { m_TimeStamp = timestamp; }
#endif
		/* Virtual API */

		/**
		 * @brief Virtual destructor of this class.
		 */
		virtual ~RawPacket() {}

		/**
		 * @brief Sets provided raw data as new data to handle. 
		 * @param[in] pRawData A pointer to the new raw data
		 * @param[in] rawDataLen The new raw data length in bytes
		 * @param[in] timestamp The time-stamp packet was received by the NIC
		 * @param[in] layerType The link layer type for this raw data
		 * @param[in] frameLength When reading from pcap files, sometimes the captured length is different from the actual packet length. This parameter represents the packet 
		 * length. This parameter is optional, if not set or set to -1 it is assumed both lengths are equal
		 * @return true if raw data was set successfully, false otherwise.
		 */
		virtual bool setRawData(const_pointer /* pRawData */, size rawDataLen, time_t timestamp, LinkLayerType layerType = LINKTYPE_ETHERNET, length frameLength = -1);

		/**
		 * @brief Method to check if current object is in the null-state.
		 * @return true if object is NOT in the null-state, false otherwise.
		 */
		virtual operator bool() const { return m_FrameLength || m_linkLayerType != LINKTYPE_ETHERNET; }

		/**
		 * @brief Method to check if current object is in the null-state.
		 * Basically the negation of operator bool call.
		 * @return true if object is in the null-state, false otherwise.
		 */
		inline bool isInNullState() const { return !(this->operator bool()); }

		/* Abstract API: Object manipulation */

		/**
		 * @brief Special method that allocates new object of underlying type on a heap.
		 * Object is constructed using default constructor.
		 * @return Pointer to the allocated object or PCAPPP_NULLPTR if allocation failed.
		 */
		virtual RawPacket* newObject() = 0;

		/**
		 * @brief Special method that allocates new object of underlying type on a heap.
		 * Object is constructed using copy constructor to which this object is passed as other instance.
		 * @return Pointer to the allocated object or PCAPPP_NULLPTR if allocation failed.
		 */
		virtual RawPacket* copy() = 0;

		/**
		 * @brief Special method that allocates new object of underlying type on a heap.
		 * Object is constructed using move constructor to which this object is passed as other instance.
		 * @return Pointer to the allocated object or PCAPPP_NULLPTR if allocation failed.
		 */
		virtual RawPacket* move() = 0;

		/* Abstract API: Data acquisition */

		/**
		 * @brief Method to get raw data pointer.
		 * This overload is called if object is not const-qualified.
		 * @return A pointer to the raw data.
		 */
		virtual pointer getRawData() = 0;

		/**
		 * @brief Method to get raw data pointer.
		 * This overload is called if object is const-qualified.
		 * @return A pointer to the const-qualified raw data.
		 */
		virtual const_pointer getRawData() const = 0;

		/**
		 * @brief Method to get raw data pointer.
		 * Same as call to getRawData member function of const object.
		 * @return A pointer to the const-qualified raw data.
		 */
		virtual const_pointer getRawDataReadOnly() const = 0;

		/**
		 * @brief Method to get raw data length in bytes.
		 * @return Raw data length in bytes.
		 */
		virtual length getRawDataLen() const = 0;

		/**
		 * @brief Method to get an indication whether raw data was already set for this instance.
		 * Raw data can be set using the non-default constructor, using setRawData(), using
		 * the copy constructor or using the assignment operator.
		 * @return True if raw data was set for this instance, false otherwise.
		 */
		virtual bool isPacketSet() const = 0;

		/**
		 * @brief Returns owning status of underlying data.
		 * @return true if object owns it's underlying data, false otherwise.
		 */
		virtual bool isOwning() const = 0;

		/* Abstract API: Data modification */

		/**
		 * @brief Represents the facility to release the ownership of underlying data.
		 * @return Pointer to beginning of underlying data.
		 */
		virtual pointer releseData() = 0;

		/**
		 * @brief Reallocates underlying data to a specified size.
		 * @todo m_FrameLength member may be invalidated here.
		 * @param[in] newBufferLength New size of data.
		 * @param[in] initialValue Per-byte initial value of new memory on allocation.
		 * @return true if operation ended successfully, false otherwise (you may expect that object is in null-state).
		 */
		virtual bool reallocateData(size newBufferLength, memory_value initialValue = 0) = 0;

		/**
		 * @brief Clear/deallocate underlying data and set object to a null-state.
		 * @return true if operation ended successfully, false otherwise.
		 */
		virtual bool clear() = 0;

		/**
		 * @brief Append memory capable of holding dataToAppendLen data entries and set it per-byte to initialValue on allocation.
		 * Appending 0 bytes is always a success.
		 * @todo m_FrameLength member may be invalidated here.
		 * @param[in] dataToAppendLen Size of data to be appended.
		 * @param[in] initialValue Initial value for new memory.
		 * @return true if operation ended successfully, false otherwise.
		 */
		virtual bool appendData(size dataToAppendLen, memory_value initialValue = 0) = 0;

		/**
		 * @brief Append memory capable of holding dataToAppendLen data entries and copy data from dataToAppend to it (concatenate).
		 * Appending 0 bytes is always a success.
		 * @todo m_FrameLength member may be invalidated here.
		 * @param[in] dataToAppend Buffer memory to be appended to current data.
		 * @param[in] dataToAppendLen Size of data to be appended.
		 * @return true if operation ended successfully, false otherwise.
		 */
		virtual bool appendData(const_pointer dataToAppend, size dataToAppendLen) = 0;

		/**
		 * @brief Inserts memory capable of holding dataToInsertLen data entries and set it per-byte to initialValue on allocation.
		 * Depending on Memory Proxy type may handle case with negative atIndex.\n
		 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
		 * @todo m_FrameLength member may be invalidated here.
		 * @param[in] atIndex Index before which insertion take place.
		 * @param[in] dataToInsertLen Size of data to be inserted.
		 * @param[in] initialValue Initial value for new memory.
		 * @return true if operation finished successfully, false otherwise.
		 */
		virtual bool insertData(index atIndex, size dataToInsertLen, memory_value initialValue = 0) = 0;
		
		/**
		 * @brief Inserts memory capable of holding dataToInsertLen data entries and copy data from dataToAppend to it.
		 * Depending on Memory Proxy type may handle case with negative atIndex.\n
		 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
		 * @todo m_FrameLength member may be invalidated here.
		 * @param[in] atIndex Index before which insertion take place.
		 * @param[in] dataToInsert Buffer memory to be inserted to current data.
		 * @param[in] dataToInsertLen Size of data to be inserted.
		 * @return true if operation finished successfully, false otherwise.
		 */
		virtual bool insertData(index atIndex, const_pointer dataToInsert, size dataToInsertLen) = 0;

		/**
		 * @brief Removes memory capable of holding numOfBytesToRemove data entries starting from atIndex.
		 * Depending on Memory Proxy type may handle case with negative atIndex is handled correctly.\n
		 * See @ref memory_indexes_in_insert_and_remove_operation "Indexes in Insert and Remove operations" note.
		 * @todo m_FrameLength member may be invalidated here.
		 * @param[in] atIndex Index from which removal take place.
		 * @param[in] numOfBytesToRemove Size of data to be removed.
		 * @return true if operation finished successfully, false otherwise.
		 */
		virtual bool removeData(index atIndex, size numOfBytesToRemove) = 0;

	protected:
		time_t m_TimeStamp;
		length m_FrameLength;
		LinkLayerType m_linkLayerType;
	};

} // namespace pcpp

#endif
