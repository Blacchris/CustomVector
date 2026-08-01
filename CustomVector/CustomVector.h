#pragma once
#include <iostream>
#include <initializer_list>


using namespace std;

template<typename T>
class Vector {

private:

    using  value_type         = T;
    using  reference          = T&;
    using  const_reference    = const T&;
    using  const_iterator     = const T*;
    using  size_type          = size_t;

    value_type* data_;
    size_type size_;
    size_type capacity_;
    

    void resize() {
        size_type newCap_ = capacity_ == 0 ? 1 : capacity_ * 2;
        value_type* newdata_ = new value_type[newCap_];
        for (size_type i {}; i < size_; ++i) {
            newdata_[i] = data_[i];
        }
        delete[] data_;
        data_ = newdata_;
        capacity_ = newCap_;
    }


    void reserve(size_type newCap_) {
        if (newCap_ <= capacity_) return;
        value_type* newdata_ = new value_type[newCap_];
        if (data_ != nullptr) {
            for (size_type i{}; i < size_; ++i) newdata_[i] = data_[i];
        }
        delete[] data_;
        data_ = newdata_;
        capacity_ = newCap_;
    }
  

public:
  Vector() : data_(nullptr), size_(0), capacity_(0) {}

  Vector(const Vector<value_type>& other) : size_(other.size_), capacity_(other.capacity_) {
    data_ = new T[capacity_];
    for (size_type i {}; i < size_; ++i) {
      data_[i] = other.data_[i];
    }
  }

  Vector(const initializer_list<value_type>& items) :  data_(nullptr), size_(0), capacity_(0){
    reserve(items.size());
    for (auto&item : items) {
      push_back(item);
    }
  }

  Vector(const size_type count_, const value_type _val) : data_(nullptr),  size_(0), capacity_(0){
    reserve(count_);
    for (size_type i{}; i < capacity_; ++i) {
      push_back(_val);
    }
  }

  Vector& operator=(const Vector<value_type>& other) {
    if (this == &other) return *this;

    delete[] data_;

    size_ = other.size_;
    capacity_ = other.capacity_;

    data_ = new value_type[capacity_];
    for (size_type i {}; i < size_; ++i) {
      data_[i] = other.data_[i];
    }
    return *this;
  }

  ~Vector(){  
    // clear();
    delete[] data_;
  }


  void push_back(value_type val) {
    if (size_ == capacity_)resize();
    data_[size_++] = val;
  }

  value_type pop_back() {
    if (size_ == 0)
        throw std::out_of_range("pop_back on empty Vector");
    value_type val = data_[size_ - 1];
    size_--;
    return val;
  }

  reference operator[](int index) {
    return data_[index];
  }

  
  void clear() {
    if(size_ == 0) return;
    for (size_type i {}; i < size_; ++i) {
      data_[i].~value_type();
    }
    size_ = 0;
  }

  constexpr size_type
  capacity() const noexcept { return capacity_; }

  constexpr size_type
  size() const noexcept { return size_; }


  void inspect() {
    value_type* ptr = data_;
    if (ptr != nullptr) {
      for (size_type i {}; i < size_; ++i) {
        cout << "Address: " << ptr + i << " -> Value: " << *(ptr + i) << endl;
      }
    }
  }

}; 



