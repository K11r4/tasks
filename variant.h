#include <iostream>
#include <new>
#include <variant>
#include <vector>
#include <type_traits>


//index_by_type
template<size_t N, typename T, typename Head, typename... Tail>
struct get_index_by_type_impl{
	static constexpr size_t value = std::is_same_v<T, Head>
		? N
		: get_index_by_type_impl<N+1, T, Tail...>::value;
};

template<size_t N, typename T, typename Head>
struct get_index_by_type_impl<N, T, Head>{
	static constexpr size_t value = std::is_same_v<T, Head>
		? N
		: N + 1;
};

template<typename T, typename... Types>
static constexpr size_t index_by_type = get_index_by_type_impl<0, T, Types...>::value;


//type_by_index
template<size_t N, typename Head, typename... Tail>
struct type_by_index{
	using  type =  typename type_by_index<N - 1, Tail...>::type;
};

template<typename Head, typename... Tail>
struct type_by_index<0, Head, Tail...>{
	using  type =  Head;
};


//size_of_pack
template<typename Head, typename... Types>
struct size_of_pack{
	static constexpr size_t value = size_of_pack<Types...>::value + 1;
};

template<typename Head>
struct size_of_pack<Head>{
	static constexpr size_t value = 1;
};

//type_identity
template<typename T>
struct type_identity{
	using type = T;
};


//nearest_type
template<typename Head, typename... Tail>
struct overloadHelper : overloadHelper<Tail...>{
	using overloadHelper<Tail...>::operator();
	type_identity<Head> operator()(Head) {};
};

template<typename Head>
struct overloadHelper<Head>{
	type_identity<Head> operator()(Head) {};
};

template<typename T, typename... Types>
using nearest_type = typename std::invoke_result<overloadHelper<Types...>, T>::type::type;

//VariadicUnion
template <typename... Tail>
union VariadicUnion{
	VariadicUnion() {};
	~VariadicUnion() {};

	template <size_t Index>
	auto& get() = delete;

	template<typename T>
	void put() = delete;

	template<typename T>
	void destroy() = delete;

};

template <typename Head, typename... Tail>
union VariadicUnion<Head, Tail...>{
	Head head;
	VariadicUnion<Tail...> tail;

	VariadicUnion() {};
	~VariadicUnion() {};

	template <size_t Index>
	auto& get() const & {
		if constexpr(Index == 0){
			return head;
		} else {
			return tail.template get<Index-1>();
		}
	}

	template <size_t Index>
	auto& get() & {
		if constexpr(Index == 0){
			return head;
		} else {
			return tail.template get<Index-1>();
		}
	}

	template <size_t Index>
	auto&& get() && {
		if constexpr(Index == 0){
			return std::move(head);
		} else {
			return std::move(tail.template get<Index-1>());
		}
	}

	template <typename T, typename... Args> 
	void put(Args&&... args){
		using PtrType = std::remove_const_t<Head>;
		if constexpr (std::is_same_v<T, Head>){
			PtrType* ptrToPut = const_cast<PtrType*>(&head);
			new(std::launder(ptrToPut)) T(std::forward<Args>(args)...);
		} else {
			tail.template put<T>(std::forward<Args>(args)...);
		}
	}

	template<typename T>
	void destroy() {
		if constexpr (std::is_same_v<T, Head>){
			head.~Head();
		} else {
			tail.template destroy<T>();
		}
	}

};

template <typename... Types> class Variant; 

template<typename T, typename... Types>
class VariantAlternative {
public:

	using Derived = Variant<Types...>;
	static const size_t Index = index_by_type<T, Types...>;

	VariantAlternative() {};

	void destroy(){
		auto ptr = static_cast<Derived*>(this);
		if(Index == ptr->index){
			ptr->storage.template destroy<T>();
		}
	}

	template<typename... Ts>
	void copyActiveToVariant(Variant<Ts...>& target) const {
		auto ptr = static_cast<const Derived*>(this);
		if(Index == ptr->index){
			target.storage.template put<T>(ptr->storage.template get<Index>());
		}
	}

	template<typename... Ts>
	void moveActiveToVariant(Variant<Ts...>& target) {
		auto ptr = static_cast<Derived*>(this);
		if(Index == ptr->index){
			target.storage.template put<T>(std::move(ptr->storage.template get<Index>()));
		}
	}
};

template <typename... Types>
struct VariantStorage{
	VariadicUnion<Types...> storage;
	size_t index;
};


//variant
template <typename... Types>
class Variant :
	private VariantStorage<Types...>,
	private VariantAlternative<Types, Types...>...
{
private:
	template<typename T, typename... Ts>
	friend class VariantAlternative;

	template<size_t N, typename... Ts>
	friend auto& get(Variant<Ts...>& v);

	template<size_t N, typename... Ts>
	friend auto& get(const Variant<Ts...>& v);

	template<size_t N, typename... Ts>
	friend auto&& get(Variant<Ts...>&& v);

	template<typename T, typename... Ts>
	friend bool holds_alternative(Variant<Ts...>& v);
	

public:
	static constexpr size_t SIZE = size_of_pack<Types...>::value;

	void clear(){
		this->index = SIZE;
		(VariantAlternative<Types, Types...>::destroy(), ...);
	}

	template<typename... Ts>
	void copyActiveToVariant(Variant<Ts...>& target) const {
		(VariantAlternative<Types, Types...>::copyActiveToVariant(target), ...);
	}

	template<typename... Ts>
	void moveActiveToVariant(Variant<Ts...>& target) {
		(VariantAlternative<Types, Types...>::moveActiveToVariant(target), ...);
	}

	Variant() {
		this->index = 0;
		this->storage.template put<typename type_by_index<0, Types...>::type>();
	};

	template<typename T, typename U = nearest_type<T&&, Types...>>
	Variant(T&& value){
		static_assert(index_by_type<U, Types...> != SIZE);
		this->storage.template put<U>(value);
		this->index = index_by_type<U, Types...>;
	}

	Variant& operator=(const Variant& other){
		clear();
		other.copyActiveToVariant(*this);
		this->index = other.index;
		return *this;
	};

	Variant(const Variant& other): Variant() {
		*this = other;
	};

	Variant& operator=(Variant&& other){
		clear();
		other.moveActiveToVariant(*this);
		this->index = other.index;
		return *this;
	};

	Variant(Variant&& other): Variant() {
		*this = std::move(other);
	};
	
	template <typename T, typename... Args>
	auto& emplace(Args&&... args){
		this->~Variant();
		this->storage.template put<T, Args...>(std::forward<Args>(args)...);
		this->index = index_by_type<T, Types...>;
		return this->storage.template get<index_by_type<T, Types...>>();
	}

	template<size_t I, typename... Args>
	auto& emplace(Args&&... args){
		return emplace<typename type_by_index<I, Types...>::type>(std::forward<Args>(args)...);
	}

	template<typename T>
	auto& emplace(std::initializer_list<int> lst){
		this->~Variant();
		this->storage.template put<T>(lst);
		this->index = index_by_type<T, Types...>;
		return this->storage.template get<index_by_type<T, Types...>>();
	}

	
	~Variant() {
		clear();
	}
};

template <size_t N, typename... Types>
auto& get(Variant<Types...>& v){
	if(v.index != N){
		throw std::bad_variant_access();
	}
	return v.storage.template  get<N>();
}

template <size_t N, typename... Types>
auto& get(const Variant<Types...>& v){
	if(v.index != N){
		throw std::bad_variant_access();
	}
	return v.storage.template  get<N>();
}

template <size_t N, typename... Types>
auto&& get(Variant<Types...>&& v){
	if(v.index != N){
		throw std::bad_variant_access();
	}
	return std::move(std::move(v.storage).template  get<N>());
}

template<typename T, typename... Types>
auto& get(Variant<Types...>& v){
	return get<index_by_type<T, Types...> >(v);
}

template<typename T, typename... Types>
auto& get(const Variant<Types...>& v){
	return get<index_by_type<T, Types...> >(v);
}

template<typename T, typename... Types>
T&& get(Variant<Types...>&& v){
	return get<index_by_type<T, Types...> >(std::move(v));
}

template<typename T, typename... Types>
bool holds_alternative(Variant<Types...>& v){
	return (v.index == index_by_type<T, Types...>);
}