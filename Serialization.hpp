#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <stack>
#include <type_traits>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <cassert>


// Serialization concepts
namespace SerializationConcepts {
	template <typename T, typename = void>
	struct IsSerializable;

	template<class T>
	concept Serializable = IsSerializable<T>::value;

	template<class T>
	concept TriviallySerializable = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

	template<class T>
	concept ReadConstructableSerializable = requires(std::istream & If) {
		requires std::is_constructible_v<T, std::istream&>;
		requires !std::same_as<T, bool>;
	};

	template<class T>
	concept ReadSerializable = requires(T t, std::istream & If) {
		{ t.Read(If) } -> std::same_as<void>;
	};

	template<class T>
	concept WriteSerializable = requires(const T tconst, std::ostream & Of) {
		{ tconst.Write(Of) } -> std::same_as<void>;
	};

	template<class T>
	concept VectorOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::vector<typename T::value_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::value_type>;
	};

	template<class T>
	concept ListOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::list<typename T::value_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::value_type>;
	};

	template<class T>
	concept DequeOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::deque<typename T::value_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::value_type>;
	};

	template<class T>
	concept SetOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::set<typename T::key_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::key_type>;
	};

	template<class T>
	concept MultisetOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::multiset<typename T::key_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::key_type>;
	};

	template<class T>
	concept MapOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::map<typename T::key_type, typename T::mapped_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::key_type>;
		Serializable<typename T::mapped_type>;
	};

	template<class T>
	concept MultimapOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::multimap<typename T::key_type, typename T::mapped_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::key_type>;
		Serializable<typename T::mapped_type>;
	};

	template<class T>
	concept UnorderedSetOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::unordered_set<typename T::key_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::key_type>;
	};

	template<class T>
	concept UnorderedMapOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::unordered_map<typename T::key_type, typename T::mapped_type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::key_type>;
		Serializable<typename T::mapped_type>;
	};

	template<class T>
	concept ArrayOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::array<typename T::value_type, std::tuple_size<T>::value>>, std::remove_cvref_t<T>>;
		Serializable<typename T::value_type>;
	};

	/*  template<class T>
	  concept UniquePtrOfSerializable = requires(const T & t) {
		  std::same_as<std::remove_cvref_t<std::unique_ptr<typename T::element_type, typename T::deleter_type>>, std::remove_cvref_t<T>>;
		  Serializable<typename T::element_type>;
	  };*/

	template<class T>
	concept ReferenceWrapperOfSerializable = requires(const T & t) {
		requires std::same_as<std::remove_cvref_t<std::reference_wrapper<typename T::type>>, std::remove_cvref_t<T>>;
		Serializable<typename T::type>;
	};

	//template<class T>
	//concept StackOfSerializable = requires(const T & t) {
	//	std::same_as<std::remove_cvref_t<std::stack<typename T::value_type, typename T::container_type>>, std::remove_cvref_t<T>>;
	//	Serializable<typename T::value_type>;
	//};

	template <typename T, typename = void>
	struct IsSerializable : std::false_type {};

	template <typename T>
	struct IsSerializable<T, std::enable_if_t<
		TriviallySerializable<T> ||
		(WriteSerializable<T> && (ReadSerializable<T> || ReadConstructableSerializable<T>)) ||
		VectorOfSerializable<T> ||
		ListOfSerializable<T> ||
		DequeOfSerializable<T> ||
		SetOfSerializable<T> ||
		MultisetOfSerializable<T> ||
		MapOfSerializable<T> ||
		MultimapOfSerializable<T> ||
		UnorderedSetOfSerializable<T> ||
		UnorderedMapOfSerializable<T> ||
		ArrayOfSerializable<T> ||
		//UniquePtrOfSerializable<T> ||
		ReferenceWrapperOfSerializable<T>
		//StackOfSerializable<T>
		>> : std::true_type{};
}


template<SerializationConcepts::Serializable T>
void SerializedRead(std::istream& If, T& t);

template<SerializationConcepts::Serializable T>
void SerializedWrite(std::ostream & Of, const T & t);

namespace Serialization_Impl {
	// Trivial serialization
	template<SerializationConcepts::TriviallySerializable T>
	void SerializedRead_Impl(std::istream& If, T& t) {
		If.read(reinterpret_cast<char*>(&t), sizeof(T));
	}

