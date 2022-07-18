#include <iostream>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <iterator>
#include <vector>
#include <list>
#include <algorithm>

template <typename T, typename Allocator = std::allocator<T>>
class List {
  template<typename K, typename V, typename H, typename E, typename A>
  friend class UnorderedMap;
 private:
  struct BaseNode {
    BaseNode* next;
    BaseNode* prev;
    BaseNode() : next(this), prev(this){};
  };

  struct Node : BaseNode {
    T value;

    template <typename... Args>
    Node(Args&&... args) : value(std::forward<Args>(args)...){};
  };

  template <bool isConst>
  struct Iterator {
    BaseNode* position;

    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = typename std::conditional<isConst, const T*, T*>::type;
    using reference = typename std::conditional<isConst, const T&, T&>::type;

    Iterator() = default;

    Iterator(const BaseNode* target)
        : position(const_cast<BaseNode*>(target)){};

    Iterator(const Iterator<false>& other) : position(other.position){};

    Iterator& operator++() {
      position = position->next;
      return *this;
    };

    Iterator operator++(int) {
      Iterator copy = *this;
      ++(*this);
      return copy;
    };

    Iterator& operator--() {
      position = position->prev;
      return *this;
    };

    Iterator operator--(int) {
      Iterator copy = *this;
      --(*this);
      return copy;
    };

    reference operator*() { return static_cast<Node*>(position)->value; }

    pointer operator->() { return &(static_cast<Node*>(position)->value); }

    bool operator==(const Iterator<isConst>& other) const {
      return position == other.position;
    }

    bool operator!=(const Iterator<isConst>& other) const {
      return position != other.position;
    }

    bool operator==(const Iterator<!isConst>& other) const {
      return position == other.position;
    }

    bool operator!=(const Iterator<!isConst>& other) const {
      return position != other.position;
    }
  };

  using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using AllocTraits = typename std::allocator_traits<NodeAlloc>;
  NodeAlloc allocator;
  BaseNode baseElem;
  size_t sz;

  void connectNodes(BaseNode* a, BaseNode* b) {
    a->next = b;
    b->prev = a;
  }

  void append(BaseNode* node, BaseNode* end) {
    BaseNode* last = end->prev;
    connectNodes(last, node);
    connectNodes(node, end);
  }

  void remove(BaseNode* node) {
    BaseNode* prevNode = node->prev;
    BaseNode* nextNode = node->next;
    connectNodes(prevNode, nextNode);

    AllocTraits::destroy(allocator, static_cast<Node*>(node));
    AllocTraits::deallocate(allocator, static_cast<Node*>(node), 1);
    sz--;
  }

  void clear(BaseNode* begin) {
    while (begin->next != begin) {
      BaseNode* nodeToDelete = begin->next;
      BaseNode* nextNode = nodeToDelete->next;

      connectNodes(&baseElem, nextNode);

      AllocTraits::destroy(allocator, static_cast<Node*>(nodeToDelete));
      AllocTraits::deallocate(allocator, static_cast<Node*>(nodeToDelete), 1);
    }
    if (begin == &baseElem) sz = 0;
  }

  template <typename... Args>
  void fill(size_t countObjects, Args&&... args) {
    for (size_t i = 0; i < countObjects; ++i) {
      try {
        Node* nodeToConstruct = AllocTraits::allocate(allocator, 1);
        try {
          Node* nodeToConstruct = AllocTraits::allocate(allocator, 1);
          AllocTraits::construct(allocator, nodeToConstruct,
                                 std::forward<Args>(args)...);

          append(nodeToConstruct, &baseElem);
        } catch (...) {
          AllocTraits::deallocate(allocator, nodeToConstruct, 1);
          clear(&baseElem);
          throw;
        }
      } catch (...) {
        clear(&baseElem);
        throw;
      }
    }

    sz = countObjects;
  }

 public:
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  void swap(List& other){
    if(sz and other.sz){
      std::swap(baseElem.prev->next, other.baseElem.prev->next);
      std::swap(baseElem.next->prev, other.baseElem.next->prev);
      std::swap(baseElem.prev, other.baseElem.prev);
      std::swap(baseElem.next, other.baseElem.next);
    } else if(sz) {
      connectNodes(&other.baseElem, baseElem.next);
      connectNodes(baseElem.prev, &other.baseElem);
      connectNodes(&baseElem, &baseElem);
    } else if(other.sz){
      connectNodes(&baseElem, other.baseElem.next);
      connectNodes(other.baseElem.prev, &baseElem);
      connectNodes(&other.baseElem, &other.baseElem);
    }
    std::swap(allocator, other.allocator);
    std::swap(sz, other.sz);
  }

