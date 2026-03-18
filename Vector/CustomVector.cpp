#include <iostream>
#include <string>


using namespace std;
template<typename T>
class Vector{

private:
  T* data;
  size_t sz;
  size_t cap;

  void resize(){
    size_t newcap = cap == 0 ? 1 : cap * 2;
    T* newdata = new T[newcap];
    for(size_t i = 0; i < sz; ++i){
      newdata[i] = move(data[i]);
    }
    delete[] data;
    data = newdata;
    cap = newcap;
  }
public:
  Vector(size_t s): sz(s), cap(s * 2), data(new T[cap]){};
  Vector(): data(nullptr), sz(0), cap(0){};
  // Vector(size_t size = 0): data(size ? new T[size] : nullptr), size
  //Deep Copy Constructor that duplicates underlying array
  Vector(const Vector& other): data(nullptr), sz(other.sz), cap(other.cap){
    if(cap > 0){
      data = new T[cap];
      for(size_t i = 0; i < sz; ++i){
        data[i] = other.data[i];
      }
    }
  cout << "Copy Constructor was called" << endl;
  } 

  Vector(Vector&& other) noexcept : sz(other.sz), cap(other.cap), data(other.data) {
    other.sz = 0;
    other.cap = 0;
    other.data = nullptr;
    cout << "Move was Called" << endl;
  }

  Vector& operator=(Vector&& other){
    if(this != &other){
      delete[] data;
      data = other.data;
      sz = other.sz;
      cap = other.cap;

      other.data = nullptr;
      other.sz = 0;
      other.cap = 0;
    }
    return *this;
  }

  void swapper(Vector& v1, Vector& v2) noexcept {
    swap(v1.data, v2.data);
    swap(v1.sz, v2.sz);
    swap(v1.cap, v2.cap);
  }

  Vector& operator=(Vector other) noexcept {
    swapper(*this, other);
    return *this;
    // if(this == &other) return *this;
    // delete[] data;
    // sz = other.sz;
    // cap = other.cap;
    
    // if (cap > 0) {
    //   data = new T[cap];
    //   for(size_t i = 0; i < other.sz; ++i){
    //     data[i] = other.data[i];
    //   }
    // } else {
    //   data = nullptr;
    // }
    // return *this;
  }

  ~Vector(){delete[] data;};



  // void swap(Vector& other){
  //   swap(data, other.data);
  //   swap(sz, other.sz);
  //   swap(cap, other.cap);
  // }

  // Vector& operator=(Vector other){
  //   swap(other);
  //   return *this;
  // }

  void erase(int index){
    if(index >= sz){
      throw out_of_range("Out of range");
    }
    
    data[index].~T();
    for(int i = index; i < sz - 1; ++i){
      data[i] = move(data[i + 1]);
    }
    sz--;
  }

  void push_back (T value){
    if(sz == cap) resize();
    data[sz++] = value;
  }

  size_t capacity() const{
    return cap;
  }

  void pop_back(){
    if(sz > 0) --sz;
  }

  bool empty() const{
    if(sz == 0) return true;
    return false;
  }

  //operator[] is a special member function that lets you use the square bracket syntax ([]) on objects of your class.
  T& operator[](size_t index){
    if(data == nullptr){
      cout << "Segmentation fault" << endl;
    }
    return data[index];
  }

  const T& operator[](size_t index) const{
    return data[index];
  }
  
  T& at(size_t index){
    //Bounds checking
    if(index >= sz) throw std::out_of_range("Index out of range");
    return data[index];
  }
  const T& at(size_t index) const {
    //Bounds checking
    if(index >= sz) throw std::out_of_range("Index out of range");
    return data[index];
  }

  const T& front() const {
    if(sz == 0) throw std::out_of_range("Vector is empty");
    return data[0];
  }

  const T& back() const {
    if(sz == 0) throw std::out_of_range("Vector is empty");
    return data[sz - 1];
  }

  size_t size() const {return sz;}
};


int main() {


 
  Vector<string> st;
  for(int i = 0; i < 10; ++i){
    st.push_back("hdhjyre");
  }

  // st.erase(4);
  for(size_t i = 0; i < st.size(); ++i) cout << st[i] << "  ";

  cout << endl;

  string s = "4252";
  s.~string();

  cout << s << endl;









  return 0;
}