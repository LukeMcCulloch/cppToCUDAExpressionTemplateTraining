#include <iostream>
#include <cstddef>

// --- Function template ---
// The compiler generates a separate, real function for each concrete
// type T actually used, by substituting T with that type. "Template
// argument deduction" figures out T from the arguments you pass, so you
// usually don't have to spell it out.
template <typename T>
T my_max(T a, T b) {
    return (a > b) ? a : b;// the compiler generates a separate, real function 
    // for each concrete type T actually used, by substituting T with that type. 
    // "Template argument deduction" figures out T from the arguments you pass, 
    // so you usually don't have to spell it out.
    // how it works when objects are different types, e.g. int and double?  
    // The compiler will try to find a common type that both can be converted to, 
    // and use that as T.  If no common type can be found, it will result in a compilation error.
}


// --- Class template ---
// Same idea, for a whole class: T is a placeholder filled in when you
// name a concrete type. Box<int> and Box<double> are two DIFFERENT,
// unrelated types stamped out from the same source.
template <typename T>
class Box {
public:
    explicit Box(T value) : value_(value) {}
    T get() const { return value_; }
private:
    T value_;
};



// --- Non-type template parameter ---
// Template parameters aren't limited to types -- a compile-time constant
// VALUE can be one too. N becomes part of the TYPE itself:
// FixedArray<int,3> and FixedArray<int,4> are different types, and N is
// known at compile time everywhere inside the class -- enough to size a
// plain C array with no heap allocation at all. This is exactly the
// pattern behind the FixedTape<MaxOps> design from the very first CUDA
// AD sketch, back at the start of this whole project -- recognize it
// here in its simplest form.
template <typename T, std::size_t N>
class FixedArray {
public:
    T& operator[](std::size_t i) { return data_[i]; } // no new needed: 
    // N is known at compile time, 
    //      so the compiler can lay out T data_[N] directly inside the object's own storage
    // -- no heap allocation, no delete needed anywhere --
    // Since the compiler already knows exactly how big the array needs to be, 
    //      it can lay out T data_[N] directly inside the object's own storage
    const T& operator[](std::size_t i) const { return data_[i]; }
    std::size_t size() const { return N; } // N is compile-time -- no member needed to store it
private:
    T data_[N]; // stack storage, fixed at compile time, no new/delete anywhere
};



int main() {
    std::cout << "my_max(3, 7)           = " << my_max(3, 7) << "  (T deduced as int)\n";
    std::cout << "my_max(3.5, 2.1)       = " << my_max(3.5, 2.1) << "  (T deduced as double)\n";
    std::cout << "my_max<double>(3, 7.5) = " << my_max<double>(3, 7.5) << "  (T given explicitly)\n";

    Box<int> ibox(42); // explicit <int>
    std::cout << "\nBox<int>(42).get() = " << ibox.get() << "\n";

    Box bbox(3.14); // CTAD (C++17): compiler deduces Box<double> from the constructor argument
    std::cout << "Box(3.14).get()    = " << bbox.get() << "  (CTAD deduced Box<double>)\n";

    FixedArray<int, 3> arr; // N=3 baked into the type at compile time
    arr[0] = 10; arr[1] = 20; arr[2] = 30;
    std::cout << "\nFixedArray<int,3>: size=" << arr.size()
              << " contents=[" << arr[0] << "," << arr[1] << "," << arr[2] << "]\n";


    // memory notes:
    FixedArray<int,3> a;                     // 'a' is a local variable -> stack. data_[3] is embedded in it -> also stack.
    auto* p = new FixedArray<int,3>();       // the WHOLE object is heap-allocated by `new` -> data_[3] is on the HEAP too, embedded in that same block.
    struct Holder { FixedArray<int,3> arr; }; // if a Holder is heap-allocated, arr's storage is wherever Holder's storage is.
    return 0;
}