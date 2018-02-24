#ifndef PCAPPP_ALLOCATOR_POINTER_PAIR
#define PCAPPP_ALLOCATOR_POINTER_PAIR

#include <cstdlib>

#include "CPP11.h"
#include "TypeUtils.h"
#include "MemoryUtils.h"
#include "MoveSemantics.h"
#include "CompressedPair.h"
#include "AllocatorAdapter.h"

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
		 * \namespace Implementation
		 * \brief Special namespace that wraps up an implementation details of some classes from pcpp::memory namespace.
		 */
		namespace Implementation
		{
			/**
			 * @class AllocatorPointerPair
			 * @brief This is a helper class that is manages compressed pair of allocator and pointer. 
			 * @tparam[in] Allocator Represents memory allocator that must satisfy pcpp::memory::allocator_traits.
			 * @tparam[in] traits Defines a set of requirements for Allocator type (Default: pcpp::memory::allocator_traits<Allocator>)
			 */ 
			template < typename Allocator, typename traits = allocator_traits<Allocator> >
			struct AllocatorPointerPair
			{
				/**
				 * Defines type of allocator adapter.
				 */
				typedef AllocatorAdapter<Allocator> Adapter;
				/**
				 * Defines type of pointer to memory.
				 */
				typedef typename traits::pointer pointer;
				/**
				 * Defines type of pair that stores allocator and pointer compressed if it is possible.
				 */
				typedef CompressedPair<Adapter, typename traits::pointer> allocator_pointer_pair_t;

				/**
				 * @brief Default constructor.
				 * Value-initializes the stored pointer and the stored allocator. 
				 * Requires that Allocator is DefaultConstructible.
				 */
				AllocatorPointerPair() : m_Pair(Adapter(), pointer()) {}

	#ifdef PCAPPP_HAVE_NULLPTR_T
				/**
				 * @brief Special case constructor for nullptr.
				 * On platforms where nullptr keyword is supported this constructor overrides next one if nullptr is explicitly provided.
				 */
				AllocatorPointerPair(std::nullptr_t) : m_Pair(Adapter(), pointer()) {}
	#endif
				/**
				 * @brief Main constructor.
				 * Constructs a MemoryProxyAllocatorHelper initializing the stored pointer with p 
				 * and value-initializing the stored allocator. 
				 * Requires that Allocator is DefaultConstructible.
				 * @param p Pointer to memory.
				 */
				explicit AllocatorPointerPair(pointer p) : m_Pair(Adapter(), p) {}

				/**
				 * @brief Copy constructor.
				 * @param other Instance to be copied.
				 */
				AllocatorPointerPair(const AllocatorPointerPair& other) :
					m_Pair(other.m_Pair) {}

				/**
				 * @brief Copy assignment operator.
				 * @param other Instance to be copied.
				 * @return Reference to this object.
				 */
				AllocatorPointerPair& operator=(const AllocatorPointerPair& other)
				{
					// Handle self assignment case
					if (this == &other)
						return *this;
					m_Pair = other.m_Pair;
					return *this;
				}

				/**
				 * @brief Move constructor.
				 * This is the move constructor which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
				 * @param other Instance to be moved from.
				 */
				PCAPPP_MOVE_CONSTRUCTOR(AllocatorPointerPair) :
					m_Pair(PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Pair))
				{
					// Nullify provided object
					PCAPPP_MOVE_OTHER.m_Pair.get_first() = Adapter();
					PCAPPP_MOVE_OTHER.m_Pair.get_second() = pointer();
				}

				/**
				 * @brief Move assignment operator.
				 * This is the move assignment operator which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
				 * @param other Instance to be moved from.
				 * @return Reference to this object.
				 */
				PCAPPP_MOVE_ASSIGNMENT(AllocatorPointerPair)
				{
					// Handle self assignment case
					if (this == &PCAPPP_MOVE_OTHER)
						return *this;
					m_Pair = PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Pair);
					// Nullify provided object
					PCAPPP_MOVE_OTHER.m_Pair.get_first() = Adapter();
					PCAPPP_MOVE_OTHER.m_Pair.get_second() = pointer();
					return *this;
				}

				/**
				 * @brief Method to access the stored allocator object.
				 * @return Reference to the allocator object.
				 */
				inline Adapter& get_allocator() { return (m_Pair.get_first()); }
				/**
				 * @brief Method to access the stored allocator object.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the allocator object.
				 */
				inline const Adapter& get_allocator() const { return (m_Pair.get_first()); }

				/**
				 * @brief Method to access the stored pointer.
				 * @return Reference to the pointer.
				 */
				inline typename traits::pointer& get_pointer() { return (m_Pair.get_second()); }
				/**
				 * @brief Method to access the stored pointer.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the pointer.
				 */
				inline const typename traits::pointer& get_pointer() const { return (m_Pair.get_second()); }
			protected:
				allocator_pointer_pair_t m_Pair;    //<! Pair of allocator and pointer.
			};

		} // namespace pcpp::memory::Implementation

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_ALLOCATOR_POINTER_PAIR */