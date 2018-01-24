#ifndef PCAPP_MEMORYPROXY
#define PCAPP_MEMORYPROXY

#include <memory>
#include <cstring>
#include <cstdint>
#include <exception>

/// @file

/**
* \namespace pcpp
* \brief The main namespace for the PcapPlusPlus lib
*/
namespace pcpp
{
#define USE_CPP11
	namespace memory
	{

		#ifndef USE_CPP11
			#ifndef nullptr
				#define nullptr NULL
			#endif
		#endif

		template < typename T >
		struct allocator
		{
			typedef T value_type;
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef T& reference;
			typedef const T& const_reference;

			virtual T* allocate(std::size_t) = 0;

			virtual void deallocate(pointer) = 0;
		};

		template < typename T >
		struct default_allocator :
			public allocator<T>
		{
			typedef allocator<T> BaseAllocator;

			T* allocate(std::size_t) { return new T(); }

			void deallocate(pointer p) { delete p; }

		};

		template < typename T >
		struct default_allocator<T[]> :
			public allocator<T>
		{
			typedef allocator<T> BaseAllocator;

			T* allocate(std::size_t size) { return new T[size]; }

			void deallocate(pointer p) { delete[] p; }
		};

		template < typename Allocator >
		struct allocator_traits
		{
			typedef Allocator allocator_type;
			typedef typename Allocator::BaseAllocator base_allocator;
			typedef typename Allocator::value_type  value_type;
			typedef typename Allocator::pointer pointer;
			typedef typename Allocator::const_pointer const_pointer;
			typedef typename Allocator::reference reference;
			typedef typename Allocator::const_reference const_reference;
		};

		template < typename Allocator, typename traits = allocator_traits<Allocator> >
		typename traits::allocator_type& staticAllocator()
		{
			static typename traits::allocator_type allocator;
			return allocator;
		}

		template < typename Allocator,
			typename traits = allocator_traits<Allocator>,
			typename traits::allocator_type& (*GetAllocator)() = &staticAllocator<Allocator>,
			typename traits::allocator_type::pointer (traits::allocator_type::* Allocate)(std::size_t) = &traits::allocator_type::allocate,
			void (traits::allocator_type::* Deallocate)(typename traits::pointer) = &traits::allocator_type::deallocate >
		struct AllocatorAdapter
		{
			typedef traits allocator_traits;

			AllocatorAdapter() : m_Allocator(GetAllocator()) {}

			AllocatorAdapter(const AllocatorAdapter& other) { m_Allocator = other.m_Allocator; }

			AllocatorAdapter& operator=(const AllocatorAdapter& other) 
			{ 
				if (this == &other)
					return *this;
				m_Allocator = other.m_Allocator;
				return *this;
			}

#ifdef USE_CPP11

			AllocatorAdapter(AllocatorAdapter&& other) { m_Allocator = std::move(other.m_Allocator); }

			AllocatorAdapter& operator=(AllocatorAdapter&& other)
			{
				if (this == &other)
					return *this;
				m_Allocator = std::move(other.m_Allocator);
				return *this;
			}
#endif

			virtual ~AllocatorAdapter() {};

			inline typename traits::allocator_type& getAllocator() const { return m_Allocator; }

			inline void setAllocator(typename traits::allocator_type& allocator) const { m_Allocator = allocator; }

			inline typename traits::pointer allocate(std::size_t length = 1) const { (m_Allocator.*Allocate)(length); }

			inline void deallocate(typename traits::pointer p) const { (m_Allocator.*Deallocate)(p); }

			inline void initialize() { m_Allocator = GetAllocator(); }

		protected:
			mutable typename traits::allocator_type m_Allocator;
		};

		template < typename T, typename Allocator, typename ProxyTag,
			typename traits = allocator_traits<Allocator> >
		class MemoryProxyInterface 
		{
		public:
			typedef typename traits::value_type value_type;
			typedef typename traits::pointer pointer;
			typedef typename traits::const_pointer const_pointer;
			typedef typename traits::reference reference;
			typedef typename traits::const_reference const_reference;

