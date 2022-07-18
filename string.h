#include <cstring>
#include <iostream>

class String {
 private:
  char* str;
  int capacity;
  int size;  // with null byte

  int getLargePower(int n) const;
  int countLen(const char* str) const;
  void reverse(int begin, int end);

 public:
  String() {
    size = 1;
    capacity = getLargePower(size);

    str = new char[capacity];
    str[0] = 0;
  }

  ~String() { delete[] str; }

  String(const char* str);
  String(int n, char c);
  String(const String&);

  String& operator=(const String& string) {
    if (this != &string) {
      delete[] str;
      size = string.size;
      capacity = string.capacity;

      str = new char[capacity];
      memcpy(str, string.str, size);
    }

    return *this;
  }

  char& operator[](int idx);
  char operator[](int idx) const;
  String& operator+=(const String& src);
  String& operator+=(char c);

  size_t length() const;

  void push_back(char c);
  void pop_back();

  char& front();
  char& back();

  char front() const;
  char back() const;

  String substr(int start, int count) const;

  size_t find(const String& t) const;
  size_t rfind(const String& t) const;
  size_t find(const char* c) const;
  size_t rfind(const char* c) const;

  bool empty() const;
  void clear();

  friend std::ostream& operator<<(std::ostream& os, const String& s);
};

// Private methods
void String::reverse(int begin, int end) {
  for (int i = begin; i < (begin + end) / 2; ++i) {
    char swap = str[i];
    str[i] = str[end - i - 1];
    str[end - i - 1] = swap;
  }
}

int String::getLargePower(int n) const {
  int ans = 1;
  while (ans < n) ans *= 2;
  return ans;
}

int String::countLen(const char* src) const {
  int length = 0;
  while (*src) {
    length++;
    src++;
  }
  return length;
}

// Constructors
String::String(const char* src) {
  size = countLen(src) + 1;
  capacity = getLargePower(size);

  str = new char[capacity];

  memcpy(str, src, size);
}

String::String(int n, char c) {
  size = n + 1;
  capacity = getLargePower(size);

  str = new char[capacity];
  memset(str, c, n);
  str[n] = 0;
}

String::String(const String& string) {
  size = string.size;
  capacity = string.capacity;

  str = new char[capacity];
  memcpy(str, string.str, size);
}

// Base operators
char& String::operator[](int idx) {
  if (idx < size - 1 and idx >= 0) {
    return str[idx];
  }
  return str[size - 1];
}

char String::operator[](int idx) const {
  if (idx < size - 1 and idx >= 0) {
    return str[idx];
  }
  return str[size - 1];
}

String& String::operator+=(const String& src) {
  capacity = getLargePower(size + src.size - 1);
  char* newMemory = new char[capacity];

  memcpy(newMemory, str, size - 1);
  memcpy(newMemory + size - 1, src.str, src.size);

  size = size + src.size - 1;

  delete str;
  str = newMemory;

  return *this;
}

String& String::operator+=(char c) {
  push_back(c);
  return *this;
}

// Length
size_t String::length() const { return static_cast<size_t>(size - 1); }

// Push and pop
void String::push_back(char c) {
  if (size + 1 > capacity) {
    capacity *= 2;
    char* newMemory = new char[capacity];
    memcpy(newMemory, str, size);
    delete[] str;
    str = newMemory;
  }

  str[size - 1] = c;
  str[size] = 0;
  size++;
}

void String::pop_back() {
  if (size - 1 < capacity / 4) {
    capacity /= 2;
    char* newMemory = new char[capacity];
    memcpy(newMemory, str, size);
    delete[] str;
    str = newMemory;
  }

  str[size - 2] = 0;
  size--;
}

// Front and Back
char& String::front() {
  if (size - 1) {
    return str[0];
  }
  return str[size - 1];
}

char& String::back() {
  if (size - 1) {
    return str[size - 2];
  }
  return str[size - 1];
}

char String::front() const {
  if (size - 1) {
    return str[0];
  }
  return str[size - 1];
}

char String::back() const {
  if (size - 1) {
    return str[size - 2];
  }
  return str[size - 1];
}

// Empty and clear;
bool String::empty() const { return (size - 1) == 0; }

void String::clear() {
  delete[] str;
  size = 1;
  capacity = getLargePower(size);

  str = new char[capacity];
  str[0] = 0;
}

// Operator +
String operator+(const String& left, const String& right) {
  String result(left);
  result += right;
  return result;
};

String operator+(const String& str, char c) {
  String result(str);
  result += c;
  return result;
};

String operator+(char c, const String& str) {
  String result(1, c);
  result += str;
  return result;
};

// Operator ==
bool operator==(const String& left, const String& right) {
  if (left.length() != right.length()) return false;

  for (size_t i = 0; i < left.length(); ++i) {
    if (left[i] != right[i]) return false;
  }

  return true;
}

bool operator==(const char* str, const String& right) {
  String left(str);
  return left == right;
}

bool operator==(const String& left, const char* str) {
  String right(str);
  return left == right;
}

// Substr
String String::substr(int start, int count) const {
  String subString;
  if (start >= 0 and count > 0 and (start + count) <= (size - 1)) {
    subString.size = count + 1;
    subString.capacity = getLargePower(subString.size);

    delete[] subString.str;
    subString.str = new char[capacity];
    memcpy(subString.str, str + start, count);
    subString.str[subString.size - 1] = 0;
  }
  return subString;
}

// Find and rfind
size_t String::find(const String& target) const {
  if ((target.size - 1) and (size - 1)) {
    String source = target;
    source += '\0';
    source += (*this);

    int n = source.length();
    int* zFunc = new int[n];
    memset(zFunc, 0, sizeof(int) * n);

    int l = 0, r = 0;
    for (int i = 1; i < n; ++i) {
      if (i <= r)
        zFunc[i] = (zFunc[i - l] < r - i + 1 ? zFunc[i - l] : r - i + 1);

      while (i + zFunc[i] < n and source[i + zFunc[i]] == source[zFunc[i]]) {
        zFunc[i]++;
      }

      if (i + zFunc[i] - 1 > r) {
        r = i + zFunc[i] - 1;
        l = i;
      }
    }

    for (size_t i = 0; i < source.length(); ++i) {
      if (zFunc[i] == target.size - 1) return i - 1 - target.length();
    }
  }

  return length();
}

size_t String::rfind(const String& target) const {
  if ((target.size - 1) and (size - 1)) {
    String targetCopy = target;
    targetCopy.reverse(0, targetCopy.length());

    String sourceCopy = (*this);
    sourceCopy.reverse(0, sourceCopy.length());

    size_t result = sourceCopy.find(targetCopy);
    if (result != length()) {
      return length() - (result + target.length() - 1) - 1;
    }
  }

  return length();
}

size_t String::find(const char* str) const {
  String target(str);
  return find(target);
}

size_t String::rfind(const char* str) const {
  String target(str);
  return rfind(target);
}

// Operators >> and <<
std::ostream& operator<<(std::ostream& os, const String& src) {
  os << src.str;
  return os;
}

std::istream& operator>>(std::istream& is, String& str) {
  str.clear();
  char t;
  while (is.get(t) and t != '\n' and t != ' ') {
    str += t;
  }
  return is;
}
