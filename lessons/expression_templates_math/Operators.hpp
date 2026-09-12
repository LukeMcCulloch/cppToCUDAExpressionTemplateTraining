#pragma once

#include "VecExpr.hpp"
#include "ExprTraits.hpp"





//--------------
// ADD, vector - vector
//--------------
// AddExpr stores whatever ExprTraits<LHS>::ExprRef / ExprTraits<RHS>::ExprRef
// resolve to -- a reference for a Vector operand, a value for a nested
// expression-node operand. That's the whole fix, made structural instead
// of hoped-for.
template <typename LHS, typename RHS>
class AddExpr : public VecExpr<AddExpr<LHS, RHS>> {
public:
    AddExpr(const LHS& l, const RHS& r) : lhs_(l), rhs_(r) {}

    double operator[](std::size_t i) const { return lhs_[i] + rhs_[i]; }// iteration though all i is done by the evaluate function
    std::size_t size() const { return lhs_.size(); }

private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};



//
// note!: AddExpr is not a member of any of the classes AddExpr, Vector, or struct VecExpr 
//
template <typename LHS, typename RHS>
AddExpr<LHS, RHS> operator+(const VecExpr<LHS>& lhs, const VecExpr<RHS>& rhs) {
    return AddExpr<LHS, RHS>(lhs.self(), rhs.self());
    // about self(): the lhs and rhs are VecExpr<LHS> and VecExpr<RHS> respectively, 
    // so self() returns a const LHS& and const RHS& respectively.  
    // The AddExpr constructor takes const LHS& and const RHS& respectively, so this works out.
    // CRTP:
    // const Derived& self() const { return static_cast<const Derived&>(*this); } 
}



//--------------
// ADDITION Scalar + Vector
//--------------

template <typename RHS>
class ScalarVecAddExpr : public VecExpr<ScalarVecAddExpr<RHS>> {
public:
    ScalarVecAddExpr(double s, const RHS& r) : s_(s), rhs_(r) {}
    double operator[](std::size_t i) const { return s_ + rhs_[i]; }
    std::size_t size() const { return rhs_.size(); }
private:
    double s_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};

template <typename RHS>
ScalarVecAddExpr<RHS> operator+(double s, const VecExpr<RHS>& rhs) {
    return ScalarVecAddExpr<RHS>(s, rhs.self());
}
template <typename LHS>
ScalarVecAddExpr<LHS> operator+(const VecExpr<LHS>& lhs, double s) {
    return ScalarVecAddExpr<LHS>(s, lhs.self());
}




//--------------
// SUBTRACT Scalar - Vector
// not commutative -- v - s and s - v need separate classes
//--------------

// v - s

template <typename LHS>
class VecSubScalarExpr  : public VecExpr<VecSubScalarExpr<LHS>> {
public:
    VecSubScalarExpr (const LHS& l, double s) : lhs_(l), s_(s) {}
    double operator[](std::size_t i) const { return lhs_[i] - s_; }
    std::size_t size() const { return lhs_.size(); }
private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    double s_;
};

template <typename LHS>
VecSubScalarExpr <LHS> operator-(const VecExpr<LHS>& lhs_, double s) {
    return VecSubScalarExpr<LHS>(s, lhs_.self());
}


// s - v

template <typename RHS>
class ScalarSubVecExpr  : public VecExpr<ScalarSubVecExpr<RHS>> {
public:
    ScalarSubVecExpr (double s, const RHS& r) : s_(s), rhs_(r) {}
    double operator[](std::size_t i) const { return s_ - rhs_[i]; }
    std::size_t size() const { return rhs_.size(); }
private:
    double s_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};


template <typename RHS>
ScalarSubVecExpr <RHS> operator-(double s, const VecExpr<RHS>& rhs) {
    return ScalarSubVecExpr<RHS>(s, rhs.self());
}




//--------------
// SUBTRACT, vector - vector
//--------------

template <typename LHS, typename RHS>
class SubExpr : public VecExpr<SubExpr<LHS, RHS>> {
public:
    SubExpr(const LHS& l, const RHS& r) : lhs_(l), rhs_(r) {}

