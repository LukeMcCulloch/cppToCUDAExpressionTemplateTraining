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
        std::cout << "aquired " << size_ << " int at static_cast<void*>(data_):" << static_cast<void*>(data_) << "\n"; 
        std::cout << "aquired " << size_ << " int at " << data_ << "\n"; // use endl to flush the buffer (here we obviously do not flush the buffer)
        std::cout << "IntBuffer of size " << " " << n << " constructed." << std::endl;
        std::cout << "acquired " << *this << "\n";
        std::cout << "vanilla constructor done.  acquired " << *this << "\n";
    }

    // Copy constructor: allocate OUR OWN buffer, then copy elements over.
    // Runs when constructing a new object from an existing one, e.g.
    // `IntBuffer b = a;` or passing/returning by value.
    //vThis is what fixes the double-free from lesson 01 :
    // a and b now own separate heap allocations, 
    // so each destructor only ever frees its own.
    IntBuffer(const IntBuffer& other)
        : size_(other.size_), data_(new int[other.size_]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        std::cout << "copy-constructed " << size_ << " ints at " << data_
                << " (from " << other.data_ << ")\n";
        std::cout << "copy constructor done.  acquired " << *this << "\n";
    }

    

    // Copy assignment: `this` already owns a buffer from earlier: 
    // e.g. IntBuffer b; already happened before b = a;
    // it must release it before taking on a new one, and must guard against
    // self-assignment (`a = a;`), which would free the buffer out from
    // under itself before trying to copy from it.
    //
    // not that copy assignment is able to resize the data array
    IntBuffer& operator=(const IntBuffer& other) {
        if (this == &other) return *this;

        delete[] data_;
        size_ = other.size_;
        data_ = new int[size_];
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        std::cout << "copy-assigned " << size_ << " ints at " << data_ << "\n";
        std::cout << "copy assignment done.  acquired " << *this << "\n";
        return *this;
    }

    ~IntBuffer() 
    {
        std::cout << "IntBuffer of size " << " " << size_ << " to be destructed." << std::endl;
        delete[] data_; // release the heap array, c++ 98 onwards guarantees that delete[] is safe to call on nullptr.
        std::cout << "released " << size_ << " int at " << data_ << "\n"; // data_ is dangling
                        // now, but printing its ADDRESS value (not dereferencing it) is fine.
                        
        std::cout << "released " << size_ << " int at " << static_cast<void*>(data_) << "\n";
        std::cout << "IntBuffer of size " << " " << size_ << " destructed." << std::endl;
        /*
        static_cast<void*>(data_) converts the type the compiler treats data_ as 
        from int* to void* 
        - without touching the actual bits, 
            the address stays exactly the same value either way. 
        - void* is C++'s "generic pointer" type: it can hold any object pointer's value 
            but has lost the information about what type it points to 
            (you can't dereference a void* or do pointer arithmetic on it for exactly that reason 
        - the compiler no longer knows the pointee's size).
        */

    }




    // this element accessor returns a refernce so callers of it 
    // can both read and write though it.  
    // It isn't const so it can allow writes.
    //
    // a real implementation would also add a
    // `const int& operator[](std::size_t) const` overload so a `const
    // IntBuffer` can still be read from just not written to. Not added
    // yet -- worth doing as a small follow-up exercise.
    //
    int &operator[](std::size_t i) { return data_[i]; } // return reference to the i-th element of the buffer // why not the element itself?  why not const? 
    
    // `const int& operator[](std::size_t) const` overload so a `const
    // IntBuffer` can still be read from just not written to. 
    const int& operator[](std::size_t i) const { return data_[i]; }

    std::size_t size() const { return size_;  } // return the size of the buffer


    // public gettr and setter for the data_ pointer, for demonstration purposes only.  In real code, you would not expose the raw pointer like this.
    int* data() const { return data_; }

	// access control is per-class, not per-object 
    // so this method can access the private data_ of any IntBuffer, not just this one.
    bool shares_storage_with(const IntBuffer& other) const { return data_ == other.data_; }

	// friend declaration: this function is not a member of IntBuffer, 
    // but it can access its private members.
	// tinkering around with this function is a good way to see how 
    // the copy constructor and assignment operator work, 
    // and to see the effects of shallow vs deep copies.
    friend std::ostream& operator<<(std::ostream&, const IntBuffer&); // just a declaration


private:

    std::size_t size_;
    int* data_;
    // Copy constructor and copy assignment ARE user-defined above (Rule
    // of 3 satisfied). Move constructor and move assignment are still
    // NOT written -- and because a destructor + copy ctor + copy
    // assignment are all user-declared, the compiler will NOT implicitly
    // generate move operations for us either. Every situation that could
    // have used a cheap move (returning an IntBuffer by value without
    // NRVO kicking in, a std::vector<IntBuffer> reallocating, ...)
    // silently falls back to a full deep copy instead. That's the Rule
    // of 5 lesson still to come.
};

std::ostream& operator<<(std::ostream& os, const IntBuffer& b) {
    os << "IntBuffer(size=" << b.size_
        << ", data_=" << static_cast<const void*>(b.data_) << ") = [";
    for (std::size_t i = 0; i < b.size_; ++i) {
        os << b.data_[i];
        if (i + 1 < b.size_) os << ", ";
    }
    return os << "]";
}

void demo_copy_bug() 
{

    IntBuffer a(4);
    a[0] = 42;

    IntBuffer b = a;    // <-- COPY CONSTRUCTOR INVOKED HERE (the one defined
                        // above, not a compiler-generated one anymore).
                        // Unlike lesson 01, this allocates b its OWN buffer
                        // and copies the values across -- a and b no longer
                        // share storage.

    

    // print a=?=b?
	// in this sample code, we have an explicit copy constructor, 
    // so a and b do NOT share the same storage.  
    // If you comment out the copy constructor, 
    // then a and b will share the same storage, 
    // which is a bug.
    std::cout << "a and b share storage: " << (a.data() == b.data() ? "yes" : "no") << "\n";
    std::cout << "a and b share storage: " << (a.shares_storage_with(b) ? "yes" : "no") << "\n";

    b[0] = 99;
    std::cout << "a[0]=" << a[0] << " (should still be 42 -- separate storage now)\n";

} // Both destructors now run cleanly: each object owns an independent
  // allocation, so there is no double free here (contrast with lesson 01,
  // where this exact shape of code corrupted the heap and aborted).


// FIXED:
// scope ends, a and b are destructed, and the heap array is deleted twice, which is undefined behavior.  
// The program may crash, or it may not.  It may even appear to work correctly sometimes, but it's still a bug.
//----------------
// The lesson here is that you should not rely on the compiler-generated copy constructor for classes that manage resources (like heap memory).
//----------------
// scope ends: b's destructor runs delete[] on the shared pointer, then
// a's destructor runs delete[] on the SAME pointer again -- a double
// free. Undefined behavior: might crash here, might corrupt the heap
// silently and crash somewhere unrelated later, might "work" by luck.
// NOW THAT IS ALL FIXED BY HAVING A COPY CONSTRUCTOR



int main() {
    std::cout << "Lesson: Rule of 3" << std::endl;
    demo_copy_bug();

    IntBuffer c(2);
    IntBuffer a(4);
    a[0] = 42;
    c = a;   // <-- COPY ASSIGNMENT INVOKED HERE (operator= defined above).
            // c already exists, so this is assignment, not construction.
            // The self-assignment guard (`this == &other`) doesn't trigger
            // since c and a are different objects.
    std::cout << "c[0]=" << c[0] << "\n";
    return 0;
}

// old output:

/*

IntBuffer of size  4 to be constructed.
aquired 4 int at 0000013EC77CAAF0
IntBuffer of size  4 constructed.       <- this whole block is only `a`. Notice there is no second "to be constructed/acquired/constructed" sequence anywhere for `b`.
a[0]=99 (changing b changed a too)      <- confirms a and b share one buffer
IntBuffer of size  4 to be destructed.  <- b's destructor (b is destroyed first: locals
released 4 int at 0000013EC77CAAF0          unwind in REVERSE order of construction, so
IntBuffer of size  4 destructed.            the second-declared variable goes first)
IntBuffer of size  4 to be destructed.  <- a's destructor starts...
//                                          ...and then nothing. No "released", no "destructed" for a. The program has crashed before it could finish a's destructor.
//                                          no "destructed". The program died mid-destructor.
//

The implicit copy constructor copies the pointer too.  For some reason they call this a shallow copy.  
The copy constructor is not doing a deep copy of the heap array, so both `a` and `b` point to the same array.  When `b` is destructed, it deletes the array, and then when `a` is destructed, it tries to delete the same array again, which is undefined behavior.  
This is a classic example of why you need to define your own copy constructor (and assignment operator) for classes that manage resources.

*/

// new output:

/*

IntBuffer of size  4 to be constructed.
aquired 4 int at 0x5700060162b0
IntBuffer of size  4 constructed.
copy-constructed 4 ints at 0x5700060166e0 (from 0x5700060162b0)
a[0]=42 (changing b changed a too)
IntBuffer of size  4 to be destructed.
released 4 int at 0x5700060166e0
IntBuffer of size  4 destructed.
IntBuffer of size  4 to be destructed.
released 4 int at 0x5700060162b0
IntBuffer of size  4 destructed.
IntBuffer of size  2 to be constructed.
aquired 2 int at 0x5700060162b0
IntBuffer of size  2 constructed.
IntBuffer of size  4 to be constructed.
aquired 4 int at 0x5700060166e0
IntBuffer of size  4 constructed.
copy-assigned 4 ints at 0x5700060162b0
c[0]=42
IntBuffer of size  4 to be destructed.
released 4 int at 0x5700060166e0
IntBuffer of size  4 destructed.
IntBuffer of size  4 to be destructed.
released 4 int at 0x5700060162b0
IntBuffer of size  4 destructed.

*/