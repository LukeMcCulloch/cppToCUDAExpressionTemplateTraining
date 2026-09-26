# Matrix's `backward()` — mirrored off `Vector`/`AddExpr`

> Status: **design/plan, not yet typed in or tested.** This mirrors the already-working
> `Vector` + `AddExpr` reverse-mode AD implementation onto the matrix side. Verify with
> a `pp+qq`-style test (below) once typed in, the same way the vector version was
> verified — don't assume correctness from the mirroring alone.

## Recap: how the vector version works

Three pieces, already implemented and verified:

1. **`VecExpr<Derived>`** (the CRTP base every vector expression node inherits from)
   forward-declares `Vector` and dispatches `backward()` to the derived type:
   ```cpp
   class Vector;
   template <typename Derived>
   struct VecExpr {
       ...
       void backward(const Vector& adjoint) const { self().backward(adjoint); }
   };
   ```
2. **`AddExpr::backward`** implements the chain rule for `+`. Since `r[i] = lhs[i] + rhs[i]`
   has an identity local Jacobian (`d(r[i])/d(lhs[i]) = 1`), the incoming adjoint passes
   through unchanged to both operands:
   ```cpp
   void backward(const Vector& adjoint) const {
       lhs_.backward(adjoint);
       rhs_.backward(adjoint);
   }
   ```
3. **`Vector::backward`** (the leaf) accumulates into a lazily-allocated gradient buffer,
   because a variable used more than once in a tree must sum contributions from every
   path that touches it:
   ```cpp
   void backward(const Vector& adjoint) const {
       if (!grad_) grad_ = std::make_unique<Vector>(size_);
       for (std::size_t i = 0; i < size_; ++i) (*grad_)[i] += adjoint[i];
   }
   ```

`adjoint` itself is nothing exotic — it's just a `const Vector&`, the same class used
for ordinary data, playing the role of "dL/d(this node's output)" for whatever
downstream quantity `L` you seeded. See
[reverse_mode_ad_adjoint_meaning.md](reverse_mode_ad_adjoint_meaning.md) if that note
exists, or the chat history, for the full derivation.

## The matrix-side mirror

Same three pieces, same reasoning — elementwise `+` has an identity Jacobian
regardless of whether the container is 1D or 2D, so nothing about the chain-rule
argument changes, only the indexing (`(i,j)` instead of `[i]`).

**1. `MatExpr.hpp`** — forward-declare `Matrix`, add the dispatch:
```cpp
class Matrix;

template <typename Derived>
struct MatExpr {
    const Derived& self() const { return static_cast<const Derived&>(*this); }
    double operator()(std::size_t i, std::size_t j) const { return self()(i, j); }
    std::size_t rows() const { return self().rows(); }
    std::size_t cols() const { return self().cols(); }

    void backward(const Matrix& adjoint) const { self().backward(adjoint); }
};
```

**2. `MatOperators.hpp`** — add the identity rule to `MatAddExpr`:
```cpp
void backward(const Matrix& adjoint) const {
    lhs_.backward(adjoint);
    rhs_.backward(adjoint);
}
```

**3. `Matrix.hpp`** — add the leaf accumulation, same shape as `Vector`'s:
```cpp
void backward(const Matrix& adjoint) const {
    if (!grad_) grad_ = std::make_unique<Matrix>(rows_, cols_);
    for (std::size_t i = 0; i < rows_; ++i)
        for (std::size_t j = 0; j < cols_; ++j)
            (*grad_)(i, j) += adjoint(i, j);
}
void zero_grad() const { grad_.reset(); }
const Matrix& grad() const { return *grad_; }
```
plus a new private member: `mutable std::unique_ptr<Matrix> grad_;` alongside
`rows_, cols_, data_`.

## Why this one is "free" and what isn't

`MatAddExpr`'s Jacobian is the identity, exactly like `AddExpr`'s — no actual matrix
math needed in `backward()`, just pass the adjoint through. That stops being true for
`MatMulExpr` / `MatVecMultExpr` / `VecMatMultExpr`: their local Jacobians are real
linear maps, not identities, so propagating an adjoint back through them requires an
actual `M^T * adjoint` (and an outer product for the other operand's gradient). That
needs a `transpose()` operation that doesn't exist yet — the next real hurdle after
`MatAddExpr`/`MatSubExpr` (the latter just needs a sign flip, same as `SubExpr`).

## Verification test (mirror of the `pp+qq` vector test)

```cpp
Matrix A(2, 2), B(2, 2);
// ... fill A, B ...
auto C = A + B;
Matrix seed(2, 2);
// ... fill seed with all 1s ...
C.backward(seed);
// A.grad() and B.grad() should both equal seed, elementwise
```
