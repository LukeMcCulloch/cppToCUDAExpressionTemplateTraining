#pragma once
#include <memory> // std::unique_ptr/std::make_unique
#include "MatExpr.hpp"

#include "ExprTraits.hpp"



class Matrix : public MatExpr<Matrix>
{

public:

    Matrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), data_(std::make_unique<double[]>(rows* cols)) {
    }

    Matrix(const Matrix& other)
        : rows_(other.rows_), cols_(other.cols_), data_(new double[other.rows_ * other.cols_]) {
        for (std::size_t i = 0; i < rows_ * cols_; ++i) data_[i] = other.data_[i];
    }

    Matrix& operator=(const Matrix& other) {
        if (this == & other) return *this;
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_.reset(new double[rows_ * cols_]);
        for (std::size_t i = 0; i < rows_ * cols_; ++i) data_[i] = other.data_[i];
        return *this;
    }



    // destructor, move ctor, move assignment: unique_ptr<double[]> handles them, same as Vector.


    double& operator()(std::size_t i, std::size_t j) { return data_[i * cols_ + j]; }
    const double& operator()(std::size_t i, std::size_t j) const { return data_[i * cols_ + j]; }
    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }

private:
    std::size_t rows_, cols_;
    std::unique_ptr<double[]> data_;

};


//
//class Matrix {
//public:
//    Matrix(std::size_t rows, std::size_t cols)
//        : rows_(rows), cols_(cols), data_(std::make_unique<double[]>(rows* cols)) {
//    }
//
//    Matrix(const Matrix& other)
//        : rows_(other.rows_), cols_(other.cols_), data_(new double[other.rows_ * other.cols_]) {
//        for (std::size_t i = 0; i < rows_ * cols_; ++i) data_[i] = other.data_[i];
//    }
//    Matrix& operator=(const Matrix& other) {
//        if (this == &other) return *this;
//        rows_ = other.rows_;
//        cols_ = other.cols_;
//        data_.reset(new double[rows_ * cols_]);
//        for (std::size_t i = 0; i < rows_ * cols_; ++i) data_[i] = other.data_[i];
//        return *this;
//    }
//    // destructor, move ctor, move assignment: unique_ptr<double[]> handles them, same as Vector.
//
//    double& operator()(std::size_t i, std::size_t j) { return data_[i * cols_ + j]; }
//    const double& operator()(std::size_t i, std::size_t j) const { return data_[i * cols_ + j]; }
//    std::size_t rows() const { return rows_; }
//    std::size_t cols() const { return cols_; }
//
//private:
//    std::size_t rows_, cols_;
//    std::unique_ptr<double[]> data_;
//};

// Matrix joins the same storage-trait system as Vector -- expensive to
// copy, so store by reference when used as an operand.
template <>
struct ExprTraits<Matrix> { using ExprRef = const Matrix&; };




