#define LOG_MODULE CommonLogModuleIpUtils

#include "Logger.h"
#include "IpAddress.h"
#include "IpUtils.h"
#include <cstring>
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


namespace pcpp
{

/**
 * ===================
 * IPAddress class methods
 * ===================
 */

IPAddress::Ptr_t IPAddress::fromString(char* addressAsString)
{
	in_addr ip4Addr;
	in6_addr ip6Addr;
    if (inet_pton(AF_INET, addressAsString, &ip4Addr) != 0)
    {
    	return IPAddress::Ptr_t(new IPv4Address(addressAsString));
    }
    else if (inet_pton(AF_INET6, addressAsString, &ip6Addr) != 0)
    {
    	return IPAddress::Ptr_t(new IPv6Address(addressAsString));
    }

    return IPAddress::Ptr_t();
}

IPAddress::Ptr_t IPAddress::fromString(std::string addressAsString)
{
	return fromString((char*)addressAsString.c_str());
}

/**
 * ===================
 * IPv4Address class methods
 * ===================
 */

IPv4Address IPv4Address::Zero((uint32_t)0);

void IPv4Address::initialize(char* addressAsString)
{
	m_pInAddr = new in_addr();
	// Parse provided string
	if (inet_pton(AF_INET, addressAsString, m_pInAddr) == 0)
	{
		m_IsValid = false;
		return;
	}
	m_AddressAsString.reset(new char[MAX_ADDR_STRING_LEN]);
	std::strncpy(m_AddressAsString.get(), addressAsString, MAX_ADDR_STRING_LEN);
	m_IsValid = true;
}

IPv4Address::IPv4Address(uint32_t addressAsInt) : 
	m_pInAddr(new in_addr)
{
	std::memcpy(m_pInAddr, &addressAsInt, sizeof(in_addr));
	// Any uint32_t number is by default a valid ip4 address:
	// From 0x00000000 -> 0.0.0.0 
	// To   0xffffffff -> 255.255.255.255
	m_IsValid = true; 
}

IPv4Address::IPv4Address(in_addr* inAddr) :
	m_pInAddr(new in_addr)
{
	std::memcpy(m_pInAddr, inAddr, sizeof(in_addr));
	// Any in_addr is by default a valid ip4 address
	m_IsValid = true; 
}

IPv4Address::IPv4Address(char* addressAsString)
{
	initialize(addressAsString);
}

IPv4Address::IPv4Address(std::string addressAsString)
{
	initialize((char*)addressAsString.c_str());
}

IPv4Address::IPv4Address(const IPv4Address& other) :
	m_pInAddr(new in_addr)
{
	copyDataFrom(other);
}

IPv4Address& IPv4Address::operator=(const IPv4Address& other)
{
	// Handle self assignment case
	if (this == &other) 
		return *this;
	copyDataFrom(other);
	return *this;
}

void IPv4Address::copyDataFrom(const IPv4Address& other)
{
	if (m_pInAddr == nullptr)
		m_pInAddr = new in_addr();
	// Copy address structure
	std::memcpy(m_pInAddr, other.m_pInAddr, sizeof(in_addr));
	// Check that other have a string representation: copying it is fater that creating our own.
	if (other.m_AddressAsString)
	{
		// Reset smart pointer to handle new memory 
		m_AddressAsString.reset(new char[MAX_ADDR_STRING_LEN]);
		// Copy string representation
		std::strncpy(m_AddressAsString.get(), other.m_AddressAsString.get(), MAX_ADDR_STRING_LEN);
	} else {
		// Explicitly free our last string representation
		delete[] m_AddressAsString.release();
	}
	// We now have same validity status as other
	m_IsValid = other.m_IsValid;
}

#ifdef ENABLE_CPP11_MOVE_SEMANTICS

IPv4Address::IPv4Address(IPv4Address&& other) :
	m_pInAddr(nullptr)
{
	moveDataFrom(PCAPPP_MOVE(other));
}

IPv4Address& IPv4Address::operator=(IPv4Address&& other)
{
	// Handle self assignment case
	if (this == &other)
		return *this;
	moveDataFrom(PCAPPP_MOVE(other));
	return *this;
}

void IPv4Address::moveDataFrom(IPv4Address&& other)
{
	if (m_pInAddr)
		delete m_pInAddr;
	if (other.m_pInAddr) 
	{
		// Copy data memberso of other to current object
		m_pInAddr = other.m_pInAddr;
		m_IsValid = other.m_IsValid;
		// Reset pointer of other
		other.m_pInAddr = nullptr;
	} else {
		m_pInAddr = new in_addr();
		// Int this case other is not a valid object
		m_IsValid = false;
	}
	// If other have string representation then obtain owning of it
	if (other.m_AddressAsString) {
		m_AddressAsString.reset(other.m_AddressAsString.release());
	} else { // Deleting a nullptr is not a problem
		delete[] m_AddressAsString.release();
	}
}
#endif

IPv4Address::~IPv4Address()
{	// Deleting a nullptr is not a problem
	delete m_pInAddr;
}

std::string IPv4Address::toString() const
{
	if (!m_AddressAsString && isValid())
	{
		// Reset smart pointer to handle new memory 
		m_AddressAsString.reset(new char[MAX_ADDR_STRING_LEN]);
		// Parse adress
		if (inet_ntop(AF_INET, m_pInAddr, m_AddressAsString.get(), MAX_ADDR_STRING_LEN) == 0)
		{	// Deallocate memory if parsing fails.
			delete[] m_AddressAsString.release();
		}
	}
	// Call base class method that wraps memory of m_AddressAsString to a copy of string
	// RVO used here
	return IPAddress::toString();
}

uint32_t IPv4Address::toInt() const
{
	uint32_t result = 0;
	if (m_pInAddr != nullptr) {
		std::memcpy(&result, m_pInAddr, sizeof(uint32_t));
	}
	return result;
}

bool IPv4Address::matchSubnet(const IPv4Address& subnet, const std::string& subnetMask)
{
	IPv4Address maskAsIpAddr(subnetMask);
	if (!maskAsIpAddr.isValid())
	{
		LOG_ERROR("Subnet mask '%s' is in illegal format", subnetMask.c_str());
		return false;
	}

	int thisAddrAfterMask = toInt() & maskAsIpAddr.toInt();
	int subnetAddrAfterMask = subnet.toInt() & maskAsIpAddr.toInt();
	return (thisAddrAfterMask == subnetAddrAfterMask);
}

/**
 * ===================
 * IPv6Address class methods
 * ===================
 */
IPv6Address IPv6Address::Zero(std::string("0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0"));

void IPv6Address::initialize(char* addressAsString)
{
	m_pInAddr = new in6_addr();
	// Parse provided string
	if (inet_pton(AF_INET6, addressAsString, m_pInAddr) == 0)
	{
		m_IsValid = false;
		return;
	}
	m_AddressAsString.reset(new char[MAX_ADDR_STRING_LEN]);
	std::strncpy(m_AddressAsString.get(), addressAsString, MAX_ADDR_STRING_LEN);
	m_IsValid = true;
}

IPv6Address::IPv6Address(uint8_t* addressAsUintArr) :
	m_pInAddr(new in6_addr)
{
	std::memcpy(m_pInAddr, addressAsUintArr, 16);
	// Any array of 16 uint8_t is by default a valid ip6 address
	m_IsValid = true; 
}

IPv6Address::IPv6Address(char* addressAsString)
{
	initialize(addressAsString);
}

IPv6Address::IPv6Address(std::string addressAsString)
{
	initialize((char*)addressAsString.c_str());
}

IPv6Address::IPv6Address(const IPv6Address& other) :
	m_pInAddr(new in6_addr)
{
	copyDataFrom(other);
}

IPv6Address& IPv6Address::operator=(const IPv6Address& other)
{
	// Handle self assignment case
	if (this == &other)
		return *this;
	copyDataFrom(other);
	return *this;
}

void IPv6Address::copyDataFrom(const IPv6Address& other)
{
	if (m_pInAddr == nullptr)
		m_pInAddr = new in6_addr();
	// Copy address structure
	std::memcpy(m_pInAddr, other.m_pInAddr, sizeof(in6_addr));
	// Check that other have a string representation: copying it is fater that creating our own.
	if (other.m_AddressAsString)
	{
		// Reset smart pointer to handle new memory 
		m_AddressAsString.reset(new char[MAX_ADDR_STRING_LEN]);
		// Copy string representation
		std::strncpy(m_AddressAsString.get(), other.m_AddressAsString.get(), MAX_ADDR_STRING_LEN);
	} else {
		// Explicitly free our last string representation
		delete[] m_AddressAsString.release();
	}
	// We now have same validity status as other
	m_IsValid = other.m_IsValid;
}

#ifdef ENABLE_CPP11_MOVE_SEMANTICS
IPv6Address::IPv6Address(IPv6Address&& other) :
	m_pInAddr(nullptr)
{
	moveDataFrom(PCAPPP_MOVE(other));
}

IPv6Address& IPv6Address::operator=(IPv6Address&& other)
{
	// Handle self assignment case
	if (this == &other) 
		return *this;
	moveDataFrom(PCAPPP_MOVE(other));
	return *this;
}

void IPv6Address::moveDataFrom(IPv6Address&& other)
{
	if (m_pInAddr)
		delete m_pInAddr;
	if (other.m_pInAddr) 
	{
		// Copy data memberso of other to current object
		m_pInAddr = other.m_pInAddr;
		m_IsValid = other.m_IsValid;
		// Reset pointer of other
		other.m_pInAddr = nullptr;
	} else {
		m_pInAddr = new in6_addr();
		// Int this case other is not a valid object
		m_IsValid = false;
	}
	// If other have string representation then obtain owning of it
	if (other.m_AddressAsString) {
		m_AddressAsString.reset(other.m_AddressAsString.release());
	} else { // Deleting a nullptr is not a problem
		delete[] m_AddressAsString.release();
	}
}
#endif

IPv6Address::~IPv6Address()
{	// Deleting a nullptr is not a problem
	delete m_pInAddr;
}

std::string IPv6Address::toString() const
{
	if (!m_AddressAsString && isValid())
	{
		// Reset smart pointer to handle new memory 
		m_AddressAsString.reset(new char[MAX_ADDR_STRING_LEN]);
		// Parse adress
		if (inet_ntop(AF_INET6, m_pInAddr, m_AddressAsString.get(), MAX_ADDR_STRING_LEN) == 0)
		{	// Deallocate memory if parsing fails.
			delete[] m_AddressAsString.release();
		}
	}
	// Call base class method that wraps memory of m_AddressAsString to a copy of string
	// RVO used here
	return IPAddress::toString();
}

void IPv6Address::copyTo(uint8_t** arr, size_t& length) const
{
	length = 16;
	(*arr) = new uint8_t[length];
	std::memcpy((*arr), m_pInAddr, length);
}

void IPv6Address::copyTo(uint8_t* arr) const
{
	std::memcpy(arr, m_pInAddr, 16);
}

} // namespace pcpp