  List() : allocator(Allocator()), baseElem(BaseNode()), sz(0){};

  List(const Allocator& a) : allocator(a), baseElem(BaseNode()), sz(0){};

  List(size_t n) : allocator(Allocator()), baseElem(BaseNode()), sz(0) {
    // T defaultValue = T();
    fill(n);
  };

  List(size_t n, const T& value)
      : allocator(Allocator()), baseElem(BaseNode()), sz(0) {
    fill(n, value);
  };

  List(size_t n, const Allocator& a)
      : allocator(a), baseElem(BaseNode()), sz(n) {
    // T defaultValue = T();
    fill(n);
  };

  List(size_t n, const T& value, const Allocator& a)
      : allocator(a), baseElem(BaseNode()), sz(n) {
    fill(n, value);
  };

  List(const List& other, Allocator newAlloc)
      : allocator(newAlloc), baseElem(BaseNode()), sz(0) {
    // newAlloc = static_cast<NodeAlloc>(newAlloc);
    Node* nodeToCopy = static_cast<Node*>(other.baseElem.next);

    for (size_t i = 0; i < other.sz;
         ++i, nodeToCopy = static_cast<Node*>(nodeToCopy->next)) {
      try {
        Node* nodeToConstruct = AllocTraits::allocate(allocator, 1);
        try {
          Node* nodeToConstruct = AllocTraits::allocate(allocator, 1);
          AllocTraits::construct(allocator, nodeToConstruct, nodeToCopy->value);

          append(nodeToConstruct, &baseElem);
        } catch (...) {
          AllocTraits::deallocate(allocator, nodeToConstruct, 1);
          clear(&baseElem);
          throw;
        }
      } catch (...) {
        clear(&baseElem);
        throw;
      }
    }

    sz = other.sz;
  }

  List(const List& other)
      : List(other, std::allocator_traits<Allocator>::
                        select_on_container_copy_construction(
                            other.get_allocator())){};

  List(List&& other)
      : List() {
    swap(other);
  };

  List& operator=(List&& other) {
    clear(&baseElem);
    swap(other);

    return *this;
  };

  List& operator=(const List& other) {
    NodeAlloc newAlloc = get_allocator();
    if (std::allocator_traits<
            Allocator>::propagate_on_container_copy_assignment::value) {
      newAlloc = other.get_allocator();
    }
    NodeAlloc copy = allocator;
    List<T, Allocator> newList(other, newAlloc);

    clear(&baseElem);
    connectNodes(&baseElem, newList.baseElem.next);
    connectNodes(newList.baseElem.prev, &baseElem);
    connectNodes(&newList.baseElem, &newList.baseElem);

    sz = newList.sz;
    allocator = newAlloc;

    return *this;
  };

  Allocator get_allocator() const { return allocator; }

  size_t size() const { return sz; }

  void pop_back() { remove(baseElem.prev); }

  void pop_front() { remove(baseElem.next); }

  iterator insert(const_iterator it, T&& value) {
    Node* nodeToInsert = allocator.allocate(1);
    try {
      AllocTraits::construct(allocator, nodeToInsert, std::move(value));

      append(nodeToInsert, it.position);
      sz++;

      return iterator(it.position->prev);
    } catch (...) {
      AllocTraits::deallocate(allocator, nodeToInsert, 1);
      throw;
    }
  }

  iterator insert(const_iterator it, const T& value) {
    T temp(value);
    return insert(it, std::move(temp));
  }

  void push_back(const T& value) { insert(iterator(&baseElem), value); }

  void push_front(const T& value) { insert(iterator(baseElem.next), value); }

  void erase(const_iterator it) { remove(it.position); }

  void erase(const_reverse_iterator it) { remove(it.position); }

  void splice(iterator positionToinsert, List& other, iterator elmentToSplice){
    connectNodes(elmentToSplice.position->prev, elmentToSplice.position->next);
    connectNodes(positionToinsert.position->prev, elmentToSplice.position);
    connectNodes(elmentToSplice.position, positionToinsert.position);
    sz++; other.sz--;
  }

