#include <iostream>


using namespace std;

template<typename T>
class Vector {

private:
  T* data_;
  size_t size_;
  size_t cap_;

  void resize() {
    size_t newCap_ = cap_ == 0 ? 1 : cap_ * 2;
    T* new_data = new T[newCap_];
    for (size_t i = 0; i < size_; ++i)
    {
      new_data[i] = data_[i];
    }

    delete[] data_;
    data_ = new_data;
    cap_ = newCap_;
  }

public:
  Vector() : data_(nullptr), size_(0), cap_(0) {};
  ~Vector(){
    delete[] data_;
  }


  void push_back(T num) {
    if (size_ == cap_) {
      cout << "Resize called " << num << endl;
      resize();
    }
      
    data_[size_++] = num;
  }

  T operator[](int index) {
    return data_[index];
  }


};

int main() {


  

  
  

 

  // int arr[] = {12, 3, 4, 5};

  // arr[4] = 123;

  // cout << arr[3] << endl;
 











  return 0;
}