			typedef AllocatorAdapter<Allocator> Adapter;

			typedef std::size_t size;
			typedef int index;
			
			typedef ProxyTag tag_type;

			virtual ~MemoryProxyInterface() {};

			inline virtual size getLength() const = 0;

			inline virtual bool isOwning() const = 0;

			inline virtual pointer get() = 0;

			inline virtual const_pointer get() const = 0;

			virtual pointer relese() = 0;

			virtual bool reset(pointer ptr, size length = 0, bool ownership = true) = 0;

			virtual operator bool() const = 0;

			inline isInNullState() const { return !(this->operator bool()); }

			inline virtual typename traits::allocator_type& getAllocator() const = 0;

			inline virtual void setAllocator(typename traits::allocator_type& allocator) const = 0;

			virtual bool reallocate(size newBufferLength) = 0;

			virtual bool append(size dataToAppendLen) = 0;

			virtual bool append(const_pointer dataToAppend, size dataToAppendLen) = 0;

			virtual bool insert(index atIndex, size dataToInsertLen) = 0;

			virtual bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen) = 0;

			virtual bool remove(index atIndex, size numOfBytesToRemove) = 0;
		};
		
		namespace MemoryProxyTags
		{
			struct SizeAwareTag {};
			struct SizeUnawareTag {};
			struct ContentAwareTag {};
			struct OldMemoryModelTag {};
			struct SafeOldMemoryModelTag {};
		}
		
		template < typename T, typename Allocator, typename ProxyTag >
		class MemoryProxy { private: MemoryProxy() {} ~MemoryProxy() {} }; // Object of class generated by this template can't be created.

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

#ifdef USE_CPP11

			inline void moveDataFrom(MemoryProxy&& other)
			{
				deallocateData();
				m_Allocator = std::move(other.m_Allocator));
				m_Data = other.m_Data;
				m_Ownership = other.m_Ownership;
				m_Length = other.m_Length;
				other.initialize();
			}
#endif
		public:

			MemoryProxy() { initialize(); }

#ifdef USE_CPP11

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

#ifdef USE_CPP11

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
				if (SafeToCopyDataCondition())
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
				m_Owning = true;
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

				if (!reallocate(m_Length + dataToInsertLen))
					return false;

				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, m_Data - atIndex);
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

				if (!reallocate(m_Length + dataToInsertLen))
					return false;

				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, m_Data - atIndex);
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

		/****

		template < typename T, typename Allocator >
		class MemoryProxy< T, Allocator, MemoryProxyTags::SizeUnawareTag > :
			public MemoryProxyInterface<T, Allocator, MemoryProxyTags::SizeUnawareTag >
		{
		public:
			typedef MemoryProxyInterface<T, Allocator, MemoryProxyTags::SizeUnawareTag > Base;
		protected:

		public:

		protected:
			mutable Adapter m_Allocator;
			pointer m_Data;
			size m_Length;
			bool m_Ownership;
		};

		****/

		template < typename T, typename Allocator >
		class MemoryProxy< T, Allocator, MemoryProxyTags::ContentAwareTag > :
			public MemoryProxyInterface<T, Allocator, MemoryProxyTags::ContentAwareTag >
		{
		public:
			typedef MemoryProxyInterface<T, Allocator, MemoryProxyTags::ContentAwareTag > Base;
		protected:

			inline bool SafeToDeleteCondition() { return m_Ownership && m_Data; }

			inline bool SafeToCopyCondition() { return m_Data && m_Capacity > 0; }

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

			inline void clear() 
			{
				// Set all fields to their initial values
				m_Data = nullptr;
				m_Length = 0;
				m_Capacity = 0;
				m_Ownership = false;
			}

			inline void initialize()
			{
				m_Allocator.initialize();
				clear();
			}

			inline bool copyDataFrom(const MemoryProxy& other)
			{
				// Deallocate current data
				deallocateData();
				// Copy allocator of other object
				m_Allocator = other.m_Allocator;
				// Check that copying of data is safe
				if (other.SafeToCopyCondition())
				{
					// Allocate new data
					// It is allocator's responsibility to handle memory allocation exceptions
					m_Data = m_Allocator.allocate(other.m_Capacity);
					// Check if new buffer was allocated
					if (!m_Data) // Expect nullptr/NULL returned when execption thrown on allocation
						return false;
					// Copy whole data to this object
					std::memcpy(m_Data, other.m_Data, other.m_Capacity * sizeof(typename Base::value_type));
					// We definitely own data now
					m_Ownership = true;
					// Copy length and capacity values
					m_Length = other.m_Length;
					m_Capacity = other.m_Capacity;
					return true;
				}
				else // Other object must be in some null-state
				{	// Put our object in proper null-state
					clear();
					return false;
				}
			}

#ifdef USE_CPP11

			inline void moveDataFrom(MemoryProxy&& other)
			{
				// Deallocate current data
				deallocateData();
				// Move allocator from other
				m_Allocator = std::move(other.m_Allocator));
				// Copy other data members
				m_Data = other.m_Data;
				m_Ownership = other.m_Ownership;
				m_Length = other.m_Length;
				m_Capacity = other.m_Capacity;
				// Set other object to null-state
				other.initialize();
			}
