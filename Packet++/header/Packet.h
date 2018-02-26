#ifndef PACKETPP_PACKET
#define PACKETPP_PACKET

#include "RawPacket.h"
#include "GenericRawPacket.h"
#include "Layer.h"
#ifdef USE_DPDK
// When DPDK is used there is a derived class from RawPacket - MBufRawPacket
// that must be handled differently in castBasedCopyResetRawPacket and castBasedMoveResetRawPacket member functions.
#include "MBufRawPacket.h" 
#endif // USE_DPDK

#include <utility>
#include <vector>

/// @file

/**
 * \namespace pcpp
 * \brief The main namespace for the PcapPlusPlus lib
 */
namespace pcpp
{

	/**
	 * @class Packet
	 * This class represents a parsed packet. It contains the raw data (RawPacket instance), and a linked list of layers, each layer is a parsed
	 * protocol that this packet contains. The layers linked list is ordered where the first layer is the lowest in the packet (currently it's always
	 * Ethernet protocol as PcapPlusPlus supports only Ethernet packets), the next layer will be L2.5 or L3 (e.g VLAN, IPv4, IPv6, etc.), and so on.
	 * etc.), etc. The last layer in the linked list will be the highest in the packet.
	 * For example: for a standard HTTP request packet the layer will look like this: EthLayer -> IPv4Layer -> TcpLayer -> HttpRequestLayer <BR>
	 * Packet instance isn't read only. The user can add or remove layers, update current layer, etc.\n
	 * This class has the null-object state - state after initialize member function call.
	 *  Owning rules of underlying data:  
	 * * Objects in null-state don't own anything;  
	 * * Objects constructed by default constructor own underlying RawPacket object;  
	 * * Objects constructed with constructors other than default own their underlying RawPacket object dependent on presence and value of freeRawPacket paramather;  
	 * * Objects created by copy from other own their copy of underlying RawPacket object;  
	 * * Move operation transfers owning state from one object to another (a.e. if object from which move was made owns it's data then object to which move was made now owns this data and vise-versa);  
	 * * Object from which move was made is a object in null-state;  
	 * * Underlying RawPacket object is freed on object deconstruction only if object owns it;  
	 * * Object definitely own it's underlying RawPacket object after call to resetRawPacket member function.  
	 *
	 */
	class Packet {
		friend class Layer;
		RawPacket* m_RawPacket;
		Layer* m_FirstLayer;
		Layer* m_LastLayer;
		uint64_t m_ProtocolTypes;
		size_t m_MaxPacketLen;
		bool m_FreeRawPacket;

		/**
		 * @brief Setups Packet object to a null-state.
		 * Basically zeroes all fields, no data is touched.
		 */
		void initialize();

		/**
		 * @brief Makes current object a copy of the other.
		 * Deallocates current data.
		 * @param[in] other Object to make copy from.
		 * @todo Add new layer structure copy mechanism.
		 * @todo Close UB for other with corrupted RawPacket.
		 */
		void copyDataFrom(const Packet& other);

		/**
		 * @brief Moves data from other.
		 * Deallocates current data. Underlying raw packet of other is not copied.\n
		 * m_Packet pointers in all layers are switched to this object.\n
		 * other is set to the null-state - state after default construction.
		 * @param[in:out] other Object to move from.
		 */
		void moveDataFrom(Packet&& other);

		/**
		 * @brief Deallocates packet data.
		 * Deallocates layers allocated in packet and underlying RawPacket object if m_FreeRawPacket is up.
		 */
		void destructPacketData();

