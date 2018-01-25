#ifndef PCAPPP_MEMORYPROXY
#define PCAPPP_MEMORYPROXY

//#define PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER
#define PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR

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

		typedef std::uint8_t Data_t;

		typedef default_allocator<Data_t[]> default_allocator_t;

		typedef MemoryProxy<default_allocator_t, MemoryProxyTags::SizeAwareTag> DefaultSizeAwareMemoryProxy;

		typedef MemoryProxy<default_allocator_t, MemoryProxyTags::ContentAwareTag> DefaultContentAwareMemoryProxy;

		typedef MemoryProxy<default_allocator_t, MemoryProxyTags::OldMemoryModelTag> DefaultOldMemoryModelMemoryProxy;

		/*
		 * NOTE: Size comparison.
		 *
		 * 1) PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER is not defined and PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR is defined
		 * 2) PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER is defined     and PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR is defined
		 * 3) PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER is not defined and PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR is not defined
		 * 4) PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER is defined     and PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR is not defined
		 *
		 * A) sizeof(DefaultSizeAwareMemoryProxy)
		 * B) sizeof(DefaultContentAwareMemoryProxy)
		 * C) sizeof(DefaultOldMemoryModelMemoryProxy)
		 * D) siseof(AllocatorAdapter<default_allocator_t>)
		 * E) siseof(default_allocator_t)
		 *
		 * For 64 byte system (tested on MSVC)
		 * .---.----.----.----.----.---.
		 * |   | A  | B  | C  | D  | E |
		 * :---+----+----+----+----+---:
		 * | 1 | 32 | 40 | 32 |  1 | 1 |
		 * :---+----+----+----+----+---:
		 * | 2 | 40 | 48 | 40 | 12 | 1 |
		 * :---+----+----+----+----+---:
		 * | 3 | 40 | 48 | 40 |  8 | 8 |
		 * :---+----+----+----+----+---:
		 * | 4 | 48 | 56 | 48 | 16 | 8 |
		 * '---'----'----'----'----'---'
		 *
		 * For 32 byte system (tested on MSVC)
		 * .---.----.----.----.---.---.
		 * |   | A  | B  | C  | D | E |
		 * :---+----+----+----+---+---:
		 * | 1 | 16 | 20 | 16 | 1 | 1 |
		 * :---+----+----+----+---+---:
		 * | 2 | 24 | 28 | 24 | 8 | 1 |
		 * :---+----+----+----+---+---:
		 * | 3 | 20 | 24 | 20 | 4 | 4 |
		 * :---+----+----+----+---+---:
		 * | 4 | 24 | 28 | 24 | 8 | 4 |
		 * '---'----'----'----'---'---'
		 * ASCII table generator: https://ozh.github.io/ascii-tables/
		 */

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORYPROXY */
