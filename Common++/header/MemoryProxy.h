#ifndef PCAPPP_MEMORYPROXY
#define PCAPPP_MEMORYPROXY

/**
 * @page memory_predef_mem_system_state Predefined memory system state
 * @tableofcontents
 * NOTE: Predefined memory system state.
 * @section intro "Introduction"
 * The defines found in MemoryProxy.h describes the memory system predefined state.
 * @subsection def "Default state"
 * The default is that PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER is undefined and PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR is defined
 * @section details "Details"
 * @subsection pcappp_virtual_allocator_adapter "PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER macro"
 * If PCAPPP_VIRTUAL_ALLOCATOR_ADAPTER is defined then the class pcpp::memory::AllocatorAdapter has virtual specifier for it's functions
 * and may be a proper base interface class. This is only needed if you want to careate your own adapters for allocators based on AllocatorAdapter interface.
 * The cons of having the virtual interface is that size of AllocatorAdapter would be the same as a size of pointer on your platform or higher.
 * We are heavily rely on the size of this class because it is a member of RawPacket class around which a whole library constructed and it's size
 * is significant for efficiency.
 * @subsection pcappp_non_virtual_def_allocator "PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR macro"
 * If PCAPPP_NON_VIRTUAL_DEF_ALLOCATOR is defined then the allocator class has no virtual methods (and it is not an abstract class).
 * It's size is dropped from a size of pointer on your platform to 1 byte (or less). And therefore the size of default_allocator.
 * We are heavily rely on the size of this class because it is a member of AllocatorAdapter class by default.
 * 
 * For size comparison refer to @ref memory_size_comparison_for_default_memory_proxy "Size comparison" note.
 */

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

		/**
		 * @page memory_size_comparison_for_default_memory_proxy Size comparison for default MemoryProxy classes
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
		 *
		 * Test results for other compilers are welcome. Just add an issue to https://github.com/seladb/PcapPlusPlus
		 *
		 * ASCII table generator: https://ozh.github.io/ascii-tables/
		 */

	} // namespace pcpp::memory
	/* Put / before /* to uncomment (//*)
	namespace sizeof_wrapper
	{
		// The VS have build-in support of constexpr expression evaluation.
		// Just uncomment this lines and put your mouse over the letter to see the size.
		// This feature is supported in VS2015 and above versions.
		// For newer compilers "constexpr auto" type will work better.
		const unsigned long long DefaultSizeAwareMemoryProxy_Size = sizeof(memory::DefaultSizeAwareMemoryProxy); // A case
		const unsigned long long DefaultContentAwareMemoryProxy_Size = sizeof(memory::DefaultContentAwareMemoryProxy); // B case
		const unsigned long long DefaultOldMemoryModelMemoryProxy_Size = sizeof(memory::DefaultOldMemoryModelMemoryProxy); // C case
		const unsigned long long AllocatorAdapter_default_allocator_t_Size = sizeof(memory::AllocatorAdapter<memory::default_allocator_t>); // D case
		const unsigned long long default_allocator_t_Size = sizeof(memory::default_allocator_t); // E case
	}
	//*/
} // namespace pcpp

#endif /* PCAPPP_MEMORYPROXY */
