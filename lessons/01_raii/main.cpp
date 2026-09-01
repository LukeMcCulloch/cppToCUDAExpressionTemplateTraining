#include <cstddef> // for std::size_t
#include <iostream> // for std::cout, std::endl




// RAII in its purest form: acquire in the constructor, release in the
// destructor. IntBuffer owns a heap array of ints for its whole lifetime.
class IntBuffer
{

public:

    explicit IntBuffer(std::size_t n) : size_(n), data_(new int[n]) 
    {
        // members are initialized in the order they're declared in the class (size_ then data_ here), 
        // not the order they appear in the list
        std::cout << "IntBuffer of size " << " " << n << " to be constructed." << std::endl;

        for (std::size_t i = 0; i < size_; ++i) data_[i] = 0; // initialize to zero
        std::cout << "aquired " << size_ << " int at " << data_ << "\n"; // why no endl?  (if we wait and flush more prints at once, we can see the order of construction and destruction more clearly)??
        std::cout << "IntBuffer of size " << " " << n << " constructed." << std::endl;

    }

    ~IntBuffer() 
    {
        std::cout << "IntBuffer of size " << " " << size_ << " to be destructed." << std::endl;
        delete[] data_; // release the heap array, c++17 guarantees that delete[] is safe to call on nullptr.
        std::cout << "released " << size_ << " int at " << data_ << "\n"; // why no endl?
        std::cout << "IntBuffer of size " << " " << size_ << " destructed." << std::endl;
    }

    int &operator[](std::size_t i) { return data_[i]; } // return reference to the i-th element of the buffer // why not the element itself?  why not const? 
    std::size_t size() const { return size_;  } // return the size of the buffer

private:

    std::size_t size_;
    int* data_;
    // No copy constructor/assignment, no move constructor/assignment
    // written here. The compiler generates all four for us by default.
    // That generated copy is about to do something you don't want --
    // watch closely below.
};

void demo_copy_bug() 
{

    IntBuffer a(4);
    a[0] = 42;

    IntBuffer b = a;    // compiler-generated copy constructor is called here, which does a shallow copy of the data_ pointer... meaning that:
                        // copies size_ and the raw pointer data_ member-by-member, 
                        // so now a and b both point to the same heap array of ints.  
                        // When a and b go out of scope, they will both try to delete[] the same pointer, which is undefined behavior.

    b[0] = 99;
    std::cout << "a[0]=" << a[0] << " (changing b changed a too)\n"; // why no endl again?  because the next line is a cout, and it will flush the buffer anyway.  
    // but if you want to be explicit, you can add endl.


} // scope ends, a and b are destructed, and the heap array is deleted twice, which is undefined behavior.  
// The program may crash, or it may not.  It may even appear to work correctly sometimes, but it's still a bug.
//----------------
// The lesson here is that you should not rely on the compiler-generated copy constructor for classes that manage resources (like heap memory).
//----------------
// scope ends: b's destructor runs delete[] on the shared pointer, then
  // a's destructor runs delete[] on the SAME pointer again -- a double
  // free. Undefined behavior: might crash here, might corrupt the heap
  // silently and crash somewhere unrelated later, might "work" by luck.

  int main() 
  {
      demo_copy_bug();
      return 0;
  }