		/**
		 * @brief Re-construct all packet layers.
		 * @param[in] parseUntil Parse the packet until it reaches this protocol. Can be useful for cases when you need to parse only up to a certain layer and want to avoid the
		 * performance impact and memory consumption of parsing the whole packet. Default value is ::UnknownProtocol which means don't take this parameter into account
		 * @param[in] parseUntilLayer Parse the packet until certain layer in OSI model. Can be useful for cases when you need to parse only up to a certain layer and want to avoid the
		 * performance impact and memory consumption of parsing the whole packet. Default value is ::OsiModelLayerUnknown which means don't take this parameter into account
		 */
		void parseLayers(ProtocolType parseUntil = UnknownProtocol, OsiModelLayer parseUntilLayer = OsiModelLayerUnknown);
	public:

		/**
		 * @brief A default constructor for creating a new packet. 
		 * Very useful when creating packets. When using this constructor an 
		 * underlying RawPacket object is created in null-state ready for adding layers.
		 * @param[in] maxPacketLen The expected packet length in bytes.
		 */
		Packet(size_t maxPacketLen = 1);

		/**
		 * @brief A constructor for creating a packet out of already allocated RawPacket. 
		 * Very useful when parsing packets that came from the network.
		 * When using this constructor a pointer to the RawPacket is saved (data isn't copied) and the RawPacket is parsed, meaning all layers
		 * are created and linked to each other in the right order. In this overload of the constructor the user can specify whether to free
		 * the instance of raw packet when the Packet is free or not
		 * @param[in] rawPacket A pointer to the raw packet
		 * @param[in] freeRawPacket Optional parameter. A flag indicating if the destructor should also call the raw packet destructor or not. Default value is false
		 * @param[in] parseUntil Optional parameter. Parse the packet until you reach a certain protocol (inclusive). Can be useful for cases when you need to parse only up to a
		 * certain layer and want to avoid the performance impact and memory consumption of parsing the whole packet. Default value is ::UnknownProtocol which means don't take this
		 * parameter into account
		 * @param[in] parseUntilLayer Optional parameter. Parse the packet until you reach a certain layer in the OSI model (inclusive). Can be useful for cases when you need to
		 * parse only up to a certain OSI layer (for example transport layer) and want to avoid the performance impact and memory consumption of parsing the whole packet.
		 * Default value is ::OsiModelLayerUnknown which means don't take this parameter into account
		 */
		Packet(RawPacket* rawPacket, bool freeRawPacket = false, ProtocolType parseUntil = UnknownProtocol, OsiModelLayer parseUntilLayer = OsiModelLayerUnknown);

		/**
		 * @brief A constructor for creating a packet out of already allocated RawPacket. 
		 * Very useful when parsing packets that came from the network.
		 * When using this constructor a pointer to the RawPacket is saved (data isn't copied) and the RawPacket is parsed, meaning all layers
		 * are created and linked to each other in the right order. In this overload of the constructor the user can specify whether to free
		 * the instance of raw packet when the Packet is free or not. This constructor should be used to parse the packet up to a certain layer
		 * @param[in] rawPacket A pointer to the raw packet
		 * @param[in] parseUntil Optional parameter. Parse the packet until you reach a certain protocol (inclusive). Can be useful for cases when you need to parse only up to a
		 * certain layer and want to avoid the performance impact and memory consumption of parsing the whole packet
		 */
		Packet(RawPacket* rawPacket, ProtocolType parseUntil);

		/**
		 * @brief A constructor for creating a packet out of already allocated RawPacket. 
		 * Very useful when parsing packets that came from the network.
		 * When using this constructor a pointer to the RawPacket is saved (data isn't copied) and the RawPacket is parsed, meaning all layers
		 * are created and linked to each other in the right order. In this overload of the constructor the user can specify whether to free
		 * the instance of raw packet when the Packet is free or not. . This constructor should be used to parse the packet up to a certain layer in the OSI model
		 * @param[in] rawPacket A pointer to the raw packet
		 * @param[in] parseUntilLayer Optional parameter. Parse the packet until you reach a certain layer in the OSI model (inclusive). Can be useful for cases when you need to
		 * parse only up to a certain OSI layer (for example transport layer) and want to avoid the performance impact and memory consumption of parsing the whole packet
		 */
		Packet(RawPacket* rawPacket, OsiModelLayer parseUntilLayer);

