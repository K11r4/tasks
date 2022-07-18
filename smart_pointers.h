#include <bits/stdc++.h>
#define myS "*----*\n O   |\n/|\\  |\n/ \\  |\n    ===="

template<typename T>
class SharedPtr
{
private:
	template<typename U>
	friend class SharedPtr;

	template<typename U>
	friend class WeakPtr;

	
	template<typename U, typename... Args>
	friend SharedPtr<U> makeShared(Args&&...);

	template<typename U, typename Allocator, typename... Args>
	friend SharedPtr<U> allocateShared(Allocator, Args&&...);
	/*
	template<typename... Args>
	friend makeShared(Args&&...);*/

	template<typename Alloc, typename Block>
	using BlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Block>;

	


	struct ControlBlock{
		size_t sharedCnt;
		size_t weakCnt;
		T* ptr;

		virtual ~ControlBlock() = default;
		virtual void killObject() = 0;
		virtual void suicide() = 0;
	};




	template<typename Deleter=std::default_delete<T>, typename Allocator=std::allocator<T>>
	struct ControlBlockRegular : ControlBlock
	{	
		Deleter del;
		Allocator alloc; 

		ControlBlockRegular() = default;

		ControlBlockRegular(T* src, Deleter deleter, Allocator allocator): del(deleter), alloc(allocator) {
			this->sharedCnt = 1;
			this->weakCnt = 0;
			this->ptr = src;
		};

		void killObject() override {
			del(this->ptr);
		}

		void suicide() override {
			//std::cout << myS << std::endl;
			using ControlBlockRegularAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<ControlBlockRegular<Deleter, Allocator>>;
			
			ControlBlockRegularAlloc allocToUse(alloc);
			//std::allocator_traits<ControlBlockRegularAlloc>::destroy(allocToUse, this);
			//del.~Deleter();
			//alloc.~Allocator();
			std::allocator_traits<ControlBlockRegularAlloc>::deallocate(allocToUse, this, 1);
			//delete this;
		}
	};


	template<typename Allocator=std::allocator<T>>
	struct ControlBlockMS : ControlBlock
	{
		Allocator alloc; 
		alignas(T) char object[sizeof(T)];

		void killObject() override {
			T* pointerToKill = reinterpret_cast<T*>(object);
			pointerToKill->~T();
		}

		void suicide() override {
			//std::cout << myS << std::endl;
			using ControlBlockMSAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<ControlBlockMS<Allocator>>;
			
			ControlBlockMSAlloc allocToUse(alloc);
			std::allocator_traits<ControlBlockMSAlloc>::destroy(allocToUse, this);
			std::allocator_traits<ControlBlockMSAlloc>::deallocate(allocToUse, this, 1);
		}

	};

	struct make_from_weak_tag{};
	struct make_shared_tag{};

	ControlBlock* ptr;

	SharedPtr(make_shared_tag, ControlBlock* other){
		ptr = other;
		++(ptr->sharedCnt);
	}

	SharedPtr(make_from_weak_tag, ControlBlock* other){
		ptr = other;
		++(ptr->sharedCnt);
	}

public:

	template<typename U>
	void swap(SharedPtr<U>& other){
		auto temp = other.ptr;
		other.ptr = reinterpret_cast<decltype(other.ptr)>(ptr);
		ptr = reinterpret_cast<decltype(ptr)>(temp);
	}

	SharedPtr(): ptr(nullptr){
		//++ptr->sharedCnt;
	};

	explicit SharedPtr(T* src): ptr(new ControlBlockRegular<>()){
		ptr->ptr = src;
		++ptr->sharedCnt;
	};

	template<typename Deleter>
	explicit SharedPtr(T* src, Deleter del): ptr(new ControlBlockRegular<Deleter>()){
		ptr->ptr = src;
		static_cast<ControlBlockRegular<Deleter>*>(ptr)->del = del;
		++ptr->sharedCnt;
	}

	template<typename Deleter, typename Allocator>
	explicit SharedPtr(T* src, Deleter del, Allocator alloc) {
		using ControlBlockAllocType = ControlBlockRegular<Deleter, Allocator>;
	
		using ControlBlockAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<ControlBlockAllocType>;
		using AllocatorTraits = std::allocator_traits<ControlBlockAlloc>;

		ControlBlockAlloc newAlloc = alloc;
		ControlBlockAllocType* newBlock = AllocatorTraits::allocate(newAlloc, 1);

		ptr = newBlock;
		//@Lexolordan Why we can't use alloc to construct ConstrolBlock?
		//AllocatorTraits::construct(newAlloc, newBlock); 

		new(newBlock) ControlBlockAllocType(src, del, alloc);
	}

	//template<typename U>
	SharedPtr(const SharedPtr& other): ptr(reinterpret_cast<decltype(ptr)>(other.ptr)) {
		if(ptr)
			++(ptr->sharedCnt);
	}

	void reset(){
		SharedPtr<T>().swap(*this);
	}

	template<typename U>
	void reset(U* newSrc){
		SharedPtr<U>(newSrc).swap(*this);
	}
	
	SharedPtr(SharedPtr&& other): ptr(reinterpret_cast<decltype(ptr)>(other.ptr)) {
		++(ptr->sharedCnt);
		other.reset();
	}

