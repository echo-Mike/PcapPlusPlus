#ifndef PCAPP_MEMORYPROXY
#define PCAPP_MEMORYPROXY

#include <memory>
#include <cstring>
#include <cstdint>

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
		struct deleter
		{
			virtual void operator()(T*) = 0;
		};

		template < typename T > 
		struct default_delete :
			public deleter<T>
		{
			void operator()(T* ptr) { delete ptr; }
		};

		template < typename T >
		struct default_delete<T[]> :
			public deleter<T>
		{
			void operator()(T* ptr) { delete[] ptr; }
		};
#else

		template < typename T >
		using default_delete = std::default_delete<T>;
#endif

		template < typename T >
		struct allocator
		{
			virtual T* operator()(std::size_t) = 0;
		};

		template < typename T >
		struct default_allocator :
			public allocator<T>
		{
			T* operator()(std::size_t) { new T(); }
		};

		template < typename T >
		struct default_allocator<T[]> :
			public allocator<T>
		{
			T* operator()(std::size_t size) { new T[size]; }
		};
	}

	template < typename T >
	class MemoryProxy
	{
	public:
		typedef T value;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef std::size_t size;
		typedef int index;
	protected:

		inline void initialize() 
		{
			m_Length = 0;
			m_Ownership = false;
		}

		inline bool copyDataFrom(const MemoryProxy& other)
		{
			m_Length = other.m_Length;
			m_Ownership = other.m_Ownership;
		}

#ifdef USE_CPP11

		inline bool moveDataFrom(MemoryProxy&& other)
		{
			copyDataFrom(other);
			other.initialize();
		}
#endif
	public:

		MemoryProxy() { initialize(); }

		MemoryProxy(size length, bool ownership = true) :
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

		virtual ~MemoryProxy() {}

		inline size getLength() const { return m_Length; }

		inline bool isOwning() const { return m_Ownership; }

		virtual pointer get() const = 0;

		virtual pointer relese() = 0;

		virtual void reset(pointer ptr) = 0;

		virtual operator bool() const = 0;

		inline isInNullState() const { return !(this->operator bool()); }

		virtual bool reallocate(size newBufferLength) = 0;

		virtual bool append(const_pointer dataToAppend, size dataToAppendLen) = 0;

		virtual bool insert(index atIndex, size dataToInsertLen) = 0;

		virtual bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen) = 0;

		virtual bool remove(index atIndex, size numOfBytesToRemove) = 0;

	protected:
		size m_Length;
		bool m_Ownership;
	};

//#ifdef USE_CPP11

	template < typename T, 
		typename Allocator = memory::default_allocator<T[]>, 
		typename Deleter = memory::default_delete<T[]> >
	class CPP11MemoryProxy :
		public MemoryProxy<T>
	{
	public:
		typedef Deleter deleter;
		typedef Allocator allocator;
		typedef MemoryProxy<T> Base;
	private:
		typedef std::unique_ptr<T[], Deleter> SmartPtr;

		inline void initialize()
		{
			Base::initialize();
			m_Allocator = allocator();
			m_Data = SmartPtr(nullptr);
		}

		bool copyDataFrom(const MemoryProxy& other);

		bool moveDataFrom(MemoryProxy&& other);

	public:

		CPP11MemoryProxy() { initialize(); }

		CPP11MemoryProxy(std::nullptr_t) { initialize(); }

		CPP11MemoryProxy(pointer p, size length = 0, bool ownership = true);

		CPP11MemoryProxy(const CPP11MemoryProxy& other);

		CPP11MemoryProxy& operator=(const CPP11MemoryProxy& other);

		CPP11MemoryProxy(CPP11MemoryProxy&& other);

		CPP11MemoryProxy& operator=(CPP11MemoryProxy&& other);

		~CPP11MemoryProxy();

		pointer get() const;

		pointer relese();

		void reset(pointer ptr);

		operator bool() const;

		bool reallocate(size newBufferLength);

		bool append(const_pointer dataToAppend, size dataToAppendLen);

		bool insert(index atIndex, size dataToInsertLen);

		bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen);

		bool remove(index atIndex, size numOfBytesToRemove);

	private:
		allocator m_Allocator;
		SmartPtr m_Data;
	};

	//typedef CPP11MemoryProxy<uint8_t> pcpp_MemoryProxy;
//#else

	template < typename T, 
		typename Allocator = memory::default_allocator<T[]>, 
		typename Deleter = memory::default_delete<T[]> >
	class CPP99MemoryProxy :
		public MemoryProxy<T>
	{
	public:
		typedef Deleter deleter;
		typedef Allocator allocator;
		typedef MemoryProxy<T> Base;
	private:

		inline void initialize()
		{
			Base::initialize();
			m_Allocator = new allocator();
			m_Deleter = new deleter();
			m_Data = NULL;
		}

		bool copyDataFrom(const MemoryProxy& other);
	public:

		CPP99MemoryProxy() { initialize(); }

		CPP99MemoryProxy(pointer p, size = 0, bool ownership = true);

		CPP99MemoryProxy(const CPP99MemoryProxy& other);

		CPP99MemoryProxy& operator=(const CPP99MemoryProxy& other);

		~CPP99MemoryProxy();

		pointer get() const;

		pointer relese();

		void reset(pointer ptr);

		operator bool() const;

		bool reallocate(size newBufferLength);

		bool append(const_pointer dataToAppend, size dataToAppendLen);

		bool insert(index atIndex, size dataToInsertLen);

		bool insert(index atIndex, const_pointer dataToInsert, size dataToInsertLen);

		bool remove(index atIndex, size numOfBytesToRemove);

	private:
		allocator m_Allocator;
		deleter m_Deleter;
		pointer m_Data;
	};

	//typedef CPP99MemoryProxy<uint8_t> pcpp_MemoryProxy;
//#endif

} // namespace pcpp

#endif /* PCAPP_MEMORYPROXY */