		/**
		 * @brief Copy constructor that copies all data from another instance.
		 * Copies the raw data and re-creates all layers. So when the original Packet
		 * is being freed, no data will be lost in the copied instance.
		 * @param[in] other The instance to make copy of.
		 */
		Packet(const Packet& other);

		/**
		 * @brief Copy assignment operator of this class.
		 * It first frees all layers allocated by this instance (Notice: it doesn't free layers that weren't allocated by this
		 * class, for example layers that were added by addLayer() or insertLayer() ). In addition it frees the raw packet if it was allocated by
		 * this instance (meaning if it was allocated by this instance constructor).
		 * Afterwards it copies the data from the other packet in the same way used in the copy constructor.
		 * @param[in] other The instance to make copy of.
		 */
		Packet& operator=(const Packet& other);
		
		/**
		 * @brief Move constructor that moves all data from another instance.
		 * Notice no raw data is copied all other data members are copied.\n
		 * Sets the new object to the null-state and internally calls moveDataFrom member function.\n
		 * other object is set to the null-state.
		 * @param[in] other The instance to move from.
		 */
		Packet(Packet&& other);

		/**
		 * @brief Move assignment operator of this class.
		 * When using this operator on an already initialized Packet instance,
		 * the original raw data is freed first. Then the data of other instance 
		 * is moved to this instance, the same way the move constructor works.
		 * @param[in] other The instance to move from.
		 */
		Packet& operator=(Packet&& other);

		/**
		 * @brief Destructor for this class. 
		 * Frees all layers allocated by this instance (Notice: it doesn't free layers that weren't allocated by this
		 * class, for example layers that were added by addLayer() or insertLayer() ). In addition it frees the raw packet if it was allocated by
		 * this instance (meaning if it was allocated by this instance constructor)
		 */
		~Packet();

		/**
		 * @brief Set a RawPacket and re-construct all packet layers.
		 * @param[in] rawPacket Raw packet to set
		 * @param[in] freeRawPacket A flag indicating if the destructor should also call the raw packet destructor or not
		 * @param[in] parseUntil Parse the packet until it reaches this protocol. Can be useful for cases when you need to parse only up to a certain layer and want to avoid the
		 * performance impact and memory consumption of parsing the whole packet. Default value is ::UnknownProtocol which means don't take this parameter into account
		 * @param[in] parseUntilLayer Parse the packet until certain layer in OSI model. Can be useful for cases when you need to parse only up to a certain layer and want to avoid the
		 * performance impact and memory consumption of parsing the whole packet. Default value is ::OsiModelLayerUnknown which means don't take this parameter into account
		 */
		void setRawPacket(RawPacket* rawPacket, bool freeRawPacket, ProtocolType parseUntil = UnknownProtocol, OsiModelLayer parseUntilLayer = OsiModelLayerUnknown);
		
		/**
		 * @brief Resets underlying raw packet to provided packet rawPacket by copy- or move-constructor.
		 * Deallocation of current underlying raw packet is made only after new packet is allocated and only if m_FreeRawPacket is up.\n
		 * If it is possible layers are not reset.
		 * @param[in] rp Left- or Right-reference to value or a value itself. Raw packet to use in operations.
		 * Usage:\n
		 * Obtain owning of current underlying raw packet by making copy of it:\n
		 * packet.resetRawPacket(*packet.getRawPacket());\n
		 * Or by moving data from it:\n
		 * packet.resetRawPacket(std::move(*packet.getRawPacket()));\n
		 * If provided rawPacket is obtained from pointer dereferencing and it points to object of class derived from RawPacket (a.e. MBufRawPacket)
		 * then the pointer must be casted to the derived type before dereferencing:\n
		 * packet.resetRawPacket(*dynamic_cast<MBufRawPacket*>(packet.getRawPacket())); // Copy\n
		 * packet.resetRawPacket(std::move(*dynamic_cast<MBufRawPacket*>(packet.getRawPacket()))); // Move
		 */
		template < typename RawPacketT >
		void resetRawPacket(RawPacketT&& rawPacket);