  void debug() const {
    std::cout << "[" << sz << "] ";
    for (BaseNode* i = baseElem.next; i != &baseElem; i = i->next) {
      std::cout << (static_cast<Node*>(i))->value << " ";
    }
    std::cout << std::endl;
  }

  ~List() { clear(&baseElem); }

  iterator begin() { return iterator(baseElem.next); }

  iterator end() { return iterator(&baseElem); }

  reverse_iterator rbegin() { return reverse_iterator(&baseElem); }

  reverse_iterator rend() { return reverse_iterator(baseElem.next); }

  const_iterator begin() const { return iterator(baseElem.next); }

  const_iterator end() const { return iterator(&baseElem); }

  const_iterator cbegin() const { return const_iterator(baseElem.next); }

  const_iterator cend() const { return const_iterator(&baseElem); }

  const_reverse_iterator rbegin() const { return reverse_iterator(&baseElem); }

  const_reverse_iterator rend() const {
    return reverse_iterator(baseElem.next);
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(&baseElem);
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(baseElem.next);
  }
};

template<
	typename Key,
	typename Value, 
	typename Hash=std::hash<Key>, 
	typename Equal=std::equal_to<Key>, 
	typename Alloc=std::allocator<std::pair<const Key, Value>>
>
class UnorderedMap{
	//friend int main(); //TODO:delete!!!
public:
	using NodeType = std::pair<const Key, Value>;

private:
	using SecretNodeType = std::pair<Key, Value>;

	struct ListNode{
		SecretNodeType kv;
		size_t hashKey;
	};

	using AllocTraits = typename std::allocator_traits<Alloc>;
	
	using MapAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType>;

	using ListAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ListNode>;
	using MapList = List<ListNode, ListAlloc>;

	using ListIter = typename MapList::iterator;
	using VectorAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ListIter>;
	using MapVector = std::vector<ListIter, VectorAlloc>;
	/*
	using AnotherShitTraits  = std::allocator_traits<Alloc>;
	using AllocTraits = typename AnotherShitTraits::template rebind_traits<NodeType>;
	*/

	constexpr const float static maxLoadFactor = 3.0;
	constexpr const int static defaultSize = 1000;

	template<bool isConst>
	struct MapIterator{
		ListIter position;

		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
    	using value_type = NodeType;
    	using pointer = typename std::conditional<isConst, const NodeType*, NodeType*>::type;  
    	using reference = typename std::conditional<isConst, const NodeType&, NodeType&>::type;


		MapIterator(ListIter target): position(target) {};
		MapIterator(const MapIterator<false>& other): position(other.position) {};


		MapIterator& operator++(){
			position++;
			return *this;
		};

		MapIterator operator++(int){
			MapIterator copy = *this;
			++(*this);
			return copy;
		};

		MapIterator& operator--(){
			position++;
			return *this;
		};

		MapIterator operator--(int){
			MapIterator copy = *this;
			--(*this);
			return copy;
		};

		reference operator*(){
			return *(reinterpret_cast<NodeType*>(&(*position).kv));
		
		}

		pointer operator->(){
			return reinterpret_cast<NodeType*>(&(*position).kv);
		}

		bool operator==(const MapIterator<isConst>& other) const {
			return position == other.position;
		}

		bool operator!=(const MapIterator<isConst>& other) const {
			return position != other.position;
		}

		bool operator==(const MapIterator<!isConst>& other) const {
			return position == other.position;
		}

		bool operator!=(const MapIterator<!isConst>& other) const {
			return position != other.position;
		}

	};

	MapAlloc allocator;
	MapList elements;
	MapVector hashTable;

	size_t cap;
	size_t buckets;
	size_t sz;

	ListIter findInBucket(ListIter begin, const Key& key){
		if(begin == elements.end())
			return begin;


		size_t hash = (*begin).hashKey % cap;
		while(begin != elements.end() and (*begin).hashKey % cap == hash){
			if(Equal{}(key, (*begin).kv.first))
				return begin;
			begin++;
		}

		return elements.end();
	}

	void swap(UnorderedMap&& other){

	}

	void checkSize(){
		if(size() / cap > max_load_factor())
			rehash(2 * cap);
	}

public:

	using Iterator = MapIterator<false>;
	using ConstIterator = MapIterator<true>;

	Iterator begin(){
		return Iterator(elements.begin());
	}

	Iterator end(){
		return Iterator(elements.end());
	}

	ConstIterator cbegin(){
		return ConstIterator(elements.begin());
	}

	ConstIterator cend(){
		return ConstIterator(elements.end());
	}

	UnorderedMap(): allocator(Alloc()), elements(MapList()), hashTable(defaultSize, elements.end()), cap(defaultSize), buckets(0), sz(0) {
		//std::cout << "Default constructor\n";
	};

	UnorderedMap(const UnorderedMap& other) {
		//Need exeption safety
		//std::cout << "Copy constructor\n";
		MapVector newTable = MapVector(other.hashTable.size());
		elements = MapList(other.elements);

		//No exeptions

		newTable.assign(other.hashTable.size(), elements.end());
		for(auto it = elements.begin(); it != elements.end(); ++it){
			size_t idx = (*it).hashKey % newTable.size();
			if(newTable[idx] == elements.end()){
				newTable[idx] = it;

			}

		}

		//allocator = elements.get_allocator();
		hashTable = std::move(newTable);
		sz = other.sz;
		cap = other.cap;
		buckets = other.buckets;
	};

	UnorderedMap(UnorderedMap&& other) {
		//std::cout << "Move constructor\n";
		elements = std::move(other.elements);
		hashTable = std::move(other.hashTable);

		for(size_t i = 0; i < hashTable.size(); ++i)
			if(hashTable[i] == other.elements.end())
				hashTable[i] = elements.end();

		allocator = elements.get_allocator();
		sz = std::move(other.sz);
		cap = std::move(other.cap);
		buckets = std::move(other.buckets);

	};

	UnorderedMap& operator=(UnorderedMap&& other) {
		//std::cout << "Move assignment operator\n";
		elements = std::move(other.elements);
		hashTable = std::move(other.hashTable);

		for(size_t i = 0; i < hashTable.size(); ++i)
			if(hashTable[i] == other.elements.end()){
				hashTable[i] = elements.end();
			}

		allocator = elements.get_allocator();
		sz = std::move(other.sz);
		cap = std::move(other.cap);
		buckets = std::move(other.buckets);

		return *this;
	};

	UnorderedMap& operator=(const UnorderedMap& other) {
		//std::cout << "Assignment operator\n";
		UnorderedMap newMap(other);
		*this = std::move(newMap);
		allocator = elements.get_allocator();
		return *this;
		/*
		elements = other.elements;
		hashTable = other.hashTable;

		for(int i = 0; i < hashTable.size(); ++i)
			if(hashTable[i] == other.elements.end())
				hashTable[i] = elements.end();

		allocator = elements.get_allocator();
		sz = other.sz;
		cap = other.cap;
		buckets = other.buckets;
		return *this;*/
	};	

	

	Iterator find(const Key& key){
		size_t hash = Hash{}(key);
		return Iterator(findInBucket(hashTable[hash % cap], key));
	}

	Value& operator[] (const Key& key){
		//std::cout << "Hello from []\n";
		size_t hash = Hash{}(key);
		ListIter position = findInBucket(hashTable[hash % cap], key);
		
		if(position == elements.end()){
			position = hashTable[hash % cap];
			SecretNodeType newElem{key, Value()};
			position = elements.insert(position, std::move(ListNode{std::move(newElem), hash}));
			hashTable[hash % hashTable.size()] = position;

			sz++;
		}

		return (*position).kv.second;
	}

	Value& at(const Key& key){
		size_t hash = Hash{}(key);
		ListIter position = findInBucket(hashTable[hash % cap], key);

		if(position == elements.end()){
			throw std::out_of_range("Key Error");
		}

		return (*position).kv.second;
	};

	//L-value insert
	std::pair<Iterator, bool> insert(const NodeType& newElem) {
		size_t hash = Hash{}(newElem.first);
		ListIter position = findInBucket(hashTable[hash % cap], newElem.first);

		bool exist = (position != elements.end());
		(*this)[newElem.first] = newElem.second;
		//position = findInBucket(position, newElem.first);

		return std::make_pair(Iterator(hashTable[hash % cap]), not exist);
	}

