#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// Rule of 0: IntBuffer owns its data via std::vector<int> instead of a
// raw int*. std::vector already correctly implements destructor, copy
// ctor, copy assignment, move ctor, and move assignment -- so IntBuffer
// declares NONE of them. The compiler-generated versions just call
// std::vector's versions member-wise, which is exactly correct: copying
// an IntBuffer deep-copies the vector automatically; moving one steals
// the vector's internal buffer automatically; destroying one frees the
// vector's heap array automatically.
class IntBuffer
{
public:
explicit IntBuffer(std::size_t n) : buf_(n) { // std::vector's constructor allocates and zero-initializes the buffer
        //std::cout << "IntBuffer of size " << n << " constructed.  acquired " << *this << "\n";
    }

    int& operator[](std::size_t i) { return buf_[i]; }
    const int& operator[](std::size_t i) const { return buf_[i]; }
    std::size_t size() const { return buf_.size(); }
    const int* data() const { return buf_.data(); } // just for observing identity below



    private:
    std::vector<int> buf_;
    // No destructor, no copy ctor/assignment, no move ctor/assignment.
    // Lessons 01-03 hand-wrote all five; here there are zero, and this
    // version is MORE correct than what we wrote by hand -- std::vector's
    // real implementation has had decades of scrutiny ours never got.
};

void demo_copy_and_move() {
    std::cout << "\n-- construct a --\n";
    IntBuffer a(4);
    a[0] = 42;
    std::cout << "a.data() = " << static_cast<const void*>(a.data()) << "\n";

    std::cout << "\n-- IntBuffer b = a;  (copy) --\n";
    IntBuffer b = a;
    std::cout << "b.data() = " << static_cast<const void*>(b.data())
              << " (different from a's -- deep copy happened, with zero code of ours)\n";

    std::cout << "\n-- IntBuffer c = std::move(a);  (move) --\n";
    const void* a_addr_before = a.data();
    IntBuffer c = std::move(a);
    std::cout << "c.data() = " << static_cast<const void*>(c.data())
              << " (SAME as a's old address " << a_addr_before << " -- moved, not copied)\n";
    std::cout << "a.size() after being moved from = " << a.size() << "\n";
}

int main() {
    std::cout << "Lesson: Rule of 0" << std::endl;
    demo_copy_and_move();
    return 0;
}