		/**
		 * @brief Method to get a pointer to the underlying RawPacket.
		 * This overload is called if object is not const-qualified.
		 * @return A pointer to the underlying RawPacket.
		 */
		inline RawPacket* getRawPacket() { return m_RawPacket; }

		/**
		 * @brief Method to get a pointer to the underlying RawPacket.
		 * This overload is called if object is const-qualified.
		 * @return A pointer to the const-qualified underlying RawPacket.
		 */
		inline const RawPacket* getRawPacket() const { return m_RawPacket; }

#ifdef USE_DPDK // If DPDK is presented in project then there is an derived class of RawPacket - MBufRawPacket

		/**
		 * @brief Method to call resetRawPacket with automatically defined type of underlying RawPacket.
		 * Call this function if packet needs to be reset by calling copy-constructor.\n
		 * Uses the dynamic_cast behavior to return nullptr if cast failed.
		 */
		void castBasedCopyResetRawPacket() 
		{
			MBufRawPacket* castResult = dynamic_cast<MBufRawPacket*>(getRawPacket());
			if (castResult)
				resetRawPacket(*castResult);
			else
				resetRawPacket(*getRawPacket()); 
		}

		/**
		 * @brief Method to call resetRawPacket with automatically defined type of underlying RawPacket.
		 * Call this function if packet needs to be reset by calling move-constructor.\n
		 * Uses the dynamic_cast behavior to return nullptr if cast failed.
		 */
		void castBasedMoveResetRawPacket()
		{
			MBufRawPacket* castResult = dynamic_cast<MBufRawPacket*>(getRawPacket());
			if (castResult)
				resetRawPacket(std::move(*castResult));
			else
				resetRawPacket(std::move(*getRawPacket()));
		}
#else // If it is not then the only class under RawPacket* is RawPacket

		/**
		 * @brief Method to call resetRawPacket with automatically defined type of underlying RawPacket.
		 * Call this function if packet needs to be reset by calling copy-constructor.\n
		 * Uses the dynamic_cast behavior to return nullptr if cast failed.
		 */
		void castBasedCopyResetRawPacket() { resetRawPacket(*getRawPacket()); }

		/**
		 * @brief Method to call resetRawPacket with automatically defined type of underlying RawPacket.
		 * Call this function if packet needs to be reset by calling move-constructor.\n
		 * Uses the dynamic_cast behavior to return nullptr if cast failed.
		 */
		void castBasedMoveResetRawPacket() { resetRawPacket(std::move(*getRawPacket())); }
#endif // USE_DPDK

		/**
		 * @brief Method to get a pointer to the underlying RawPacket in read-only manner.
		 * @return A pointer to the underlying RawPacket.
		 */
		inline RawPacket* getRawPacketReadOnly() const { return m_RawPacket; }

		/**
		 * @brief Method to get a pointer to the first (lowest) layer in the packet.
		 * This overload is called if object is not const-qualified.
		 * @return A pointer to the first (lowest) layer in the packet.
		 */
		inline Layer* getFirstLayer() { return m_FirstLayer; }

		/**
		 * @brief Method to get a pointer to the first (lowest) layer in the packet.
		 * This overload is called if object is const-qualified.
		 * @return A pointer to the first (lowest) layer in the packet.
		 */
		inline const Layer* getFirstLayer() const { return m_FirstLayer; }

		/**
		 * @brief Method to get a pointer to the last (highest) layer in the packet.
		 * This overload is called if object is not const-qualified.
		 * @return A pointer to the last (highest) layer in the packet.
		 */
		inline Layer* getLastLayer() { return m_LastLayer; }