	//R-value insert
	std::pair<Iterator, bool> insert(NodeType&& newElem) {
		size_t hash = Hash{}(newElem.first);
		SecretNodeType& magicElem = *(reinterpret_cast<SecretNodeType*>(&newElem));
		ListIter bucket = hashTable[hash % cap];

		ListIter position = findInBucket(bucket, magicElem.first);

		bool exist = (position != elements.end());
		if(exist){
			(*position).kv.second = std::move(magicElem.second);
			return std::make_pair(Iterator(position), not exist);
		} else {
			bucket = elements.insert(bucket, std::move(ListNode{std::move(magicElem), hash}));
			hashTable[hash % cap] = bucket;
			return std::make_pair(Iterator(bucket), not exist);
		}
		//position = findInBucket(position, newElem.first);

		
	}

	//TO DO: Exception safety
	template<typename InputIterator>
	void insert(InputIterator begin, InputIterator end){
		while(begin != end){
			insert(*begin);
			++begin;
		}
	}
	
	template<typename... Args>
	std::pair<Iterator, bool> emplace(Args&&... args){
		NodeType* p = AllocTraits::allocate(allocator, 1);
		try{
			AllocTraits::construct(allocator, p, std::forward<Args>(args)...);
			
			
		} catch(...){
			AllocTraits::deallocate(allocator, p, 1);
			throw;
		}

		try{
			//std::pair<Iterator, bool> result = insert(std::move(*p));
			size_t hash = Hash{}((*p).first);
			SecretNodeType& magicElem = *(reinterpret_cast<SecretNodeType*>(p));
			ListIter bucket = hashTable[hash % cap];

			ListIter position = findInBucket(bucket, magicElem.first);


			bool exist = (position != elements.end());
			if(not exist){
				bucket = elements.insert(bucket, std::move(ListNode{std::move(magicElem), hash}));
				hashTable[hash % cap] = bucket;
			}
			//std::cout << p->first << " " << p->second << "\n";
			AllocTraits::destroy(allocator, p);
			AllocTraits::deallocate(allocator, p, 1);

			checkSize();

			return std::make_pair(Iterator(exist ? elements.end() : bucket), not exist);

		} catch(...){
			AllocTraits::destroy(allocator, p);
			AllocTraits::deallocate(allocator, p, 1);
			throw;
		}
	}

	void erase(Iterator it){
		Iterator next = it;
		next++;
		if(next == end() or (*next.position).hashKey % cap != (*it.position).hashKey % cap)
			hashTable[(*it.position).hashKey % cap] = elements.end();
		else
			hashTable[(*it.position).hashKey % cap] = next.position;			

		elements.erase(it.position);
		sz--;
	}

	//TO DO: Exception safety
	void erase(Iterator begin, Iterator end){
		while(begin != end){
			erase(begin++);
		}
	}

	size_t size() const {
		return elements.size();
	}

	float max_load_factor() {
		return maxLoadFactor;
	}
	
	Alloc get_allocator(){
		return allocator;
	}

	void rehash(size_t count){
		count = std::max(static_cast<size_t>(size() / max_load_factor()), count);
		MapList newElements(elements.get_allocator());
		MapVector newTable(count, newElements.end());
		

		while(elements.size()){
			size_t idx = (*elements.begin()).hashKey % count;
			ListIter position = newTable[idx];
			newElements.splice(position, elements, elements.begin());
			newTable[idx] = --position;
		}

		buckets = count;
		for(size_t i = 0; i < count; ++i)
			if(newTable[i] == newElements.end()){
				newTable[i] = elements.end();
				buckets--;
			}

		elements = std::move(newElements);
		hashTable = std::move(newTable);
		cap = count;
	}

	void reserve(size_t count){
		rehash(count);
	}

	void debug(){
		std::cout << "[sz:" << sz <<", cap:" << cap << ", buck:" << buckets << "] ";
		for(size_t i = 0; i < hashTable.size(); ++i)
			if(hashTable[i] != elements.end())
				std::cout << (*hashTable[i]).kv.first << " | ";
		std::cout << "\n";
		for(auto it : elements){
			std::cout << " {" << it.kv.first << ", " << it.kv.second << "} " << " ";
		}
		std::cout << "\n";
	}
};