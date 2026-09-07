#include <iostream>
#include <memory>
#include <utility>


// A simple class to demonstrate ownership semantics with smart pointers.
struct Widget {
    int id;
    explicit Widget(int i) : id(i) { std::cout << "  Widget " << id << " constructed\n"; }
    ~Widget() { std::cout << "  Widget " << id << " destructed\n"; }
    void greet() const { std::cout << "  Widget " << id << " says hi\n"; }
};

// Demonstrates std::unique_ptr: exclusive ownership, no copies allowed.
void demo_unique_ptr() {
    std::cout << "-- unique_ptr: exclusive ownership --\n";
    std::unique_ptr<Widget> p1 = std::make_unique<Widget>(1);
    p1->greet();

    // std::unique_ptr<Widget> p2 = p1; // COMPILE ERROR if uncommented:
    // unique_ptr's copy constructor is explicitly deleted -- "only one
    // owner" is enforced by the type system, not just a convention.

    std::unique_ptr<Widget> p2 = std::move(p1); // fine: transfers ownership
    std::cout << "  p1 is now " << (p1 == nullptr ? "null" : "NOT null") << " after the move\n";
    p2->greet();
} // only ONE destructor runs here, for whichever one currently owns the
  // Widget (p2) -- no double free, because unique_ptr already implements
  // the exact rule-of-5 logic you hand-wrote for IntBuffer, once, correctly.

// A custom deleter -- the exact shape you'll reach for with CUDA device
// memory later. Instead of a plain `delete`, unique_ptr calls whatever
// callable you give it. Standing in for cudaFree with a print statement
// so this lesson stays buildable without nvcc.
struct FakeCudaDeleter {
    void operator()(Widget* p) const {
        std::cout << "  [FakeCudaDeleter] would call cudaFree here for Widget " << p->id << "\n";
        delete p;
    }
};

void demo_custom_deleter() {
    std::cout << "\n-- unique_ptr with a custom deleter (CUDA-shaped) --\n";
    std::unique_ptr<Widget, FakeCudaDeleter> dp(new Widget(2));
    dp->greet();
    // when dp goes out of scope, FakeCudaDeleter::operator() runs instead
    // of delete -- this is precisely how you'll write
    // std::unique_ptr<float, CudaFreeDeleter> once you're on real CUDA.
}

void demo_shared_ptr() {
    std::cout << "\n-- shared_ptr: reference-counted shared ownership --\n";
    std::shared_ptr<Widget> s1 = std::make_shared<Widget>(3);
    std::cout << "  use_count after s1 created: " << s1.use_count() << "\n";
    {
        std::shared_ptr<Widget> s2 = s1; // copy is fine -- refcount increments
        std::cout << "  use_count after s2 = s1 (copy): " << s1.use_count() << "\n";
        s2->greet();
    } // s2 destructed -- refcount decrements, Widget survives (s1 still holds it)
    std::cout << "  use_count after s2 goes out of scope: " << s1.use_count() << "\n";
} // s1 destructed here -- refcount hits 0, NOW the Widget is actually destroyed

// Just needs to LOOK at a Widget, not own it -- a raw pointer is still
// exactly the right tool here. "Owning vs. borrowing" is the actual
// distinction, not "smart = good, raw = bad."
void inspect(const Widget* w) {
    if (w) std::cout << "  inspecting Widget " << w->id << " (not owned by this function)\n";
}

int main() {
    demo_unique_ptr();
    demo_custom_deleter();
    demo_shared_ptr();

    std::cout << "\n-- raw pointer as a non-owning observer --\n";
    auto owner = std::make_unique<Widget>(4);
    inspect(owner.get()); // .get() hands out a non-owning raw pointer -- inspect() never owns it

    return 0;
}