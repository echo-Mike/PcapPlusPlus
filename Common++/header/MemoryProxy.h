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

		template < typename T > 
		struct default_delete
		{
			void operator()(T* ptr) { delete ptr; }
		};

		template < typename T >
		struct default_delete<T[]>
		{
			void operator()(T* ptr) { delete[] ptr; }
		};

		#ifndef nullptr
			#define nullptr NULL
		#endif
#else

		template < typename T >
		using default_delete = std::default_delete<T>;
#endif

		template < typename T >
		struct default_allocator
		{
			T* operator()() { new T(); }
		};

		template < typename T >
		struct default_allocator<T[]> :
			public allocator<T>
		{
			T* operator()(std::size_t size) { new T[size]; }
		};
	}

	template < typename T = uint8_t,
		typename Allocator = memory::default_allocator<T[]>,
		typename Deleter = memory::default_delete<T[]>,
		T* (Allocator::* Allocate)(std::size_t) = &Allocator::operator(),
		void (Deleter::* Deallocate)(T*) = &Deleter::operator() >
	class MemoryProxy
	{
	public:
		typedef T value;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef std::size_t size;
		typedef int index;
		typedef Deleter deleter;
		typedef Allocator allocator;
	protected:

		inline bool SafeToDeleteCondition() { return m_Ownership && m_Data; }

		inline bool SafeToCopyCondition() { return m_Data && m_Length > 0; }

		inline bool deallocateData()
		{
			try
			{
				if (SafeToDeleteCondition())
					(m_Deleter.*Deallocate)(m_Data);
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
			m_Allocator = allocator();
			m_Deleter = deleter();
			m_Data = nullptr;
			m_Length = 0;
			m_Ownership = false;
		}

		inline bool copyDataFrom(const MemoryProxy& other)
		{
			deallocateData();
			m_Deleter = other.m_Deleter;
			m_Allocator = other.m_Allocator;
			if (other.SafeToCopyCondition())
			{
				m_Data = (m_Allocator.*Allocate)(other.m_Length);
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
			m_Deleter = std::move(other.m_Deleter);
			m_Allocator = std::move(other.m_Allocator);
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
			m_Allocator(), m_Deleter(), m_Data(p),
			m_Length(length), m_Ownership(ownership) {}

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

		inline allocator getAllocator() const { return m_Allocator; }

		inline void setAllocator(allocator& alloc) const { m_Allocator = alloc; }

		inline allocator getDeleter() const { return m_Deleter; }

		inline void setDeleter(deleter& del) const { m_Deleter = del; }

		pointer relese()
		{
			pointer old = m_Data;
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

		inline isInNullState() const { return !(*this); }

		bool reallocate(size newBufferLength) 
		{
			
		}

		bool append(const_pointer dataToAppend, size dataToAppendLen)
		{

		}

		bool insert(index atIndex, size dataToInsertLen)
		{

		}

		bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen)
		{

		}

		bool remove(index atIndex, size numOfBytesToRemove)
		{

		}

	protected:
		mutable allocator m_Allocator;
		mutable deleter m_Deleter;
		pointer m_Data;
		size m_Length;
		bool m_Ownership;
	};

} // namespace pcpp

#endif /* PCAPP_MEMORYPROXY */
