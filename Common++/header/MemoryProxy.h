#ifndef PCAPPP_MEMORYPROXY
#define PCAPPP_MEMORYPROXY

#include "MemoryProxyInterface.h"
#include "SizeAwareMemoryProxy.h"
#include "ContentAwareMemoryProxy.h"
#include "OldMemoryModelMemoryProxy.h"

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
		/**
		 * Represents type that used in memory manipulations as a type of a single memory cell.
		 */
		typedef std::uint8_t Data_t;
		/**
		 * Represents type of default allocator used by the library. It allocates arrays of default data type Data_t.
		 */
		typedef default_allocator<Data_t[]> default_allocator_t;
		/**
		 * Represents type of SizeAwareMemoryProxy that uses default_allocator_t as an allocator.
		 */
		typedef SizeAwareMemoryProxy< default_allocator_t > DefaultSizeAwareMemoryProxy;
		/**
		 * Represents type of ContentAwareMemoryProxy that uses default_allocator_t as an allocator.
		 */
		typedef ContentAwareMemoryProxy< default_allocator_t > DefaultContentAwareMemoryProxy;
		/**
		 * Represents type of OldMemoryModelMemoryProxy that works with default data type Data_t.
		 */
		typedef OldMemoryModelMemoryProxy< Data_t > DefaultOldMemoryModelMemoryProxy;

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORYPROXY */
