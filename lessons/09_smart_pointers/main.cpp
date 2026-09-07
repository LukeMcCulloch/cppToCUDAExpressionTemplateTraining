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



/*

"The framing worth having going in: std::unique_ptr is Rule of 5, already correctly written, for a single heap object — it's what IntBuffer would be if all it needed to do was own one thing. You're not learning a new concept here so much 
as seeing the thing you built by hand handed back to you, pre-built, for free."

-- what parts of our rule of 5 tutorial (IntBuffer class) does unique_ptr implement?

move semantics (move constructor and move operator=), but not copy semantics?
Yes, exactly right — confirmed precisely, member by member:

Special member	IntBuffer (lesson 3)	unique_ptr
Destructor	delete[] data_;	Implements it 
— calls delete (or your custom deleter) on the owned pointer. Same job.
Copy constructor Deep-copies into a new buffer	Deleted. 
Doesn't exist at all — attempting to copy is a compile error.
Copy assignment	Deep-copies, with self-assignment guard	Deleted. Same as above.
Move constructor 
Steals the pointer, nulls the source
Implements it — identical steal-and-null logic.
Move assignment	Releases its own resource, steals, nulls the source, self-move guard 
Implements it — same shape.
So your summary is exactly it: move semantics, fully implemented; copy semantics, 
not implemented at all (deleted, not just "not written"). 
That's the precise sense in which unique_ptr is "3 of the 5, done for you" 
rather than "all 5, done for you" 
— the two it omits are omitted on purpose, because allowing them would contradict what "unique" means.

*/


/*

and by the same token, does shared_ptr implement copy semantics?  (i.e. copy constructor and copy assignment)

Yes — and this is the exact opposite pattern from unique_ptr: shared_ptr implements all five, none deleted.

Special member	shared_ptr
Destructor	Decrements the refcount; deletes the managed object only if that count hits zero.
Copy constructor	Implemented, not deleted. Copies the pointer and increments the refcount 
— both shared_ptrs now point at the same object.
Copy assignment	Implemented. Releases whatever the target currently owned (decrementing its old refcount, 
possibly destroying it), then points at the new object and increments its count.
Move constructor	Implemented — steals the pointer, leaves the source null. No refcount change at all, 
since ownership transferred rather than duplicated.
Move assignment	Implemented — releases what it currently owns, then steals from the source, leaves source null.

The precision worth locking in: "copy" means a genuinely different thing across all three types 
you now have side by side. 
IntBuffer's copy deep-copies values into new, independent storage. 
unique_ptr has no copy at all — copying is a compile error. 
shared_ptr's copy does neither of those — it's a cheap pointer copy plus an atomic refcount increment, 
and both copies end up pointing at the literal same underlying object. 
That last point is also why move is still worth having on shared_ptr 
even though copy is allowed: copying costs an atomic increment 
(real, measurable overhead, especially on a hot path); 
moving a shared_ptr skips that entirely, 
since nothing about "how many owners exist" changes when you transfer rather than duplicate ownership.

So, I suppose we could re-implement our IntBuffer class with unique_ptr and then build IntMatrix on top of that, 
and we would have a pretty nice bulletproof foundation for a basic integer matrix array class?

Or would a shared_ptr be more appropriate, on average, for numerical code?

Good instinct on the first part, with one important nuance to add 
— and a clear "no" on the second part, for a reason that matters a lot.

Rebuilding IntBuffer on unique_ptr<int[]>: yes, and it's actually a genuinely good foundation 
— but not full Rule of 0. unique_ptr gives you destructor and move for free, correctly, forever. 
But it deletes copy 
— so if IntBuffer just holds a bare unique_ptr<int[]> and declares nothing else, 
IntBuffer itself becomes move-only too 
(the compiler can't generate a copy constructor when the member's copy constructor doesn't exist). 
If you want IntBuffer/IntMatrix to stay copyable — and for a general-purpose numerical type, 
you almost certainly do, since B = A; should give you an independent matrix, not an error 
— you'd still need to hand-write the copy constructor and copy assignment yourself, 
exactly like you already did. So the honest framing: unique_ptr takes destructor 
and move completely off your plate, permanently, 
and leaves copy as the one thing still yours to get right 
— a real, meaningful reduction in what you have to maintain by hand, 
just not the full "zero special members" you got with std::vector in lesson 4.

Here's the genuinely nice payoff this unlocks, 
tying back to something from a while ago: 
    construct the unique_ptr<int[]> with raw new int[n] 
    instead of std::make_unique<int[]>(n), 
    and you skip the forced zero-initialization std::vector always imposes. 
    make_unique<T[]> value-initializes every element, same tax as vector; 
    plain new int[n] (no parentheses) doesn't. 
    That's a real, concrete performance edge for numerical buffers you're about to overwrite anyway 
    — unique_ptr done this way is arguably a better foundation for IntMatrix than std::vector, 
    not merely an equivalent one, 
    precisely for the reason we flagged earlier about std::vector not being ideal for hot numerical code.

shared_ptr as the backing storage, though — no, and it's worth being precise about why, 
because the reason is a correctness issue, not just a performance one. 
Numerical code wants value semantics: B = A; should produce an independent matrix, 
and mutating B afterward should never touch A. 
shared_ptr's copy does the opposite on purpose 
— both copies point at the same underlying buffer, 
so B(0,0) = 5; would silently also change A. 
That's not a performance tradeoff, 
that's reintroducing the exact aliasing bug from lesson 1 
— just refcounted instead of double-freeing, 
so it wouldn't crash, 
but it would silently produce mathematically wrong results 
the moment two matrix variables were expected to be independent. 
On top of that, every copy costs a real, atomic refcount increment 
— pure overhead you'd be paying for a sharing behavior you don't even want.

shared_ptr earns its keep in a numerical/CUDA system 
for a genuinely different job: something like a large, 
read-only mesh or lookup table that many lightweight objects need to reference 
without owning or duplicating it 
— sharing one thing on purpose, not backing a value type 
that's supposed to behave like a plain number or array. 
That's a different design question than "what should IntMatrix store internally," 
and it's one you're likely to actually hit once you're building the mesh/BEM side of this work 
— just not this one.


*/