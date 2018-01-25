#ifndef PCAPPP_SAFEOLDMEMORYMODELMEMORYPROXY
#define PCAPPP_SAFEOLDMEMORYMODELMEMORYPROXY

#include <cstdint>
#include <exception>

#include "MemoryProxyInterface.h"

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
		/* // Is not used
		template < typename T, typename Allocator >
		class MemoryProxy< T, Allocator, MemoryProxyTags::SafeOldMemoryModelTag > :
			public MemoryProxyInterface<T, Allocator, MemoryProxyTags::SafeOldMemoryModelTag >
		{
		public:
			typedef MemoryProxyInterface<T, Allocator, MemoryProxyTags::SafeOldMemoryModelTag > Base;
		protected:

		public:

		protected:
			mutable Adapter m_Allocator;
			pointer m_Data;
			size m_Length;
			bool m_Ownership;
		};
		*/
	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_SAFEOLDMEMORYMODELMEMORYPROXY */