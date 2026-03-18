// =============================================================================
// HEADER INCLUDES
// =============================================================================
#include <iostream>   // For cout, endl (used in main)
#include <string>     // For std::string (used in main and as template type)
#include <initializer_list>  // For Vector v = {1,2,3}; syntax
#include <iterator>   // For std::reverse_iterator (reverse begin/end)
#include <stdexcept>  // For std::out_of_range (thrown by at(), erase(), insert())

// =============================================================================
// CUSTOM VECTOR CLASS (mimics std::vector, no allocator support)
// =============================================================================
// template<typename T> makes this a class template: you use Vector<int>, Vector<string>, etc.
template<typename T>
class Vector {
public:
  // --- Type aliases (same names as std::vector so generic code can use them) ---
  using value_type      = T;             // The type of each element (e.g. int, string)
  using size_type       = size_t;        // Type for size and capacity (unsigned, large enough)
  using difference_type = ptrdiff_t;     // Type for iterator difference (e.g. it1 - it2)
  using reference       = T&;            // Reference to element (non-const)
  using const_reference = const T&;      // Reference to const element
  using pointer         = T*;            // Pointer to element
  using const_pointer   = const T*;      // Pointer to const element
  using iterator        = T*;           // Iterator is just a raw pointer to T
  using const_iterator  = const T*;      // Const iterator is pointer to const T
  using reverse_iterator       = std::reverse_iterator<iterator>;        // For rbegin()/rend()
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;   // Const reverse iterators

private:
  // --- Data members (the actual storage) ---
  T* data_;       // Dynamic array holding the elements; nullptr when empty
  size_type size_; // Number of elements currently in the vector (logical size)
  size_type cap_;  // Number of elements the allocated array can hold (capacity)

  // --- grow_to: allocate a new larger array and move existing elements ---
  // Only grows; never shrinks. Used by reserve() and when we run out of capacity.
  void grow_to(size_type new_cap) {
    if (new_cap <= cap_) return;              // Already big enough; do nothing
    T* new_data = new T[new_cap];             // Allocate new array of size new_cap
    for (size_type i = 0; i < size_; ++i)     // Move each existing element (avoids extra copies)
      new_data[i] = std::move(data_[i]);
    delete[] data_;                           // Free the old array
    data_ = new_data;                         // Point to the new array
    cap_  = new_cap;                          // Update stored capacity
  }

  // --- realloc_if_needed: if we're full, double the capacity (or set to 1 if was 0) ---
  // Called before push_back/emplace_back/insert so we always have room for one more.
  void realloc_if_needed() {
    if (size_ == cap_)                                    // No room left
      grow_to(cap_ == 0 ? 1 : cap_ * 2);  // Grow to 1 (if empty) or 2x current
  }

public:
  // =========================================================================
  // CONSTRUCTORS
  // =========================================================================

  // Default constructor: empty vector, no allocation
  Vector() noexcept : data_(nullptr), size_(0), cap_(0) {}

  // Fill constructor (count default-initialized elements): Vector<int> v(5);
  // explicit prevents accidental conversion: Vector<int> v = 5; is not allowed
  explicit Vector(size_type count) : data_(count ? new T[count]() : nullptr), size_(count), cap_(count) {}
  // () in new T[count]() means value-initialize (e.g. 0 for int, "" for string)

  // Fill constructor (count copies of value): Vector<int> v(5, 42);
  Vector(size_type count, const T& value) : data_(count ? new T[count] : nullptr), size_(count), cap_(count) {
    for (size_type i = 0; i < count; ++i)
      data_[i] = value;  // Copy value into each slot
  }

  // Initializer-list constructor: Vector<int> v = {1, 2, 3};
  Vector(std::initializer_list<T> init) : data_(nullptr), size_(0), cap_(0) {
    reserve(init.size());           // Allocate exactly enough space (no realloc during push_back)
    for (const T& x : init)
      push_back(x);                 // Copy each element from the list
  }

