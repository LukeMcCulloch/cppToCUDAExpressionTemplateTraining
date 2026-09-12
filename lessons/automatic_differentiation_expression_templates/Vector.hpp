#pragma once
#include <memory> // std::unique_ptr/std::make_unique
#include "VecExpr.hpp"




// Vector: the leaf, data-owning type. Built on the exact judgment call
// from the smart-pointers lesson -- unique_ptr<double[]> gives
// destructor AND move for free (neither is written below at all; the
// compiler-generated versions are already correct, because unique_ptr's
// own move/destructor already are). Copy is the one thing unique_ptr
// can't guess for us, so it's the only special member written by hand.
class Vector : public VecExpr<Vector>
{
public:
    explicit Vector(std::size_t n) : size_(n), data_(std::make_unique<double[]>(n)) {} // memsets to zero under the hood (or loops, but anyway), so no need to loop and zero-initialize ourselves


    
    // copy constructor: allocate our own buffer, then copy elements over.
    Vector(const Vector& other) : size_(other.size_), data_(new double[other.size_]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }


    // copy assignment: release what we currently own, then allocate our own buffer and copy elements over.
    Vector& operator=(const Vector& other) {
        if (this == &other) return *this;
        size_ = other.size_;
        data_.reset(new double[size_]);// raw new[], not make_unique, because we want to copy the elements over ourselves, not default-construct them
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        return *this;
    }

    // destructor, move assignment, move constructor: 
    //      all implemented by unique_ptr<double[]> for us, so we don't write them here.

    double& operator[](std::size_t i) { return data_[i]; } 
    const double& operator[](std::size_t i) const { return data_[i]; }
    std::size_t size() const { return size_; }


    // automatic differentiation
    void backward(const Vector& adjoint) const {
        if (!grad_) grad_ = std::make_unique<Vector>(size_);
        for (std::size_t i = 0; i < size_; ++i) (*grad_)[i] += adjoint[i];
    }
    void zero_grad() const { grad_.reset(); }
    const Vector& grad() const { return *grad_; }

    // added to Vector's private section:
    mutable std::unique_ptr<Vector> grad_;

private:
    std::size_t size_;
    std::unique_ptr<double[]> data_;// unique_ptr<double[]> implements destructor and move for us, 
    // so we don't write them here. 
    // Copy is the one thing unique_ptr doesn't implement, so we do it by hand.
};

