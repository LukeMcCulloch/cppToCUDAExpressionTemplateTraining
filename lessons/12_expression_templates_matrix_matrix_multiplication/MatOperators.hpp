#pragma once


#include "Matrix.hpp"
#include "MatExpr.hpp"
#include "ExprTraits.hpp"





// matrix - matrix multiplication
template <typename LHS, typename RHS>
class MatMulExpr : public MatExpr<MatMulExpr<LHS, RHS>> {

public:
    MatMulExpr(const LHS& l, const RHS& r) : lhs_(l), rhs_(r) {}
     
    // the dot product of row i of lhs_ with column j of rhs_, 
    // computing exactly one entry of the result, result(i,j).
    double operator()(std::size_t i, std::size_t j) const {
        double sum = 0.0;
        for (std::size_t k = 0; k < lhs_.cols(); ++k) {
            sum += lhs_(i, k) * rhs_(k, j);
        }
        return sum;
    }


    // A (m x n) * B (n x p) = result (m x p) -- result takes its ROW
    // count from lhs and its COLUMN count from rhs. The shared inner
    // dimension (lhs_.cols(), which must equal rhs_.rows()) is what
    // operator() sums over above -- it doesn't appear in the result's shape.
    std::size_t rows() const { return lhs_.rows(); }
    std::size_t cols() const { return rhs_.cols(); }



private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};



// operator free function
template <typename LHS, typename RHS>
MatMulExpr<LHS, RHS> operator*(const MatExpr<LHS>& lhs, const MatExpr<RHS>& rhs)
{
    return MatMulExpr<LHS, RHS>(lhs.self(), rhs.self());
}