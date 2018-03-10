#define LOG_MODULE CommonLogModuleIpUtils

#include <cstring>

#include "Logger.h"
#include "IpAddress.h"
#include "IpUtils.h"

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
    	return PCAPPP_MOVE_OR_RVO(IPAddress::Ptr_t(new IPv4Address(addressAsString)));
    }
    else if (inet_pton(AF_INET6, addressAsString, &ip6Addr) != 0)
    {
    	return PCAPPP_MOVE_OR_RVO(IPAddress::Ptr_t(new IPv6Address(addressAsString)));
    }

    return PCAPPP_MOVE_OR_RVO(IPAddress::Ptr_t());
}

IPAddress::Ptr_t IPAddress::fromString(std::string addressAsString)
{
	return PCAPPP_MOVE_OR_RVO(fromString((char*)addressAsString.c_str()));
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

PCAPPP_COPY_CONSTRUCTOR_IMPL(IPv4Address) :
	m_pInAddr(new in_addr)
{
	copyDataFrom(PCAPPP_COPY_OTHER);
}

PCAPPP_COPY_ASSIGNMENT_IMPL(IPv4Address)
{
	// Handle self assignment case
	if (this == &PCAPPP_COPY_OTHER)
		return *this;
	copyDataFrom(PCAPPP_COPY_OTHER);
	return *this;
}

void IPv4Address::copyDataFrom(const IPv4Address& other)
{
	if (m_pInAddr == PCAPPP_NULLPTR)
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
		if (m_AddressAsString)
			delete[] m_AddressAsString.release();
	}
	// We now have same validity status as other
	m_IsValid = other.m_IsValid;
}

PCAPPP_MOVE_CONSTRUCTOR_IMPL(IPv4Address) :
	m_pInAddr(PCAPPP_NULLPTR)
{
	PCAPPP_PREPARE_MOVE_OTHER_I(IPv4Address)
	moveDataFrom(PCAPPP_MOVE_OTHER_I);
}

PCAPPP_MOVE_ASSIGNMENT_IMPL(IPv4Address)
{
	PCAPPP_PREPARE_MOVE_OTHER_I(IPv4Address)
	// Handle self assignment case
	if (this == &PCAPPP_MOVE_OTHER_I)
		return *this;
	moveDataFrom(PCAPPP_MOVE_OTHER_I);
	return *this;
}

void IPv4Address::moveDataFrom(IPv4Address& other)
{
	if (m_pInAddr)
		delete m_pInAddr;
	if (other.m_pInAddr != PCAPPP_NULLPTR)
	{
		// Copy data members of other to current object
		m_pInAddr = other.m_pInAddr;
		m_IsValid = other.m_IsValid;
		// Reset pointer of other
		other.m_pInAddr = PCAPPP_NULLPTR;
	} else {
		m_pInAddr = new in_addr();
		// Int this case other is not a valid object
		m_IsValid = false;
	}
	// If other have string representation then obtain owning of it
	if (other.m_AddressAsString) {
		m_AddressAsString = PCAPPP_MOVE(other.m_AddressAsString);
	} else {
		if (m_AddressAsString)
			delete[] m_AddressAsString.release();
	}
}

IPv4Address::~IPv4Address()
{
	if (m_pInAddr != PCAPPP_NULLPTR)
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
	if (m_pInAddr != PCAPPP_NULLPTR) {
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

PCAPPP_COPY_CONSTRUCTOR_IMPL(IPv6Address) :
	m_pInAddr(new in6_addr)
{
	copyDataFrom(PCAPPP_COPY_OTHER);
}

PCAPPP_COPY_ASSIGNMENT_IMPL(IPv6Address)
{
	// Handle self assignment case
	if (this == &PCAPPP_COPY_OTHER)
		return *this;
	copyDataFrom(PCAPPP_COPY_OTHER);
	return *this;
}

void IPv6Address::copyDataFrom(const IPv6Address& other)
{
	if (m_pInAddr == PCAPPP_NULLPTR)
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
		if (m_AddressAsString)
			delete[] m_AddressAsString.release();
	}
	// We now have same validity status as other
	m_IsValid = other.m_IsValid;
}

PCAPPP_MOVE_CONSTRUCTOR_IMPL(IPv6Address) :
	m_pInAddr(PCAPPP_NULLPTR)
{
	PCAPPP_PREPARE_MOVE_OTHER_I(IPv6Address)
	moveDataFrom(PCAPPP_MOVE_OTHER_I);
}

PCAPPP_MOVE_ASSIGNMENT_IMPL(IPv6Address)
{
	PCAPPP_PREPARE_MOVE_OTHER_I(IPv6Address)
	// Handle self assignment case
	if (this == &PCAPPP_MOVE_OTHER_I)
		return *this;
	moveDataFrom(PCAPPP_MOVE_OTHER_I);
	return *this;
}

void IPv6Address::moveDataFrom(IPv6Address& other)
{
	if (m_pInAddr)
		delete m_pInAddr;
	if (other.m_pInAddr != PCAPPP_NULLPTR)
	{
		// Copy data members of other to current object
		m_pInAddr = other.m_pInAddr;
		m_IsValid = other.m_IsValid;
		// Reset pointer of other
		other.m_pInAddr = PCAPPP_NULLPTR;
	} else {
		m_pInAddr = new in6_addr();
		// Int this case other is not a valid object
		m_IsValid = false;
	}
	// If other have string representation then obtain owning of it
	if (other.m_AddressAsString) {
		m_AddressAsString = PCAPPP_MOVE(other.m_AddressAsString);
	} else {
		if (m_AddressAsString)
			delete[] m_AddressAsString.release();
	}
}

IPv6Address::~IPv6Address()
{
	if (m_pInAddr != PCAPPP_NULLPTR)
		delete m_pInAddr;
}

std::string IPv6Address::toString() const
{
	if (!m_AddressAsString && isValid())
	{
		// Reset smart pointer to handle new memory 
		m_AddressAsString.reset(new char[MAX_ADDR_STRING_LEN]);
		// Parse address
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

bool IPv6Address::operator==(const IPv6Address& other) const
{
	return isValid() && other.isValid() && (std::memcmp(m_pInAddr, other.m_pInAddr, sizeof(in6_addr)) == 0);
}

} // namespace pcpp
