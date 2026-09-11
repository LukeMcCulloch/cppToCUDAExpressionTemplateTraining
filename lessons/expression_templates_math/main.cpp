#include <iostream>
#include <memory>

#include "VecExpr.hpp"
#include "Vector.hpp"
#include "ExprTraits.hpp"
#include "Operators.hpp"

#include "MatExpr.hpp"
#include "MatOperators.hpp"
#include "Evaluate.hpp"





int main() {
    Vector a(3), b(3), c(3);
    a[0] = 1; a[1] = 2; a[2] = 3;
    b[0] = 10; b[1] = 20; b[2] = 30;
    c[0] = 100; c[1] = 200; c[2] = 300;

    // a + b + c parses as (a + b) + c. The inner (a+b) is a TEMPORARY
    // AddExpr<Vector,Vector>. It gets copied (cheaply -- ExprTraits
    // says "by value" for expression nodes) into the outer
    // AddExpr<AddExpr<Vector,Vector>, Vector> BEFORE that temporary is
    // destroyed. That's the fix from lesson 1, finally applied where it
    // actually matters.
    Vector result = evaluate(a + b + c);

    std::cout << "a + b + c = [";
    for (std::size_t i = 0; i < result.size(); ++i) {
        std::cout << result[i];
        if (i + 1 < result.size()) std::cout << ", ";
    }
    std::cout << "]\n(expected [111, 222, 333])\n";



    Vector result2 = evaluate(c - b - a);

    std::cout << "c - b - a = [";
    for (std::size_t i = 0; i < result2.size(); ++i) {
        std::cout << result2[i];
        if (i + 1 < result2.size()) std::cout << ", ";
    }
    std::cout << "]\n(expected [89, 178, 267])\n";



    //-----------
    // Mat * Vec
    //-----------

    Matrix M(3, 3);
    M(0, 0) = 1; M(0, 1) = 2; M(0, 2) = 3;
    M(1, 0) = 0; M(1, 1) = 1; M(1, 2) = 4;
    M(2, 0) = 5; M(2, 1) = 6; M(2, 2) = 0;

    Vector x(3);
    x[0] = 1; x[1] = 2; x[2] = 3;


    std::cout << "\n";
    std::cout << "M = " << "\n";
    std::cout << "| 1, 2, 3 |" << "\n";
    std::cout << "| 0, 1, 4 |" << "\n";
    std::cout << "| 5, 6, 0 |" << "\n";

    std::cout << "\n";
    std::cout << "x = " << "\n";
    std::cout << "| 1, 2, 3 |^T" << "\n";


    std::cout << "\n";
    Vector y = evaluate(M * x);
    std::cout << "M*x = [" << y[0] << ", " << y[1] << ", " << y[2] << "]\n";
    std::cout << "\n";
    std::cout << "(expected [14, 14, 17])\n";



    //-----------
    // Mat * Mat
    //-----------

    // MATMUL
    Matrix A(2, 2);
    A(0, 0) = 1; A(0, 1) = 2;
    A(1, 0) = 3; A(1, 1) = 4;

    Matrix B(2, 2);
    B(0, 0) = 5; B(0, 1) = 6;
    B(1, 0) = 7; B(1, 1) = 8;



    std::cout << "\n";
    std::cout << "A = " << "\n";
    std::cout << "| 1, 2 |" << "\n";
    std::cout << "| 3, 4 |" << "\n";




    std::cout << "\n";
    std::cout << "B = " << "\n";
    std::cout << "| 5, 6 |" << "\n";
    std::cout << "| 7, 8 |" << "\n";


    std::cout << "\n";
    Matrix C = evaluate(A * B);
    std::cout << "A*B = [[" << C(0, 0) << ", " << C(0, 1) << "], ["
        << C(1, 0) << ", " << C(1, 1) << "]]\n";
    std::cout << "(expected [[19, 22], [43, 50]])\n";



    //-----------
    // Vec^T * Mat
    //-----------

    Vector v(3);
    v[0] = 1; v[1] = 2; v[2] = 3;

    Matrix N(3, 2); // deliberately non-square: 3 rows, 2 columns
    N(0, 0) = 1; N(0, 1) = 2;
    N(1, 0) = 3; N(1, 1) = 4;
    N(2, 0) = 5; N(2, 1) = 6;

    std::cout << "\n";
    std::cout << "v = " << "\n";
    std::cout << "| 1, 2, 3 |" << "\n";

    std::cout << "\n";
    std::cout << "N = " << "\n";
    std::cout << "| 1, 2 |" << "\n";
    std::cout << "| 3, 4 |" << "\n";
    std::cout << "| 5, 6 |" << "\n";

    std::cout << "\n";
    Vector z = evaluate(v * N);
    std::cout << "v^T*N = [" << z[0] << ", " << z[1] << "]\n";
    std::cout << "(expected [22, 28])\n";

    return 0;
}