	template<SerializationConcepts::TriviallySerializable T>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		Of.write(reinterpret_cast<const char*>(&t), sizeof(T));
	}

	// ReadWrite serialization
	template<SerializationConcepts::ReadSerializable T>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.Read(If);
	}

	template<SerializationConcepts::ReadConstructableSerializable T>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t = T(If);
	}

	template<SerializationConcepts::WriteSerializable T>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		t.Write(Of);
	}

	// Vector serialization
	template<SerializationConcepts::VectorOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		t.reserve(Size);

		typename T::value_type Element;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Element);
			t.emplace_back(std::move(Element));
		}
	}

	template<SerializationConcepts::VectorOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem);
		}
	}

	// List serialization
	template<SerializationConcepts::ListOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::value_type Element;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Element);
			t.push_back(std::move(Element));
		}
	}

	template<SerializationConcepts::ListOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem);
		}
	}

	// Deque serialization
	template<SerializationConcepts::DequeOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::value_type Element;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Element);
			t.push_back(std::move(Element));
		}
	}

	template<SerializationConcepts::DequeOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem);
		}
	}

	// Set serialization
	template<SerializationConcepts::SetOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::key_type Element;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Element);
			t.insert(std::move(Element));
		}
	}

	template<SerializationConcepts::SetOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem);
		}
	}

	// Map serialization
	template<SerializationConcepts::MapOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::key_type Key;
		typename T::mapped_type Value;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Key);
			SerializedRead(If, Value);
			t.emplace(std::move(Key), std::move(Value));
		}
	}

	template<SerializationConcepts::MapOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem.first);
			SerializedWrite(Of, Elem.second);
		}
	}

	// UnorderedSet serialization
	template<SerializationConcepts::UnorderedSetOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::key_type Element;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Element);
			t.insert(std::move(Element));
		}
	}

	template<SerializationConcepts::UnorderedSetOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem);
		}
	}

	// UnorderedMap serialization
	template<SerializationConcepts::UnorderedMapOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::key_type Key;
		typename T::mapped_type Value;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Key);
			SerializedRead(If, Value);
			t.emplace(std::move(Key), std::move(Value));
		}
	}

	template<SerializationConcepts::UnorderedMapOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem.first);
			SerializedWrite(Of, Elem.second);
		}
	}

	// Array serialization
	template<SerializationConcepts::ArrayOfSerializable T>
	void SerializedRead_Impl(std::istream& If, T& t) {
		for (auto& Elem : t) {
			SerializedRead(If, Elem);
		}
	}

	template<SerializationConcepts::ArrayOfSerializable T>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem);
		}
	}

	//// Unique_ptr serialization
	//template<SerializationConcepts::UniquePtrOfSerializable T>
	//void SerializedRead_Impl(std::istream& If, T& t) {
	//    using ElementType = typename std::remove_cvref_t<T>::element_type;
	//    t = std::make_unique<ElementType>();
	//    SerializedRead(If, *t);
	//}
	//// Unique_ptr serialization
	//template<SerializationConcepts::UniquePtrOfSerializable T>
	//void SerializedRead_Impl(std::istream& If, T& t) {
	//    using ElementType = T::element_type;
	//    t = std::make_unique<ElementType>();
	//    //auto element = std::make_unique<ElementType>(); 
	//    SerializedRead(If, *t);
	//    //t = std::move(element);
	//}

   /* template<SerializationConcepts::UniquePtrOfSerializable T>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		assert(t != nullptr && "Unique pointer must contain a value when writing.");
		SerializedWrite(Of, *t);
	}*/

	// Reference_wrapper serialization
	template<SerializationConcepts::ReferenceWrapperOfSerializable T>
	void SerializedRead_Impl(std::istream& If, T& t) {
		SerializedRead(If, t.get());
	}

	template<SerializationConcepts::ReferenceWrapperOfSerializable T>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, t.get());
	}

	// Multimap serialization
	template<SerializationConcepts::MultimapOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::key_type Key;
		typename T::mapped_type Value;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Key);
			SerializedRead(If, Value);
			t.emplace(std::move(Key), std::move(Value));
		}
	}

	template<SerializationConcepts::MultimapOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem.first);
			SerializedWrite(Of, Elem.second);
		}
	}

	// Multiset serialization
	template<SerializationConcepts::MultisetOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedRead_Impl(std::istream& If, T& t) {
		t.clear();

		SizeType Size;
		SerializedRead(If, Size);

		typename T::key_type Element;
		for (SizeType i = 0; i < Size; i++) {
			SerializedRead(If, Element);
			t.insert(std::move(Element));
		}
	}

	template<SerializationConcepts::MultisetOfSerializable T, typename SizeType = typename T::size_type>
	void SerializedWrite_Impl(std::ostream& Of, const T& t) {
		SerializedWrite(Of, (SizeType)t.size());

		for (const auto& Elem : t) {
			SerializedWrite(Of, Elem);
		}
	}

	//// Stack serialization
	//template<SerializationConcepts::StackOfSerializable T>
	//void SerializedRead_Impl(std::istream& If, T& t) {
	//	typename T::size_type Size;
	//	SerializedRead(If, Size);

	//	T tempStack;

	//	typename T::value_type Element;
	//	for (typename T::size_type i = 0; i < Size; i++) {
	//		SerializedRead(If, Element);
	//		tempStack.push(Element);
	//	}

	//	while (!tempStack.empty()) {
	//		t.push(tempStack.top());
	//		tempStack.pop();
	//	}
	//}

	//template<SerializationConcepts::StackOfSerializable T>
	//void SerializedWrite_Impl(std::ostream& Of, const T& t) {
	//	SerializedWrite(Of, (SizeType)t.size());

	//	std::stack<typename T::value_type> tempStack(t);

	//	while (!tempStack.empty()) {
	//		SerializedWrite(Of, tempStack.top());
	//		tempStack.pop();
	//	}
	//}
}


template<SerializationConcepts::Serializable T>
void SerializedRead(std::istream& If, T& t) {
	Serialization_Impl::SerializedRead_Impl(If, t);

	if (!If) {
		throw std::runtime_error("Failed to read");
	}
}

template<SerializationConcepts::Serializable T>
void SerializedWrite(std::ostream& Of, const T& t) {
	Serialization_Impl::SerializedWrite_Impl(Of, t);

	if (!Of) {
		throw std::runtime_error("Failed to write");
	}
}