  // Copy constructor: deep copy of other's elements into a new array
  Vector(const Vector& other) : data_(nullptr), size_(other.size_), cap_(other.cap_) {
    if (cap_ > 0) {
      data_ = new T[cap_];
      for (size_type i = 0; i < size_; ++i)
        data_[i] = other.data_[i];  // Copy each element
    }
  }

  // Move constructor: steal other's array; other is left empty and valid
  Vector(Vector&& other) noexcept
    : data_(other.data_), size_(other.size_), cap_(other.cap_) {
    other.data_ = nullptr;  // Prevent other's destructor from deleting the array we now own
    other.size_ = 0;
    other.cap_  = 0;
  }

  // Destructor: free the dynamic array
  ~Vector() { delete[] data_; }

  // =========================================================================
  // ASSIGNMENT
  // =========================================================================

  // Copy assignment: make *this a copy of other (copy-and-swap: copy then swap with tmp)
  Vector& operator=(const Vector& other) {
    if (this == &other) return *this;  // Self-assignment: do nothing
    Vector tmp(other);                 // Copy other into a temporary
    swap(tmp);                          // Swap our contents with tmp; tmp gets our old data and is destroyed
    return *this;
  }

  // Move assignment: steal other's array; other is left empty
  Vector& operator=(Vector&& other) noexcept {
    if (this == &other) return *this;
    delete[] data_;         // Free our current array
    data_ = other.data_;    // Steal other's pointer
    size_ = other.size_;
    cap_  = other.cap_;
    other.data_ = nullptr;  // Leave other in valid empty state
    other.size_ = 0;
    other.cap_  = 0;
    return *this;
  }

  // Assign from initializer list: v = {1, 2, 3};
  Vector& operator=(std::initializer_list<T> init) {
    Vector tmp(init);  // Build temporary vector from list
    swap(tmp);         // Swap; old *this is destroyed when tmp goes out of scope
    return *this;
  }

  // =========================================================================
  // SWAP
  // =========================================================================
  // Swap contents of *this and other in O(1); no copying of elements
  void swap(Vector& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(cap_, other.cap_);
  }

  // =========================================================================
  // CAPACITY
  // =========================================================================
  bool empty() const noexcept { return size_ == 0; }
  size_type size() const noexcept { return size_; }
  size_type capacity() const noexcept { return cap_; }

  // Maximum number of elements we could theoretically hold (implementation limit)
  size_type max_size() const noexcept { return static_cast<size_type>(-1) / sizeof(T); }

  // Ensure capacity is at least new_cap; only grows, never shrinks
  void reserve(size_type new_cap) {
    if (new_cap <= cap_) return;
    grow_to(new_cap);
  }

  // Reduce capacity to match size (free unused memory)
  void shrink_to_fit() {
    if (cap_ == size_) return;
    if (size_ == 0) {
      delete[] data_;
      data_ = nullptr;
      cap_  = 0;
      return;
    }
    T* new_data = new T[size_];       // Allocate exactly size_ elements
    for (size_type i = 0; i < size_; ++i)
      new_data[i] = std::move(data_[i] );
    delete[] data_;
    data_ = new_data;
    cap_  = size_;
  }

  // =========================================================================
  // ELEMENT ACCESS
  // =========================================================================
  // operator[]: no bounds check (like std::vector); undefined behavior if pos >= size_
  reference operator[](size_type pos) { return data_[pos]; }
  const_reference operator[](size_type pos) const { return data_[pos]; }

  // at(): bounds-checked access; throws std::out_of_range if pos >= size_
  reference at(size_type pos) {
    if (pos >= size_) throw std::out_of_range("Vector::at");
    return data_[pos];
  }
  const_reference at(size_type pos) const {
    if (pos >= size_) throw std::out_of_range("Vector::at");
    return data_[pos];
  }

  // First element (undefined behavior if empty)
  reference front() { return data_[0]; }
  const_reference front() const { return data_[0]; }
  // Last element (undefined behavior if empty)
  reference back() { return data_[size_ - 1]; }
  const_reference back() const { return data_[size_ - 1]; }

  // Raw pointer to the underlying array (for C APIs or pointer arithmetic)
  T* data() noexcept { return data_; }
  const T* data() const noexcept { return data_; }