		/**
		 * @brief Method to get a pointer to the last (highest) layer in the packet.
		 * This overload is called if object is const-qualified.
		 * @return A pointer to the last (highest) layer in the packet.
		 */
		inline const Layer* getLastLayer() const { return m_LastLayer; }

		/**
		 * @brief Method to check if current object is in the null-state.
		 * m_MaxPacketLen is ignored on purpose - all objects created via default constructor is in the null-state (ant their underlying RawPacket objects as well).
		 * @return true if object NOT in the null-state, false otherwise.
		 */
		inline operator bool() const { return m_RawPacket || m_LastLayer || m_FirstLayer || m_FreeRawPacket || m_ProtocolTypes != UnknownProtocol; }

		/**
		 * @brief Method to check if current object is in the null-state.
		 * Basically the negation of operator bool call.
		 * @return true if object is in the null-state, false otherwise.
		 */
		inline bool isInNullState() const { return !(*this); }

		/**
		 * @brief Add a new layer as the last layer in the packet. 
		 * This method gets a pointer to the new layer as a parameter and attaches it to the packet. 
		 * Notice after calling this method the input layer is attached to the packet so
		 * every change you make in it affect the packet. Also it cannot be attached to other packets.\n
		 * If underlying RawPacket object was in null-state then raw data for it will be allocated here.
		 * @param[in] newLayer A pointer to the new layer to be added to the packet.
		 * @return True if everything went well or false otherwise (an appropriate error log message will be printed in
		 * such cases).
		 */
		bool addLayer(Layer* newLayer);

		/**
		 * @brief Insert a new layer after an existing layer in the packet. 
		 * This method gets a pointer to the new layer as a parameter and attaches it to the packet. 
		 * Notice after calling this method the input layer is attached to the packet 
		 * so every change you make in it affect the packet. Also it cannot be attached to other packets.\n
		 * If underlying RawPacket object was in null-state then raw data for it will be allocated here.
		 * @param[in] prevLayer A pointer to an existing layer in the packet which the new layer should followed by. If
		 * this layer isn't attached to a packet and error will be printed to log and false will be returned.
		 * @param[in] newLayer A pointer to the new layer to be added to the packet.
		 * @return True if everything went well or false otherwise (an appropriate error log message will be printed in
		 * such cases).
		 */
		bool insertLayer(Layer* prevLayer, Layer* newLayer);

		/**
		 * @brief Remove an existing layer from the packet.
		 * @param[in] layer The layer to remove.
		 * @return True if everything went well or false otherwise (an appropriate error log message will be printed in
		 * such cases).
		 */
		bool removeLayer(Layer* layer);

		/**
		 * @brief A templated method to get a layer of a certain type (protocol). 
		 * If no layer of such type is found, NULL is returned.
		 * @return A pointer to the layer of the requested type, NULL if not found.
		 */
		template<class TLayer>
		TLayer* getLayerOfType();

		/**
		 * @brief A templated method to get the first layer of a certain type (protocol), start searching from a certain layer.
		 * For example: if a packet looks like: EthLayer -> VlanLayer(1) -> VlanLayer(2) -> VlanLayer(3) -> IPv4Layer
		 * and the user put VlanLayer(2) as a parameter and wishes to search for a VlanLayer, VlanLayer(3) will be returned.
		 * If no layer of such type is found, NULL is returned.
		 * @param[in] after A pointer to the layer to start search from.
		 * @return A pointer to the layer of the requested type, NULL if not found.
		 */
		template<class TLayer>
		TLayer* getNextLayerOfType(Layer* after);

		/**
		 * @brief Check whether the packet contains a certain protocol.
		 * @param[in] protocolType The protocol type to search.
		 * @return True if the packet contains the protocol, false otherwise.
		 */
		inline bool isPacketOfType(ProtocolType protocolType) const { return m_ProtocolTypes & protocolType; }

