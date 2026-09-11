#pragma once


#include "Matrix.hpp"
#include "MatExpr.hpp"
#include "ExprTraits.hpp"





//--------------
// ADD Matrix with Matrix
//--------------
template <typename LHS, typename RHS>
class MatAddExpr : public MatExpr<MatAddExpr<LHS, RHS>> {
public:
    MatAddExpr(const LHS& l, const RHS& r) : lhs_(l), rhs_(r) {}

    double operator()(std::size_t i, std::size_t j) const { return lhs_(i,j) + rhs_(i, j); }// iteration though all i is done by the evaluate function
    
    std::size_t rows() const { return lhs_.rows(); }
    std::size_t cols() const { return lhs_.cols(); }


private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};


template <typename LHS, typename RHS>
MatAddExpr<LHS, RHS> operator+(const MatExpr<LHS>& lhs, const MatExpr<RHS>& rhs) {
    return MatAddExpr<LHS, RHS>(lhs.self(), rhs.self());
}



//--------------
// SUBTRACT with Matrix with Matrix
//--------------
template <typename LHS, typename RHS>
class MatSubExpr : public MatExpr<MatSubExpr<LHS, RHS>> {
public:
    MatSubExpr(const LHS& l, const RHS& r) : lhs_(l), rhs_(r) {}

    double operator()(std::size_t i, std::size_t j) const { return lhs_(i, j) - rhs_(i, j); }// iteration though all i is done by the evaluate function
    
    std::size_t rows() const { return lhs_.rows(); }
    std::size_t cols() const { return lhs_.cols(); }


private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};


template <typename LHS, typename RHS>
MatSubExpr<LHS, RHS> operator-(const MatExpr<LHS>& lhs, const MatExpr<RHS>& rhs) {
    return MatSubExpr<LHS, RHS>(lhs.self(), rhs.self());
}



//--------------
// SUBTRACT 
//--------------




//--------------
// MULTIPLY Scalar * Matrix
//--------------

template <typename RHS>
class MatMulScalarExpr : public MatExpr<MatMulScalarExpr<RHS>> {
public:
    MatMulScalarExpr(double s, const RHS& r) : s_(s), rhs_(r) {}
    double operator()(std::size_t i) const { return s_ * rhs_(i,j); }
    std::size_t size() const { return rhs_.size(); }

    
    std::size_t rows() const { return lhs_.rows(); }
    std::size_t cols() const { return rhs_.cols(); }


private:
    double s_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};


template <typename RHS>
MatMulScalarExpr<RHS> operator*(double s, const MatExpr<RHS>& rhs) {
    return MatMulScalarExpr<RHS>(s, rhs.self());
}
template <typename LHS>
MatMulScalarExpr<LHS> operator*(const MatExpr<LHS>& lhs, double s) {
    return MatMulScalarExpr<LHS>(s, lhs.self());
}



//--------------
// matrix - matrix multiplication
//--------------
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