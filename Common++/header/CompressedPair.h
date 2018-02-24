#ifndef PCAPPP_MEMORY_IMPLEMENTATION
#define PCAPPP_MEMORY_IMPLEMENTATION

#include <memory>
#include <cstdlib>

#include "CPP11.h"
#include "TypeUtils.h"
#include "MemoryUtils.h"
#include "MoveSemantics.h"

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
			 * @brief Specialization of ComplessedPair for any empty type as it's first argument and some type as second.
			 * The compression magic is happening here.
			 * PRINCIPLE:\n
			 * The basic principle is that deriving from empty (no value members only function members) base class is costless, 
			 * but storing same object as a value member costs one byte plus padding and alignment.
			 * This types of objects are commonly a function objects.\n
			 * For this template the requirement is applied to T2 - it must be copy constructible.
			 * @tparam T1 The type to be derived from.
			 * @tparam T2 The type to be stored.
			 */
			template <
				typename T1,
				typename T2,
				bool = type_traits::is_empty<T1>::value
			>
			class CompressedPair :
				private T1 // private -> Don't add any names to namespace of this class
			{
			private:
				/**
				 * Type alias for base class.
				 */
				typedef T1 Base;

				T2 m_Val2;
			public:
				/**
				 * Type of the first pair member.
				 */
				typedef T1 first_t;
				/**
				 * Type of the second pair member.
				 */
				typedef T2 second_t;

				/**
				 * @brief Main constructor of this class.
				 * By default empty objects have great default constructors.
				 * The signature of this function must be the same as for other specialization, 
				 * except base class must be provided by value to not fill the stack (sizeof Base == 1 Byte, sizeof Base& == 4/8 Byte).
				 * @param val2 The value to be provide to the copy constructor of stored value.
				 */
				CompressedPair(const Base, const T2& val2) :
					Base(), m_Val2(val2) {}

				/**
				 * @brief Copy constructor.
				 * @param[in] other The instance to make copy of.
				 */
				CompressedPair(const CompressedPair& other) :
					m_Val2(other.m_Val2) {}

				/**
				 * @brief Copy assignment operator.
				 * Don't allows self assignment.\n
				 * @param[in] other The instance to make copy of.
				 */
				CompressedPair& operator=(const CompressedPair& other)
				{
					// Handle self assignment case
					if (this == &other)
						return *this;
					m_Val2 = other.m_Val2;
					return *this;
				}

				/**
				 * @brief Move constructor.
				 * This is the move constructor which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
				 * @param other Instance to be moved from.
				 */
				PCAPPP_MOVE_CONSTRUCTOR(CompressedPair) :
					m_Val2(PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Val2)) {}

				/**
				 * @brief Move assignment operator.
				 * This is the move assignment operator which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
				 * @param other Instance to be moved from.
				 * @return Reference to this object.
				 */
				PCAPPP_MOVE_ASSIGNMENT(CompressedPair)
				{
					// Handle self assignment case
					if (this == &PCAPPP_MOVE_OTHER)
						return *this;
					m_Val2 = PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Val2);
					return *this;
				}

				/**
				 * @brief Method to access the compressed base value.
				 * @return Reference to the first stored type.
				 */
				Base& get_first() { return (*this); }
				/**
				 * @brief Method to access the compressed base value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the first stored type.
				 */
				const Base& get_first() const { return (*this); }
				/**
				 * @brief Method to access the actually stored value.
				 * @return Reference to the second stored value.
				 */
				T2& get_second() { return (m_Val2); }
				/**
				 * @brief Method to access the actually stored value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the const-qualified second stored value.
				 */
				const T2& get_second() const { return (m_Val2); }
			};

			/**
			 * @brief Specialization of ComplessedPair for two non-empty types.
			 * This template actually have nothing to do with the compression of provided types.
			 * It is simply stores them sequentially one after another.
			 * This template is a fallback for all provided non-empty types.
			 * This technique has a more general approach with variadic templates or variadic macros. 
			 * But the clearer way - variadic templates is not part of C++98 standard,
			 * so we decided to use template tag dispatching with explicit/partial specialization for dispatcher in cases when it is needed.
			 * For this template the requirement is applied to T1 and T2 - they must be copy constructible.
			 * @tparam T1 The type of first value to store.
			 * @tparam T2 The type of second value to store.
			 */
			template < 
				typename T1, 
				typename T2
			>
			class CompressedPair<T1, T2, false>
			{
			private:
				T1 m_Val1;
				T2 m_Val2;
			public:
				/**
				 * Type of the first pair member.
				 */
				typedef T1 first_t;
				/**
				 * Type of the second pair member.
				 */
				typedef T2 second_t;

				/**
				 * @brief Main constructor of this class.
				 * Simply calls copy constructors of members with corresponding parameters.\n
				 * The T1 and T2 must meet copy-constructible requirement.
				 * @param val1 The value to be provide to the copy constructor of first stored value.
				 * @param val2 The value to be provide to the copy constructor of second stored value.
				 */
				CompressedPair(const T1& val1, const T2& val2) :
					m_Val1(val1), m_Val2(val2) {}

				/**
				 * @brief Copy constructor.
				 * @param[in] other The instance to make copy of.
				 */
				CompressedPair(const CompressedPair& other) :
					m_Val1(other.m_Val1), m_Val2(other.m_Val2) {}
				
				/**
				 * @brief Copy assignment operator.
				 * Don't allows self assignment.\n
				 * @param[in] other The instance to make copy of.
				 */
				CompressedPair& operator=(const CompressedPair& other)
				{
					// Handle self assignment case
					if (this == &other)
						return *this;
					m_Val1 = other.m_Val1;
					m_Val2 = other.m_Val2;
					return *this;
				}
				
				/**
				 * @brief Move constructor.
				 * This is the move constructor which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
				 * @param other Instance to be moved from.
				 */
				PCAPPP_MOVE_CONSTRUCTOR(CompressedPair) :
					m_Val1(PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Val1)),
					m_Val2(PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Val2)) {}
				
				/**
				 * @brief Move assignment operator.
				 * This is the move assignment operator which is automatically selects between library implementation of C++11 move semantics and C++11 move semantics.
				 * @param other Instance to be moved from.
				 * @return Reference to this object.
				 */
				PCAPPP_MOVE_ASSIGNMENT(CompressedPair)
				{
					// Handle self assignment case
					if (this == &PCAPPP_MOVE_OTHER)
						return *this;
					m_Val1 = PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Val1);
					m_Val2 = PCAPPP_MOVE(PCAPPP_MOVE_OTHER.m_Val2);
					return *this;
				}

				/**
				 * @brief Method to access the first stored value.
				 * @return Reference to the first stored value.
				 */
				T1& get_first() { return (m_Val1); } // () prevents compiler from certain type of RVO
				/**
				 * @brief Method to access the first stored value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the const-qualified first stored value.
				 */
				const T1& get_first() const { return (m_Val1); }
				/**
				 * @brief Method to access the second stored value.
				 * @return Reference to the second stored value.
				 */
				T2& get_second() { return (m_Val2); }
				/**
				 * @brief Method to access the second stored value.
				 * This overload is selected by compiler if object is const-qualified.
				 * @return Reference to the const-qualified second stored value.
				 */
				const T2& get_second() const { return (m_Val2); }
			};

		} // namespace pcpp::memory::Implementation

	} // namespace pcpp::memory

} // namespace pcpp

#endif /* PCAPPP_MEMORY_IMPLEMENTATION */