		/**
		 * @brief Calls Layer#computeCalculateFields on every known layer object in packet.
		 * Each layer can have fields that can be calculate automatically from other fields using Layer#computeCalculateFields(). 
		 * This method forces all layers to calculate these fields values.
		 */
		void computeCalculateFields();

		/**
		 * Each layer can print a string representation of the layer most important data using Layer#toString(). This method aggregates this string from all layers and
		 * print it to a complete string containing all packet's relevant data
		 * @param[in] timeAsLocalTime Print time as local time or GMT. Default (true value) is local time, for GMT set to false
		 * @return A string containing most relevant data from all layers (looks like the packet description in Wireshark)
		 */
		std::string printToString(bool timeAsLocalTime = true);

		/**
		 * Similar to printToString(), but instead of one string it outputs a list of strings, one string for every layer
		 * @param[out] result A string vector that will contain all strings
		 * @param[in] timeAsLocalTime Print time as local time or GMT. Default (true value) is local time, for GMT set to false
		 */
		void printToStringList(std::vector<std::string>& result, bool timeAsLocalTime = true);

	private:
		bool extendLayer(Layer* layer, int offsetInLayer, size_t numOfBytesToExtend);
		bool shortenLayer(Layer* layer, int offsetInLayer, size_t numOfBytesToShorten);

		void reallocateRawData(size_t newSize);

		std::string printPacketInfo(bool timeAsLocalTime);
	};

	template < typename RawPacketT >
	void Packet::resetRawPacket(RawPacketT&& rawPacket)
	{
		// Obtain real (not const- or volatile-qualified) type of rawPacket
		using RealRawPacketT = typename std::decay<RawPacketT>::type; // std::decay_t is the c++14 feature
		static_assert(std::is_base_of<RawPacket, RealRawPacketT>::value, "Provided object must be derived from RawPacket or be RawPacket.");
		// This will assert if RawPacketT is const-qualified and have been moved to this function
		static_assert(std::is_constructible<RealRawPacketT, decltype(std::forward<RawPacketT>(rawPacket))>::value, "Provided type must have copy- or move-constructor.");
		// Store old raw packet in separate pointer
		RawPacket* oldRawPacket = m_RawPacket;
		bool oldFreRawPacket = m_FreeRawPacket;
		// Allocate new raw packet object by copy- or move-construction
		m_RawPacket = new RealRawPacketT(std::forward<RawPacketT>(rawPacket));
		m_FreeRawPacket = false; // Turn off raw packet deallocation in next destructPacketData call
		if ( !std::is_same<RawPacketT, RealRawPacketT>::value || // This is true if copy-constructor was called
			oldRawPacket != &rawPacket ) // Or if move-constructor called on not current underlying RawPacket
		{	
			// Layers now are messed up => recreate them
			destructPacketData();
			parseLayers();
			// If rawPacket is current underlying RawPacket and move-constructor is called
			// then all layers are not messed up and will still point to correct memory locations
		}
		// Deallocate old object if it is needed
		if (oldFreRawPacket)
			delete oldRawPacket;
		// We definitely own an underlying RawPacket now
		m_FreeRawPacket = true;
	}

	template<class TLayer>
	TLayer* Packet::getLayerOfType()
	{
		if (dynamic_cast<TLayer*>(m_FirstLayer) != NULL)
			return (TLayer*)m_FirstLayer;

		return getNextLayerOfType<TLayer>(m_FirstLayer);
	}

	template<class TLayer>
	TLayer* Packet::getNextLayerOfType(Layer* after)
	{
		if (after == NULL)
			return NULL;

		Layer* curLayer = after->getNextLayer();
		while ((curLayer != NULL) && (dynamic_cast<TLayer*>(curLayer) == NULL))
		{
			curLayer = curLayer->getNextLayer();
		}

		return (TLayer*)curLayer;
	}

} // namespace pcpp

#endif /* PACKETPP_PACKET */