#endif
		public:

			MemoryProxy() { initialize(); }

#ifdef USE_CPP11

			explicit MemoryProxy(std::nullptr_t) { initialize(); }
#endif

			explicit MemoryProxy(pointer p, size length = 0, bool ownership = true) :
				m_Allocator(), m_Data(p), m_Length(length), m_Capacity(length), m_Ownership(ownership) {}

			MemoryProxy(const MemoryProxy& other) { copyDataFrom(other); }

			MemoryProxy& operator=(const MemoryProxy& other)
			{
				// Handle self assignment case
				if (this == &other)
					return *this;
				copyDataFrom(other);
				return *this;
			}

#ifdef USE_CPP11

			MemoryProxy(MemoryProxy&& other) { moveDataFrom(std::move(other)); }

			MemoryProxy& operator=(MemoryProxy&& other)
			{
				// Handle self assignment case
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
					m_Capacity = length;
					m_Ownership = ownership;
					return true;
				}
				initialize();
				return false;
			}

			operator bool() const { return m_Data || m_Ownership || m_Length > 0 || m_Capacity > 0; }

			inline typename Adapter::allocator_traits::allocator_type& getAllocator() const { return m_Allocator.getAllocator(); }

			inline void setAllocator(typename Adapter::allocator_traits::allocator_type& allocator) const { return m_Allocator.setAllocator(allocator); }

			bool reallocate(size newBufferLength)
			{
				// Immediately return if there is enough memory
				if (newBufferLength <= m_Capacity)
					return true;
				// Provided zero length is interpreted as clear operation
				if (!newBufferLength)
				{
					deallocateData();
					clear(); 
					return true;
				}
				// Allocate new buffer of requested 
				// It is allocator's responsibility to handle memory allocation exceptions
				typename Base::pointer newBuffer = m_Allocator.allocate(newBufferLength); 
				// Check if new buffer was allocated
				if (!newBuffer) // Expect nullptr/NULL returned when execption thrown on allocation
					return false;
				// Clear new buffer
				std::memset(newBuffer, 0, newBufferLength);
				// Copy data from previous storage
				// If new storage length is smaller than previous -> copy only data that fits new storage size
				// else -> copy only old data
				if (SafeToCopyDataCondition())
					std::memcpy(newBuffer, m_Data, (newBufferLength < m_Length ? newBufferLength : m_Length) * sizeof(typename Base::value_type) );
				// Deallocate old data
				if (!deallocateData())
				{
					clear(); // Clear object data
					m_Allocator.deallocate(newBuffer); // Avoid memory leak
					return false;
				}
				// Set new data as current handled
				m_Data = newBuffer;
				// If buffer was reallocated to be smaller then data length must be corrected
				if (newBufferLength < m_Length)
					m_Length = newBufferLength;
				// New capacity is set to the actual size of memory stored 
				m_Capacity = newBufferLength;
				// We definitely own data now
				m_Owning = true;
				return true;
			}

			bool append(size dataToAppendLen)
			{
				// Append of 0 bytes is always a success
				if (!dataToAppendLen)
					return true;
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				// Set new data space to zero
				std::memset(m_Data + m_Length, 0, dataToAppendLen);
				// Increase current data length by new data length
				m_Length += dataToAppendLen;
				return true;
			}

			bool append(const_pointer dataToAppend, size dataToAppendLen)
			{
				// Append of 0 bytes from any memory is a success
				if (!dataToAppendLen)
					return true;
				// If memory to append is not presented it is an error
				if (!dataToAppend)
				{
					// TODO: Add error msg
					return false;
				}
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToAppendLen))
					return false;
				// Copy memory from source data
				// dataToAppend may (if reallocate call does not allocate) point to some byte in current data so use std::memmove
				// m_Data + m_Length is points to where tha old data past-the-end byte is
				std::memmove(m_Data + m_Length, dataToAppend, dataToAppendLen);
				// Increase current data length by new data length
				m_Length += dataToAppendLen;
				return true;
			}

			bool insert_back(index atIndex, size dataToInsertLen) 
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "insert before last byte of current data"
				// and -m_Length to "insert before first byte of current data" (a.e. at the begining)
				if (m_Length < (typename Base::size)(-atIndex))
					atIndex = -((typename Base::index)m_Length);
				// Compute normal index position that correspond to negative atIndex value
				typename Base::index index = ((typename Base::index)m_Length) + atIndex;
				// Here index must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(m_Data + index + dataToInsertLen, m_Data + index, -atIndex);
				// Fill new data with zeros
				std::memset(m_Data + index, 0, dataToInsertLen);
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}

			bool insert(index atIndex, size dataToInsertLen)
			{
				// Inserting 0 bytes is always a success
				if (!dataToInsertLen)
					return true;
				// If object has no data insert is equal to append 
				if (!m_Length)
					return append(dataToInsertLen);
				// Handle cases when insertion must start from the back
				if (atIndex < 0)
					return insert_back(atIndex, dataToInsertLen);
				// At this point atIndex must be in bound [0, m_Length]
				// where 0 correspond to "insert before fisrt byte of current data"
				// and m_Length to "insert after last byte of current data" (a.e. append)
				if ((typename Base::size)atIndex >= m_Length)
					return append(dataToInsertLen);
				// Here atIndex must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, m_Length - atIndex);
				// Fill new data with zeros
				std::memset(m_Data + atIndex, 0, dataToInsertLen);
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}

			bool insert_back(index atIndex, const_pointer dataToInsert, size dataToInsertLen)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "insert before last byte of current data"
				// and -m_Length to "insert before first byte of current data" (a.e. at the begining)
				if (m_Length < (typename Base::size)(-atIndex))
					atIndex = -((typename Base::index)m_Length);
				// Compute normal index position that correspond to negative atIndex value
				typename Base::index index = ((typename Base::index)m_Length) + atIndex;
				// Here index must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(m_Data + index + dataToInsertLen, m_Data + index, -atIndex);
				// Insert new data to it's place
				std::memcpy(m_Data + index, dataToInsert, dataToInsertLen);
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}

			bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen)
			{
				// Inserting 0 bytes from any memory is always a success
				if (!dataToInsertLen)
					return true;
				// If memory to insert is not presented it is an error
				if (!dataToInsert)
				{
					// TODO: Add error msg
					return false;
				}
				// If object has no data insert is equal to append 
				if (!m_Length)
					return append(dataToInsert, dataToInsertLen);
				// Handle cases when insertion must start from the back
				if (atIndex < 0)
					return insert_back(atIndex, dataToInsert, dataToInsertLen);
				// At this point atIndex must be in bound [0, m_Length]
				// where 0 correspond to "insert before fisrt byte of current data"
				// and m_Length to "insert after last byte of current data" (a.e. append)
				if ((typename Base::size)atIndex >= m_Length)
					return append(dataToInsert, dataToInsertLen);
				// Here atIndex must be in bound [0, m_Length)
				// Ensure that we have enough place to hold new data
				if (!reallocate(m_Length + dataToInsertLen))
					return false;
				// Move old data from where new data will be inserted to the end of memory 
				std::memmove(m_Data + atIndex + dataToInsertLen, m_Data + atIndex, m_Length - atIndex);
				// Insert new data to it's place
				std::memcpy(m_Data + atIndex, dataToInsert, dataToInsertLen);
				// Set new data length
				m_Length += dataToInsertLen;
				return true;
			}

			bool remove_back(index atIndex, size numOfBytesToRemove)
			{
				// Index at this point must be in bound [-m_Length, -1]
				// where -1 correspond to "clear last byte of data"
				// and -m_Length to "clear from begining of data"
				// Don't handle the cases when atIndex is in (-Inf,-m_Length)
				if (m_Length < (typename Base::size)(-atIndex))
					return true;
				// Compute normal index position that correspond to negative atIndex value
				typename Base::index index = ((typename Base::index)m_Length) + atIndex;
				// Here index must be in bound [0, m_Length)
				// End of data to clear must be in (atIndex, m_Length]
				if ((typename Base::size)index + numOfBytesToRemove > m_Length) // There is no tail memory to move
				{	// Just shrink current length to remaining data
					m_Length = index;
				}
				else // There is tail memory to move
				{
					// Move tail memory to new place
					std::memmove(m_Data + index, m_Data + index + numOfBytesToRemove, m_Length - index - numOfBytesToRemove);
					m_Length -= numOfBytesToRemove;
				}
				return true;
			}

			bool remove(index atIndex, size numOfBytesToRemove)
			{
				// Removing zero bytes or removing from empty data is always a success
				if (!(numOfBytesToRemove && m_Length))
					return true;
				// Handle cases when removal must start from the back
				if (atIndex < 0)
					return remove_back(atIndex, numOfBytesToRemove);
				// At this point atIndex must be in bound [0, m_Length)
				// where 0 correspond to "clear from begining of data"
				// and m_Length-1 to "clear last byte of data"
				if (atIndex >= m_Length)
					return true;
				// End of data to clear must be in (atIndex, m_Length]
				if ((typename Base::size)atIndex + numOfBytesToRemove > m_Length) // There is no tail memory to move
				{	// Just shrink current length to remaining data
					m_Length = atIndex;
				}
				else // There is tail memory to move
				{
					// Move tail memory to new place
					std::memmove(m_Data + atIndex, m_Data + atIndex + numOfBytesToRemove, m_Length - atIndex - numOfBytesToRemove);
					m_Length -= numOfBytesToRemove;
				}
				return true;
			}

		protected:
			mutable Adapter m_Allocator;
			pointer m_Data;
			size m_Length, m_Capacity;
			bool m_Ownership;
		};

		template < typename T, typename Allocator >
		class MemoryProxy< T, Allocator, MemoryProxyTags::OldMemoryModelTag > :
			public MemoryProxyInterface<T, Allocator, MemoryProxyTags::OldMemoryModelTag >
		{
		public:
			typedef MemoryProxyInterface<T, Allocator, MemoryProxyTags::OldMemoryModelTag > Base;
		protected:

		public:

		protected:
			mutable Adapter m_Allocator;
			pointer m_Data;
			size m_Length;
			bool m_Ownership;
		};

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPP_MEMORYPROXY */
