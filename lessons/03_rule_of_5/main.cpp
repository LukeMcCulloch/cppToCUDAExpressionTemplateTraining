#include <cstddef>
#include <iostream>
#include <utility> // std::move
#include <vector>

class IntBuffer {

public:

    //explicit makes sure that the constructor is not used for implicit conversions.  This is a good idea for single-argument constructors, because otherwise you can get some very confusing code.
    // the constructor is called when an IntBuffer is created, 
    // e.g. `IntBuffer a(4);` or `IntBuffer b = IntBuffer(4);`
    /*
    explicit blocks implicit conversions through that constructor. Without it, any constructor callable with exactly one argument doubles as a rule the compiler can use silently to convert that argument's type into your class, wherever a value of your class type is expected — even if you never intended that.

    Concretely, without explicit:

    IntBuffer b = 10;       // "copy-initialization": compiler implicitly builds IntBuffer(10)
    void take(IntBuffer);
    take(10);               // also implicitly allocates a 10-int buffer, just to call the function

    Both of those silently allocate a 10-element heap buffer from a plain integer, with nothing at the call site hinting that a nontrivial, heap-allocating construction just happened. That's a real footgun for a type like this one, precisely because the "conversion" is doing expensive, meaningful work (new int[n]) rather than something cheap and obviously safe.

    With explicit, both of those lines become compile errors — you're forced to say what you mean:

    IntBuffer b(10);        // fine: direct-initialization, clearly constructing on purpose
    IntBuffer b = IntBuffer(10); // also fine, still explicit about it
    take(IntBuffer(10));    // fine

    This is why it's the recommended default 
    (C++ Core Guidelines C.46: "by default, declare single-argument constructors explicit") 
    — you only leave a constructor non-explicit when you deliberately want seamless, implicit conversion from that argument type to be part of the class's design (e.g. std::string(const char*), where converting a C-string into a std::string wherever one's expected is exactly the point). For a resource-owning type like IntBuffer, where the "conversion" means "go allocate memory," that's essentially never what you want to happen by accident — hence explicit here.
        
    */
    explicit IntBuffer(std::size_t n) : size_(n), data_(new int[n]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = 0;
    }

    // rule of 3, deep copy constructor
    IntBuffer(const IntBuffer & other) 
        : size_(other.size_), data_(new int[other.size_]) {

        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    // rule of 3, deep copy assignment operator
    IntBuffer& operator=(const IntBuffer& other) {
        if (this == &other) return *this;// we need to &other because other is a reference, and we want to compare the addresses of the two objects, not the values they refer to.  If we don't do this, we could end up deleting our own (don't you mean other?) data_ and then trying to copy from it, which would be bad.
        delete[] data_;
        size_ = other.size_;
        data_ = new int[size_];
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        return *this;// dereferenced pointer to the current object, so that assignment can be chained (e.g. a = b = c).
    }

    // --- NEW for this lesson: move constructor, move assignment ---

    // MOVE CONSTRUCTOR
    // STEAL other's buffer -- no new allocation, no element copy. `noexcept`
    // is not just documentation here: std::vector only uses a type's move
    // constructor during reallocation if it's marked noexcept; otherwise it
    // falls back to copying every element instead. Why: if a move could
    // throw partway through relocating N elements, the vector could be left
    // half-moved with no way to recover. Copying is safe to abandon
    // mid-operation (the originals are untouched); moving generally isn't.
    //
    // Move constructor: steal other's buffer, then null it out so the destructor won't free it.
    IntBuffer(IntBuffer&& other) noexcept : 
        size_(other.size_), data_(other.data_) { // move (aka steal), don't replicate
        other.size_ = 0;
        other.data_ = nullptr; // null out other's pointer so its destructor won't free the buffer we just stole
        // leaves other in a valid but unspecified state (size_ = 0, data_ = nullptr), 
        // empty and safe to destruct, but not useful for anything else.  
        // This is the standard pattern for move constructors.
    }

    // MOVE ASSIGNMENT: 
    // Release what `this` currently owns, then steal other's buffer.
    // Self-move guard matters for the same reason self-copy did: without
    // it, `a = std::move(a);` would delete `this`'s buffer and then try
    // to steal from the very object it just deleted.
    IntBuffer& operator=(IntBuffer&& other) noexcept { // : size_(other.size_), data_(other.data_) { // move (aka steal), don't replicate
        if (this == &other) return *this; // self-move guard: if we' a're trying to move-assign from ourselves, do nothing (we already own our own buffer, and we don't want to delete it and then try to steal from ourselves, which would be bad)
        delete[] data_; // release what we currently own, if anything
        size_ = other.size_;
        data_ = other.data_;
        other.size_ = 0;
        other.data_ = nullptr; // null out other's pointer so its destructor won't free the buffer we just stole
        return *this;

    }

    // & Lvalue reference to the current object, so that assignment can be chained (e.g. a = b = c).  
    //      This is the same as the copy assignment operator, but for move assignment.
    //
    // && Rvalue reference to the other object, so that we can steal its buffer.  
    //      This is the same as the move constructor, but for move assignment.

    /*
    Now the actual question — could the stealing code just live in one ordinary operator=, without && at all? 
        Mechanically, yes — nothing about "steal the pointer, null out the source" requires && syntactically; you could write that logic inside operator=(IntBuffer& other) (plain non-const lvalue reference) just fine. The problem isn't that it's impossible — it's that doing so would make stealing the only behavior = ever has, 
        for every assignment, with no way to ask for the safe one.
    

    "&&" is there to distinguish the two cases: if the caller has an lvalue (a named variable), they get the copy assignment operator; if they have an rvalue (a temporary, or std::move), they get the move assignment operator. 
    The compiler uses the reference type to decide which operator= to call, 
    so you can have both behaviors coexist in the same class.

    c = a; // calls the copy assignment operator, because a is an lvalue (named variable)
    d = std::move(c); // calls the move assignment operator, because std::move(c) is an rvalue (temporary)

    
    */



    ~IntBuffer() {
        delete[] data_;
    }

    int& operator[](std::size_t i) { return data_[i]; }
    const int& operator[](std::size_t i) const { return data_[i]; }
    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};



    void demo_move_vs_copy() {
        std::cout << "\n-- construct a --\n";
        IntBuffer a(4);
        a[0] = 42;
        
        std::cout << "\n-- IntBuffer b = a;  (a is an lvalue -> copy) --\n";
        IntBuffer b = a;

        std::cout << "\n-- IntBuffer c = std::move(a);  (forced move) --\n";
        IntBuffer c = std::move(a);
        std::cout << "a.size() after being moved from = " << a.size() << "\n";

        std::cout << "\n-- d = c;  (c is an lvalue -> copy assignment) --\n";
        IntBuffer d(1);
        d = c;
        
        std::cout << "\n-- d = std::move(c);  (move assignment) --\n";
        d = std::move(c);

    }



void demo_vector_growth() {
    std::cout << "\n-- vector<IntBuffer> growth: watch what happens to the FIRST element --\n";
    std::vector<IntBuffer> v;
    v.reserve(1); // deliberately tiny, so the 2nd emplace_back must reallocate
    v.emplace_back(2);
    std::cout << "-- pushing a 2nd element now, forcing reallocation --\n";
    v.emplace_back(2);
}


int main() {
    std::cout << "Lesson: Rule of 5" << std::endl;
    demo_move_vs_copy();
    demo_vector_growth();
    return 0;
}