    double operator[](std::size_t i) const { return lhs_[i] - rhs_[i]; }// iteration though all i is done by the evaluate function
    std::size_t size() const { return lhs_.size(); }

private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};


template <typename LHS, typename RHS>
SubExpr<LHS, RHS> operator-(const VecExpr<LHS>& lhs, const VecExpr<RHS>& rhs) {
    return SubExpr<LHS, RHS>(lhs.self(), rhs.self());
    // about self(): the lhs and rhs are VecExpr<LHS> and VecExpr<RHS> respectively, 
    // so self() returns a const LHS& and const RHS& respectively.  
    // The AddExpr constructor takes const LHS& and const RHS& respectively, so this works out.
    // CRTP:
    // const Derived& self() const { return static_cast<const Derived&>(*this); } 
}





//--------------
// MULTIPLY Scalar * Vector
//--------------

template <typename RHS>
class ScalarVecMultExpr : public VecExpr<ScalarVecMultExpr<RHS>> {
public:
    ScalarVecMultExpr(double s, const RHS& r) : s_(s), rhs_(r) {}
    double operator[](std::size_t i) const { return s_ * rhs_[i]; }
    std::size_t size() const { return rhs_.size(); }
private:
    double s_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};

template <typename RHS>
ScalarVecMultExpr<RHS> operator*(double s, const VecExpr<RHS>& rhs) {
    return ScalarVecMultExpr<RHS>(s, rhs.self());
}
template <typename LHS>
ScalarVecMultExpr<LHS> operator*(const VecExpr<LHS>& lhs, double s) {
    return ScalarVecMultExpr<LHS>(s, lhs.self());
}



//--------------
// MULTIPLY Mat * Vec
//--------------

// generalized to expressions on both sides
template <typename LHS, typename RHS>
class MatVecMultExpr : public VecExpr<MatVecMultExpr<LHS, RHS>> {
    // this class is structuraly tied to producing a 1D VecExpr shaped result 
    //      (it inherits from VecExpr<MultExpr<RHS>>)
    // generalized by allowing both sides to be resultants from other computations - i.e. both sides to be expressions
public:
    MatVecMultExpr(const LHS& m, const RHS& v) : m_(m), v_(v) {}

    double operator[](std::size_t i) const {
        // dot product of one row of the matrix with the vector
        double sum = 0.0;
        for (std::size_t k = 0; k < m_.cols(); ++k)
        {
            sum += m_(i, k) * v_[k];
        }
        return sum;
    }

    std::size_t size() const { return m_.rows(); }// size of the output

private:
    typename ExprTraits<LHS>::ExprRef m_;
    typename ExprTraits<RHS>::ExprRef v_;

};


template <typename LHS, typename RHS>
MatVecMultExpr<LHS, RHS> operator*(const MatExpr<LHS>& m, const VecExpr<RHS>& v) {
    return MatVecMultExpr<LHS, RHS>(m.self(), v.self());
}






//--------------
// MULTIPLY Vec^T * Mat
//--------------


template <typename LHS, typename RHS >
class VecMatMultExpr : public VecExpr<VecMatMultExpr<LHS, RHS>> {
    // this class is structuraly tied to producing a 1D VecExpr shaped result 
    //      (it inherits from VecExpr<MultExpr<RHS>>)
    // now finished by allowing both sides to be resultants from other computations
public:
    VecMatMultExpr(const LHS& v, const RHS& m) : v_(v), m_(m) {}

    double operator[](std::size_t i) const {
        // dot product of vector^T with one column of the matrix
        double sum = 0.0;
        for (std::size_t k = 0; k < m_.rows(); ++k)
        {
            sum += v_[k] * m_(k, i);
        }
        return sum;
    }

    std::size_t size() const { return m_.cols(); } // size of the output

private:
    typename ExprTraits<LHS>::ExprRef v_;
    //typename ExprTraits<Matrix>::ExprRef m_;
    typename ExprTraits<RHS>::ExprRef m_;

};


// operator free function
template <typename LHS, typename RHS>
VecMatMultExpr<LHS, RHS> operator*(const VecExpr<LHS>& v, const MatExpr<RHS>& m) {
    return VecMatMultExpr<LHS, RHS>(v.self(), m.self());
}







