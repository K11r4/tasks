#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>

template <typename T, typename Allocator = std::allocator<T>>
class List {
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

  using NodeAlloc =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
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

  void drag(BaseNode* to, BaseNode* from) {
    connectNodes(from->prev, from->next);
    connectNodes(to->prev, from);
    connectNodes(from, to);
  }

 public:
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
      : allocator(other.get_allocator()), baseElem(BaseNode()), sz(other.sz) {
    if (other.sz) {
      connectNodes(&baseElem, other.baseElem.next);
      connectNodes(other.baseElem.prev, &baseElem);
    }
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