  // =========================================================================
  // ITERATORS
  // =========================================================================
  // begin/end: range [begin, end) over the elements
  iterator begin() noexcept { return data_; }
  const_iterator begin() const noexcept { return data_; }
  const_iterator cbegin() const noexcept { return data_; }
  iterator end() noexcept { return data_ + size_; }           // One past last element
  const_iterator end() const noexcept { return data_ + size_; }
  const_iterator cend() const noexcept { return data_ + size_; }

  // Reverse iterators: iterate from back to front
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }   // Last element
  const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
  const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }   // One before first
  const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
  const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

  // =========================================================================
  // MODIFIERS
  // =========================================================================
  // clear: logical size to 0; capacity unchanged; elements not destroyed (simplified; std may destroy)
  void clear() noexcept { size_ = 0; }

  // push_back: add one element at the end (copy version)
  void push_back(const T& value) {
    realloc_if_needed();    // Grow array if full
    data_[size_++] = value; // Copy value at end, then increment size
  }
  // push_back: add one element at the end (move version; used when you pass std::move(x))
  void push_back(T&& value) {
    realloc_if_needed();
    data_[size_++] = std::move(value);
  }

  // emplace_back: construct element in place at the end (avoids copy/move of T)
  // Args are forwarded to T's constructor: v.emplace_back(1, 2); for T(int, int)
  template<typename... Args>
  reference emplace_back(Args&&... args) {
    realloc_if_needed();
    data_[size_] = T(std::forward<Args>(args)...);  // Construct T in place
    return data_[size_++];
  }

  // pop_back: remove last element (no bounds check; undefined if empty)
  void pop_back() {
    if (size_ > 0) --size_;
  }

  // resize(count): make size count; if count > size_, new elements are value-initialized (T())
  void resize(size_type count) {
    if (count > size_) {
      reserve(count);
      for (size_type i = size_; i < count; ++i)
        data_[i] = T();
      size_ = count;
    } else {
      size_ = count;
    }
  }
  // resize(count, value): new elements are copies of value
  void resize(size_type count, const T& value) {
    if (count > size_) {
      reserve(count);
      for (size_type i = size_; i < count; ++i)
        data_[i] = value;
      size_ = count;
    } else {
      size_ = count;
    }
  }

  // erase by index (convenience; std::vector primarily has iterator erase)
  void erase(size_type pos) {
    if (pos >= size_) throw std::out_of_range("Vector::erase");
    for (size_type i = pos; i < size_ - 1; ++i)
      data_[i] = std::move(data_[i + 1]);  // Shift elements left
    --size_;
  }

  // erase one element at iterator pos; returns iterator to element after the erased one
  iterator erase(const_iterator pos) {
    difference_type off = pos - cbegin();   // Index of pos
    if (off < 0 || static_cast<size_type>(off) >= size_)
      throw std::out_of_range("Vector::erase");
    for (size_type i = static_cast<size_type>(off); i < size_ - 1; ++i)
      data_[i] = std::move(data_[i + 1]);
    --size_;
    return data_ + off;
  }
  // erase range [first, last); returns iterator to where the erased range was (now next element)
  iterator erase(const_iterator first, const_iterator last) {
    difference_type n = last - first;
    if (n <= 0) return begin() + (first - cbegin());
    difference_type start = first - cbegin();
    for (size_type i = static_cast<size_type>(start); i < size_ - static_cast<size_type>(n); ++i)
      data_[i] = std::move(data_[i + static_cast<size_type>(n)]);  // Shift block left
    size_ -= static_cast<size_type>(n);
    return data_ + start;
  }

  // insert one element (copy) before pos; returns iterator to inserted element
  iterator insert(const_iterator pos, const T& value) {
    difference_type off = pos - cbegin();
    if (off < 0 || static_cast<size_type>(off) > size_)
      throw std::out_of_range("Vector::insert");
    realloc_if_needed();
    size_type i = size_;
    while (i > static_cast<size_type>(off)) {
      data_[i] = std::move(data_[i - 1]);  // Shift elements right
      --i;
    }
    data_[off] = value;
    ++size_;
    return data_ + off;
  }
  // insert one element (move) before pos
  iterator insert(const_iterator pos, T&& value) {
    difference_type off = pos - cbegin();
    if (off < 0 || static_cast<size_type>(off) > size_)
      throw std::out_of_range("Vector::insert");
    realloc_if_needed();
    size_type i = size_;
    while (i > static_cast<size_type>(off)) {
      data_[i] = std::move(data_[i - 1]);
      --i;
    }
    data_[off] = std::move(value);
    ++size_;
    return data_ + off;
  }
  // insert count copies of value before pos
  iterator insert(const_iterator pos, size_type count, const T& value) {
    difference_type off = pos - cbegin();
    if (off < 0 || static_cast<size_type>(off) > size_)
      throw std::out_of_range("Vector::insert");
    if (count == 0) return data_ + off;
    reserve(size_ + count);
    size_type insert_at = static_cast<size_type>(off);
    for (size_type i = size_; i > insert_at; --i)
      data_[i + count - 1] = std::move(data_[i - 1]);  // Shift right by count
    for (size_type i = 0; i < count; ++i)
      data_[insert_at + i] = value;
    size_ += count;
    return data_ + off;
  }
};

