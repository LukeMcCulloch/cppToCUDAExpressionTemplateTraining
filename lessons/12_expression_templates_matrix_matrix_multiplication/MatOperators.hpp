#pragma once


#include "Matrix.hpp"
#include "MatExpr.hpp"
#include "ExprTraits.hpp"





// matrix - matrix multiplication
template <typename LHS, typename RHS>
class MatMulExpr : public MatExpr<MatVecMultExpr<LHS, RHS>> {

public:
    MatMulExpr
     
};



// operator free function
template <typename LHS, typename RHS>
MatMulExpr<LHS, RHS> operator*(const Matrix<LHS>& m, const Matrix<RHS>& n)
{
    return MatMulExpr<LHS, RHS>(m.self(), n.self());
}