	template<typename U>
	SharedPtr(const SharedPtr<U>& other): ptr(reinterpret_cast<decltype(ptr)>(other.ptr)) {
		++(ptr->sharedCnt);
	}

	SharedPtr& operator=(const SharedPtr& other) {
		SharedPtr newPtr(other);
		newPtr.swap(*this);
		return *this;
	};
	
	template<typename U>
	SharedPtr& operator=(const SharedPtr<U>& other) {
		SharedPtr<U> newPtr(other);
		newPtr.swap(*this);
		return *this;
	};

	SharedPtr& operator=(SharedPtr&& other) {
		this->reset();
		this->swap(other);
		return *this;
	}

	template<typename U>
	SharedPtr& operator=(SharedPtr<U>&& other) {
		this->reset();
		this->swap(other);
		return *this;
	}

	T& operator*() const {
		return *(ptr->ptr);
	}

	T* operator->() const {
		return ptr->ptr;
	}

	size_t use_count() const {
		if(not ptr)
			return 0;
		return (ptr->sharedCnt);
	}

	T* get() const {
		if(not ptr)
			return nullptr;
		return ptr->ptr;
	}

	void debug() const {
		std::cout << ptr->ptr << ": " << ptr->sharedCnt << " " << ptr->weakCnt << "\n"; 
	}
	

	~SharedPtr(){
		if(not ptr)
			return;
		--(ptr->sharedCnt);
		if(not (ptr->sharedCnt)){
			ptr->killObject();
			if(not (ptr->weakCnt)){
				ptr->suicide();
			}
		}
	}
};

template<typename T>
class WeakPtr{
private:
	typename SharedPtr<T>::ControlBlock* ptr;

	template<typename U>
	friend class WeakPtr;
public:
	WeakPtr(): ptr(nullptr) {};

	WeakPtr(const WeakPtr& other): ptr(reinterpret_cast<decltype(ptr)>(other.ptr)) {
		if(ptr)
			++(ptr->weakCnt);
	};

	template<typename U>
	WeakPtr(const WeakPtr<U>& other): ptr(reinterpret_cast<decltype(ptr)>(other.ptr)) {
		if(ptr)
			++(ptr->weakCnt);
	};



	template<typename U>
	WeakPtr(const SharedPtr<U>& other): ptr(reinterpret_cast<decltype(ptr)>(other.ptr)) {
		if(ptr)
			++(ptr->weakCnt);
	}

	template<typename U>
	void swap(WeakPtr<U>& other){
		auto temp = other.ptr;
		other.ptr = reinterpret_cast<decltype(other.ptr)>(ptr);
		ptr = reinterpret_cast<decltype(ptr)>(temp);
	}

	void reset(){
		WeakPtr().swap(*this);
	}

	template<typename U>
	WeakPtr& operator=(const SharedPtr<U>& other) {
		WeakPtr<U> newPtr(other);
		newPtr.swap(*this);
		return *this;
	};

	WeakPtr(WeakPtr&& other): ptr(reinterpret_cast<decltype(ptr)>(other.ptr)) {
		++(ptr->weakCnt);
		other.reset();
	}

	WeakPtr& operator=(WeakPtr&& other) {
		this->reset();
		this->swap(other);
		return *this;
	}

	bool expired() const {
		return ptr && ptr->sharedCnt == 0;
	}

	SharedPtr<T> lock() const {
		return SharedPtr<T>(typename SharedPtr<T>::make_from_weak_tag(), ptr);
	}

	size_t use_count() const {
		if(not ptr)
			return 0;
		return ptr->sharedCnt;
	}

	void debug() const {
		std::cout << ptr->ptr << ": " << ptr->sharedCnt << " " << ptr->weakCnt << "\n"; 
	}

	~WeakPtr(){
		if(not ptr)
			return;
		--(ptr->weakCnt);
		if(not ptr->sharedCnt && not ptr->weakCnt){
			//delete ptr;
			//std::cout << "Hi" << std::endl;

			ptr->suicide();
		}
	}
};

template<typename T, typename Allocator, typename... Args>
SharedPtr<T> allocateShared(Allocator alloc, Args&&... args){
	using ControlBlock = typename SharedPtr<T>::template ControlBlockMS<Allocator>;
	
	using ControlBlockAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<ControlBlock>;
	using AllocatorTraits = std::allocator_traits<ControlBlockAlloc>;

	ControlBlockAlloc allocToUse(alloc);
	ControlBlock* dirtyPtr = AllocatorTraits::allocate(allocToUse, 1);

	new(dirtyPtr) ControlBlock();

	dirtyPtr->ptr = reinterpret_cast<T*>(dirtyPtr->object);

	AllocatorTraits::construct(allocToUse, dirtyPtr->ptr, std::forward<Args>(args)...);
	return SharedPtr<T>(typename SharedPtr<T>::make_shared_tag(), dirtyPtr);
}

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args){
	using ControlBlock = typename SharedPtr<T>::template ControlBlockMS<>;
	ControlBlock* dirtyPtr = new ControlBlock();

	dirtyPtr->ptr = reinterpret_cast<T*>(dirtyPtr->object);

	new(dirtyPtr->ptr) T(std::forward<Args>(args)...);
	return SharedPtr<T>(typename SharedPtr<T>::make_shared_tag(), dirtyPtr);
}