// =============================================================================
// NON-MEMBER SWAP (so std::swap(v1, v2) and ADL swap work)
// =============================================================================
template<typename T>
void swap(Vector<T>& a, Vector<T>& b) noexcept {
  a.swap(b);
}

// =============================================================================
// RELATIONAL OPERATORS (lexicographic comparison, like std::vector)
// =============================================================================
template<typename T>
bool operator==(const Vector<T>& a, const Vector<T>& b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i)
    if (a[i] != b[i]) return false;
  return true;
}
template<typename T>
bool operator!=(const Vector<T>& a, const Vector<T>& b) { return !(a == b); }
template<typename T>
bool operator<(const Vector<T>& a, const Vector<T>& b) {
  for (size_t i = 0; i < a.size() && i < b.size(); ++i) {
    if (a[i] < b[i]) return true;
    if (b[i] < a[i]) return false;
  }
  return a.size() < b.size();  // Shorter prefix is less
}
template<typename T>
bool operator>(const Vector<T>& a, const Vector<T>& b) { return b < a; }
template<typename T>
bool operator<=(const Vector<T>& a, const Vector<T>& b) { return !(b < a); }
template<typename T>
bool operator>=(const Vector<T>& a, const Vector<T>& b) { return !(a < b); }

// =============================================================================
// MAIN (demo)
// =============================================================================
int main() {
  using namespace std;




  Vector<int> v(10);

  for(size_t i = 0; i < v.size(); ++i) v.push_back(i * 12);
  for(size_t j = 0; j < v.size(); ++j) cout << v[j] << " ";

  // Vector<string> v;           // Default-construct empty vector
  // v.reserve(10);             // Pre-allocate for 10 elements (avoids realloc in loop)
  // for (int i = 0; i < 10; ++i)
  //   v.push_back("hdhjyre");  // push_back const char* (string is constructed)

  // for (size_t i = 0; i < v.size(); ++i)
  //   cout << v[i] << " ";
  // cout << endl;

  // Vector<int> from_list = { 1, 2, 3, 4, 5 };  // Initializer-list constructor
  // for (auto it = from_list.begin(); it != from_list.end(); ++it)
  //   cout << *it << " ";
  // cout << endl;

  // Vector<string> copied(v);  // Copy constructor
  // cout << "copied.size() = " << copied.size() << ", v.size() = " << v.size() << endl;

  // v.erase(4);                // Erase element at index 4
  // cout << "after erase(4), size = " << v.size() << endl;

  // v.resize(20, "resized");   // Resize to 20, new slots filled with "resized"
  // cout << "after resize(20, \"resized\"), size = " << v.size() << endl;

  // v.shrink_to_fit();         // Release extra capacity
  // cout << "after shrink_to_fit(), capacity = " << v.capacity() << endl;

  return 0;
}
