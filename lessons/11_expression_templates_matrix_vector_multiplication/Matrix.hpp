#pragma once




// class Matrix {
// public:
//     Matrix(std::size_t rows, std::size_t cols)
//         : rows_(rows), cols_(cols), data_(std::make_unique<double[]>(rows * cols)) {}


        
//     std::size_t rows() const { return rows_; }
//     std::size_t cols() const { return cols_; }

// private:
//     std::size_t rows_, cols_;
//     std::unique_ptr<double[]> data_;
// }


// // Matrix joins the same storage-trait system as Vector -- expensive to
// // copy, so store by reference when used as an operand.
// template <>
// struct ExprTraits<Matrix> { using ExprRef = const Matrix&; };