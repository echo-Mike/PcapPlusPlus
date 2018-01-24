#ifndef PCAPPP_MEMORYPROXY
#define PCAPPP_MEMORYPROXY

#include "MemoryProxyInterface.h"
#include "SizeAwareMemoryProxy.h"
#include "ContentAwareMemoryProxy.h"
#include "OldMemoryModelMemoryProxy.h"
#include "SafeOldMemoryModelMemoryProxy.h"

/// @file

/**
 * \namespace pcpp
 * \brief The main namespace for the PcapPlusPlus lib
 */
namespace pcpp
{
	/**
	 * \namespace memory
	 * \brief Namespace that contains memory manipulation features
	 */
	namespace memory
	{

		typedef std::uint8_t Data_t;

		typedef MemoryProxy<Data_t, default_allocator<Data_t[]>, MemoryProxyTags::SizeAwareTag> DefaultSizeAwareMemoryProxy;

		typedef MemoryProxy<Data_t, default_allocator<Data_t[]>, MemoryProxyTags::ContentAwareTag> DefaultContentAwareMemoryProxy;

		typedef MemoryProxy<Data_t, default_allocator<Data_t[]>, MemoryProxyTags::OldMemoryModelTag> DefaultOldMemoryModelMemoryProxy;

		typedef MemoryProxy<Data_t, default_allocator<Data_t[]>, MemoryProxyTags::SafeOldMemoryModelTag> DefaultSafeOldMemoryModelMemoryProxy;

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORYPROXY */
