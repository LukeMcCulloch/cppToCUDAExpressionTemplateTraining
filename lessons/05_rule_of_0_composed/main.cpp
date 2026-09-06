#include <cstddef>
#include <iostream>
#include <utility>

// EXACTLY lesson 03's IntBuffer, unchanged -- a correctly rule-of-5'd
// owning primitive. Kept instrumented on purpose: composing a
// higher-level type out of THIS (rather than std::vector, as in lesson
// 04) means we keep that print visibility for free.
#include <cstddef>
#include <iostream>
#include <utility>

// EXACTLY lesson 03's IntBuffer, unchanged -- a correctly rule-of-5'd
// owning primitive. Kept instrumented on purpose: composing a
// higher-level type out of THIS (rather than std::vector, as in lesson
// 04) means we keep that print visibility for free.
class IntBuffer {
public:
    explicit IntBuffer(std::size_t n) : size_(n), data_(new int[n]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = 0;
    }

    // Copy constructor: allocate OUR OWN buffer, then copy elements over.
    // Runs when constructing a new object from an existing one, e.g.
    // `IntBuffer b = a;` or passing/returning by value.
    IntBuffer(const IntBuffer& other)
        : size_(other.size_), data_(new int[other.size_]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        std::cout << "  [IntBuffer] copy-constructed size=" << size_ << "\n";
    }

    // Copy assignment: `this` already owns a buffer from earlier:
    // e.g. IntBuffer b; already happened before b = a;
    // it must release it before taking on a new one, and must guard against
    // self-assignment (`a = a;`), which would free the buffer out from
    // under itself before trying to copy from it.
    IntBuffer& operator=(const IntBuffer& other) {
        if (this == &other) return *this;
        delete[] data_;
        size_ = other.size_;
        data_ = new int[size_];
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        std::cout << "  [IntBuffer] copy-assigned size=" << size_ << "\n";
        return *this;
    }

    //  move constructor, move assignment
    // Move constructor: steal other's buffer, then null it out so the destructor won't free it.
    // Move assignment: release what `this` currently owns, then steal other's buffer.
    // Self-move guard matters for the same reason self-copy did: without
    // it, `a = std::move(a);` would delete `this`'s buffer 
    // and then try to steal from the very object it just deleted.
    IntBuffer(IntBuffer&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "  [IntBuffer] move-constructed size=" << size_ << "\n";
    }

    // Move assignment: release what `this` currently owns, then steal other's buffer.
    // Self-move guard matters for the same reason self-copy did: without
    // it, `a = std::move(a);` would delete `this`'s buffer 
    // and then try to steal from the very object it just deleted.
    IntBuffer& operator=(IntBuffer&& other) noexcept {
        if (this == &other) return *this;
        delete[] data_;
        size_ = other.size_;
        data_ = other.data_;
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "  [IntBuffer] move-assigned size=" << size_ << "\n";
        return *this;
    }

    ~IntBuffer() {
        std::cout << "  [IntBuffer] destructing size=" << size_ << "\n";
        delete[] data_;
    }

    int& operator[](std::size_t i) { return data_[i]; }
    const int& operator[](std::size_t i) const { return data_[i]; }
    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};

// IntMatrix: built ON TOP OF IntBuffer. Declares NONE of the five special
// members -- all five are compiler-generated, and all five are correct,
// because each one just does the obvious member-wise thing: run
// IntBuffer's already-correct version on `data_`, and trivially copy/move
// the two std::size_t members. Same Rule-of-0 principle as lesson 04's
// std::vector version -- except this time the "already-correct primitive"
// underneath is one you wrote yourself, three lessons ago.
class IntMatrix {

public:
    IntMatrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), data_(rows * cols) {}

    int& operator()(std::size_t row, std::size_t col) {
        return data_[row * cols_ + col];
    }

    const int& operator()(std::size_t row, std::size_t col) const {
        return data_[row * cols_ + col];
    }

    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }


private:
    std::size_t rows_;
    std::size_t cols_;
    IntBuffer data_;
    // No destructor. No copy ctor/assignment. No move ctor/assignment.
    // Zero lines, and correct.
};



int main() {
    std::cout << "-- construct a (2x2) --\n";
    IntMatrix a(2, 2);
    a(0, 0) = 42;

    std::cout << "\n-- IntMatrix b = a;  (copy) --\n";
    IntMatrix b = a; // IntMatrix's compiler-generated copy ctor runs IntBuffer's
                     // REAL copy ctor on data_ -- watch for "[IntBuffer]
                     // copy-constructed" below, from code you wrote in lesson 03.
    b(0, 0) = 99;
    std::cout << "a(0,0)=" << a(0, 0) << " (should still be 42 -- separate storage)\n";

    std::cout << "\n-- IntMatrix c = std::move(a);  (move) --\n";
    IntMatrix c = std::move(a); // same story, IntBuffer's move ctor runs instead
    std::cout << "c(0,0)=" << c(0, 0) << "\n";

    std::cout << "\n-- end of main, watch destructor order --\n";
    return 0;
}