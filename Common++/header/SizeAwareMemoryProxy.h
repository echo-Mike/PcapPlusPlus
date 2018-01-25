#ifndef PCAPPP_SIZEAWAREMEMORYPROXY
#define PCAPPP_SIZEAWAREMEMORYPROXY

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

		template < typename T, typename Allocator >
		class MemoryProxy< T, Allocator, MemoryProxyTags::SizeAwareTag > :
			public MemoryProxyInterface<T, Allocator, MemoryProxyTags::SizeAwareTag >
		{
		public:
			typedef MemoryProxyInterface<T, Allocator, MemoryProxyTags::SizeAwareTag > Base;
		protected:

			inline bool SafeToDeleteCondition() { return m_Ownership && m_Data; }

			inline bool SafeToCopyCondition() { return m_Data && m_Length > 0; }

			inline bool deallocateData()
			{
				try
				{
					if (SafeToDeleteCondition())
						m_Allocator.deallocate(m_Data);
				}
				catch (const std::exception& e)
				{
					// TODO:: add exception handler
					return false;
				}
				catch (...)
				{
					// TODO:: add exception handler
					return false;
				}
				return true;
			}

			inline void initialize()
			{
				m_Allocator.initialize();
				m_Data = nullptr;
				m_Length = 0;
				m_Ownership = false;
			}

			inline bool copyDataFrom(const MemoryProxy& other)
			{
				deallocateData();
				m_Allocator = other.m_Allocator;
				if (other.SafeToCopyCondition())
				{
					m_Data = m_Allocator.allocate(other.m_Length);
					if (this == &other)
						std::memmove(m_Data, other.m_Data, other.m_Length);
					else
						std::memcpy(m_Data, other.m_Data, other.m_Length);
					m_Ownership = true;
					m_Length = other.m_Length;
					return true;
				}
				else
				{
					m_Ownership = false;
					m_Data = nullptr;
					m_Length = 0;
					return false;
				}
			}

#ifdef ENABLE_CPP11_MOVE_SEMANTICS

			inline void moveDataFrom(MemoryProxy&& other)
			{
				deallocateData();
				m_Allocator = std::move(other.m_Allocator);
				m_Data = other.m_Data;
				m_Ownership = other.m_Ownership;
				m_Length = other.m_Length;
				other.initialize();
			}
#endif
		public:

			MemoryProxy() { initialize(); }

// In case of unsupported std::nullptr_t nullptr will be a macro def (from CPP11.h)
#ifndef nullptr

			explicit MemoryProxy(std::nullptr_t) { initialize(); }
#endif

			explicit MemoryProxy(pointer p, size length = 0, bool ownership = true) :
				m_Allocator(), m_Data(p), m_Length(length), m_Ownership(ownership) {}

			MemoryProxy(const MemoryProxy& other) { copyDataFrom(other); }

			MemoryProxy& operator=(const MemoryProxy& other)
			{
				if (this == &other)
					return *this;
				copyDataFrom(other);
				return *this;
			}

#ifdef ENABLE_CPP11_MOVE_SEMANTICS

			MemoryProxy(MemoryProxy&& other) { moveDataFrom(std::move(other)); }

			MemoryProxy& operator=(MemoryProxy&& other)
			{
				if (this == &other)
					return *this;
				moveDataFrom(std::move(other));
				return *this;
			}
#endif

			~MemoryProxy() { deallocateData(); }

			inline size getLength() const { return m_Length; }

			inline bool isOwning() const { return m_Ownership; }

			inline pointer get() { return m_Data; }

			inline const_pointer get() const { return m_Data; }

			pointer relese()
			{
				typename Base::pointer old = m_Data;
				initialize();
				return old;
			}

			bool reset(pointer ptr, size length = 0, bool ownership = true)
			{
				if (deallocateData())
				{
					m_Data = ptr;
					m_Length = length;
					m_Ownership = ownership;
					return true;
				}
				initialize();
				return false;
			}

			operator bool() const { return m_Data || m_Ownership || m_Length > 0; }

			inline typename Adapter::allocator_traits::allocator_type& getAllocator() const { return m_Allocator.getAllocator(); }

			inline void setAllocator(typename Adapter::allocator_traits::allocator_type& allocator) const { return m_Allocator.setAllocator(allocator); }

			bool reallocate(size newBufferLength)
			{
				if (newBufferLength == m_Length)
					return true;
				if (!newBufferLength)
				{
					deallocateData();
					m_Ownership = false;
					m_Data = nullptr;
					m_Length = 0;
					return true;
				}
				typename Base::pointer newBuffer = m_Allocator.allocate(newBufferLength); // It is allocator's responsibility to handle memory allocation exceptions
				if (!newBuffer) // Expect nullptr when execption thrown on allocation
					return false;
				std::memset(newBuffer, 0, newBufferLength);
				if (SafeToCopyCondition())
					std::memcpy(newBuffer, m_Data, newBufferLength < m_Length ? newBufferLength : m_Length );
				if (m_Length > 0)
				{
					if (!deallocateData())
					{
						m_Ownership = false;
						m_Data = nullptr;
						m_Length = 0;
						return false;
					}
				}
				m_Data = newBuffer;
				m_Length = newBufferLength;
				m_Ownership = true;
				return true;
			}

			bool append(size dataToAppendLen)
			{
				if (!dataToAppendLen)
					return true;
				typename Base::size oldLength = m_Length;
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				std::memset(m_Data + oldLength, 0, dataToAppendLen);
				return true;
			}

			bool append(const_pointer dataToAppend, size dataToAppendLen)
			{
				if (!(dataToAppend && dataToAppendLen))
				{
					// TODO: Add error handler
					return false;
				}
				typename Base::size oldLength = m_Length;
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				std::memmove(m_Data + oldLength, dataToAppend, dataToAppendLen);
				return true;
			}

			bool insert_back(index atIndex, size dataToInsertLen) 
			{
				typename Base::size oldLength = m_Length;

				if (m_Length < (typename Base::size)(-atIndex))
					atIndex = -((typename Base::index)m_Length);

				typename Base::size index = ((typename Base::index)oldLength) + atIndex;

				if (!reallocate(m_Length + dataToInsertLen))
					return false;

				std::memmove(m_Data + index + dataToInsertLen, m_Data + index, -atIndex);
				std::memset(m_Data + index, 0, dataToInsertLen);
				return true;
			}

			bool insert(index atIndex, size dataToInsertLen)
			{
				if (!dataToInsertLen)
					return true;

				if (!m_Length)
					return append(dataToInsertLen);

				if (atIndex < 0)
					return insert_back(atIndex, dataToInsertLen);

				if (atIndex + 1 > m_Length)
					atIndex = m_Length - 1;

				const typename Base::size oldLength = m_Length;

				if (!reallocate(m_Length + dataToInsertLen))
					return false;

				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, oldLength - atIndex);
				std::memset(m_Data + atIndex, 0, dataToInsertLen);
				return true;
			}

			bool insert_back(index atIndex, const_pointer dataToInsert, size dataToInsertLen)
			{
				typename Base::size oldLength = m_Length;

				if (m_Length < (typename Base::size)(-atIndex))
					atIndex = -((typename Base::index)m_Length);

				typename Base::size index = ((typename Base::index)oldLength) + atIndex;

				if (!reallocate(m_Length + dataToInsertLen))
					return false;

				std::memmove(m_Data + index + dataToInsertLen, m_Data + index, -atIndex);
				std::memcpy(m_Data + index, dataToInsert, dataToInsertLen);
				return true;
			}

			bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen)
			{
				if (!dataToInsertLen)
					return true;

				if (!dataToInsert )
				{
					// TODO: Add error handler
					return false;
				}

				if (!m_Length)
					return append(dataToInsert, dataToInsertLen);

				if (atIndex < 0)
					return insert_back(atIndex, dataToInsert, dataToInsertLen);

				if (atIndex + 1 > m_Length)
					atIndex = m_Length - 1;
				
				const typename Base::size oldLength = m_Length;

				if (!reallocate(m_Length + dataToInsertLen))
					return false;

				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, oldLength - atIndex);
				std::memcpy(m_Data + atIndex, dataToInsert, dataToInsertLen);
				return true;
			}

			bool remove_back(index atIndex, size numOfBytesToRemove)
			{
				typename Base::size oldLength = m_Length;

				if (m_Length < (typename Base::size)(-atIndex))
					return true;

				typename Base::size index = ((typename Base::index)m_Length) + atIndex;

				if (index + numOfBytesToRemove > m_Length)
					numOfBytesToRemove = m_Length - index;

				if (numOfBytesToRemove >= m_Length)
					return reallocate(0);

				std::memmove(m_Data + index, m_Data + index + numOfBytesToRemove, m_Length - index - numOfBytesToRemove);
				return reallocate(m_Length - numOfBytesToRemove);
			}

			bool remove(index atIndex, size numOfBytesToRemove)
			{
				if (!(numOfBytesToRemove && m_Length))
					return true;
				
				if (atIndex < 0)
					return remove_back(atIndex, numOfBytesToRemove);

				if (atIndex >= m_Length)
					return true;

				if (atIndex + (typename Base::index)numOfBytesToRemove > m_Length)
					numOfBytesToRemove = m_Length - atIndex;

				if (numOfBytesToRemove >= m_Length)
					return reallocate(0);

				std::memmove(m_Data + atIndex, m_Data + atIndex + numOfBytesToRemove, m_Length - atIndex - numOfBytesToRemove);
				return reallocate(m_Length - numOfBytesToRemove);
			}

		protected:
			mutable Adapter m_Allocator;
			pointer m_Data;
			size m_Length;
			bool m_Ownership;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_SIZEAWAREMEMORYPROXY */