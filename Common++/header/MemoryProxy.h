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
		 * Represents type that is used in memory manipulations as a type of a single memory cell.
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

		// Forward declaration.
		class DPDKMemoryProxy;

		/**
		 * @brief Helper structure that deduces type of default memory proxy from provided tag.
		 * By default deduces to DefaultOldMemoryModelMemoryProxy;
		 * @tparam MemoryProxyTag Tag for which the deduction is made.
		 */
		template < typename MemoryProxyTag = MemoryProxyTags::OldMemoryModelTag, typename = void >
		struct MemoryProxyDispatcher
		{
			typedef DefaultOldMemoryModelMemoryProxy memory_proxy_t;
		};
		/**
		 * @brief Specialization of MemoryProxyDispatcher that deduces DefaultSizeAwareMemoryProxy from SizeAwareTag.
		 */
		template <>
		struct MemoryProxyDispatcher< MemoryProxyTags::SizeAwareTag >
		{
			typedef DefaultSizeAwareMemoryProxy memory_proxy_t;
		};
		/**
		 * @brief Specialization of MemoryProxyDispatcher that deduces DefaultContentAwareMemoryProxy from ContentAwareTag.
		 */
		template <>
		struct MemoryProxyDispatcher< MemoryProxyTags::ContentAwareTag >
		{
			typedef DefaultContentAwareMemoryProxy memory_proxy_t;
		};
		/**
		 * @brief Specialization of MemoryProxyDispatcher that deduces DPDKMemoryProxy from DpdkTag.
		 */
		template <>
		struct MemoryProxyDispatcher< MemoryProxyTags::DpdkTag >
		{
			typedef DPDKMemoryProxy memory_proxy_t;
		};
		/**
		 * @brief Specialization of MemoryProxyDispatcher that deduces provided MemoryProxy type from CustomTag.
		 */
		template < typename MemoryProxy >
		struct MemoryProxyDispatcher< MemoryProxyTags::CustomTag, MemoryProxy >
		{
			typedef MemoryProxy memory_proxy_t;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORYPROXY */
