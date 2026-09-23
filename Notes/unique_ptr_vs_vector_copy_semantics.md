# Why `Vector` needs hand-written copy semantics

Question: why does our `Vector` class (backed by `std::unique_ptr<double[]>`) need a
hand-written copy constructor / copy assignment operator, why is `unique_ptr` built
that way, and what would change if the underlying storage supplied copy semantics
on its own?

## Why `unique_ptr` has no copy constructor — by design, not oversight

`unique_ptr`'s contract is: *exactly one owner, always.* Its copy constructor and
copy assignment operator are explicitly `= delete`d in the standard library:

```cpp
unique_ptr(const unique_ptr&) = delete;
unique_ptr& operator=(const unique_ptr&) = delete;
```

If copying were allowed to just memberwise-copy the raw pointer, two `unique_ptr`s
would end up holding the *same* address. Both destructors would eventually run and
both would `delete` that same pointer — the classic double-free bug from the
`IntBuffer` lesson, except baked into the standard library. So the committee made
copying a **compile error** instead of a runtime landmine.

Move is different: transferring ownership (steal the pointer, null out the source)
never creates two owners, so `unique_ptr` *does* give you move for free. Copy it
refuses to guess at, because there's no universally safe default for "what does
copying this owned resource mean?"

This is exactly why `data_ = new double[n];` fails in `Vector`'s copy assignment —
`unique_ptr::operator=` has no raw-pointer overload — and why the correct call is
`data_.reset(new double[n]);`, an explicit, deliberate ownership transfer.

## Why that pushes the job onto `Vector`

Once `unique_ptr<double[]>` refuses to copy, the *compiler-generated* `Vector` copy
constructor (the one you'd get for free by not writing one) would try to
memberwise-copy `data_` — calling `unique_ptr`'s deleted copy constructor — which is
a compile error. The compiler is refusing to guess what copying a `Vector` should
mean; you decide. The correct decision for a numeric container is a **deep copy**:
allocate a fresh buffer, copy every element, so two `Vector`s are fully independent
afterward. That's what the hand-written copy ctor/assignment in `Vector.hpp` does.

## What if the member type supplied copy semantics itself?

If `data_` were `std::vector<double>` instead of `std::unique_ptr<double[]>`,
`Vector` wouldn't need to write *any* special member functions — Rule of 0 all the
way:

```cpp
std::vector<double> data_;
```

`std::vector`'s own copy constructor already does exactly the deep-copy logic we
wrote by hand (allocate new buffer, copy each element) — the standard library
authors already did the Rule-of-5 exercise once so every user of `vector` doesn't
have to. `size()` becomes `data_.size()`, `operator[]` becomes `data_[i]`, the
constructor becomes `data_(n, 0.0)`. No destructor, no copy ctor, no copy
assignment, no move ctor, no move assignment — all five correctly synthesized,
because every member already knows how to copy/move/destroy itself.

This is *why* the exercise used `unique_ptr<double[]>` instead of `std::vector`
from the start: using `vector` would have skipped the entire lesson. `unique_ptr`
gives move-for-free but refuses copy on purpose, forcing the deep-copy logic to be
written by hand once — so it's understood, not just trusted.

## The trap: `shared_ptr<double[]>`

Reaching for `shared_ptr` instead would compile — its copy constructor *is*
defined — but gives the *wrong* semantics for a numeric `Vector`. Copying a
`shared_ptr` doesn't clone the buffer; it increments a reference count and hands
back a second handle to the *same* buffer. Two `Vector`s sharing one
`shared_ptr<double[]>` would alias each other: mutating one through `operator[]`
would silently mutate the other, since there's only one underlying array. That's
reference semantics, correct when shared ownership is actually wanted (e.g. a graph
node several edges point to), but a real bug for something meant to behave like a
value — `Vector a = b;` should produce two independent vectors, not two views onto
one.

## Summary

| Storage type              | Copy behavior              | Special members needed |
|----------------------------|-----------------------------|-------------------------|
| `unique_ptr<double[]>`      | none (deleted) — must write by hand | hand-written copy ctor/assignment (what we have) |
| `std::vector<double>`       | deep copy, built in         | none — Rule of 0 |
| `shared_ptr<double[]>`      | shallow, reference-counted (aliasing) | none needed to compile, but wrong semantics for a value type |
