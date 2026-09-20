#include <iostream>
#include <memory>

#include "VecExpr.hpp"
#include "Vector.hpp"
#include "ExprTraits.hpp"
#include "Operators.hpp"
#include "dot.hpp"

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



    //-----------
    // all M * M * V expression templates
    //-----------

    std::cout << "\n";
    std::cout << "A = " << "\n";
    std::cout << "| 1, 2 |" << "\n";
    std::cout << "| 3, 4 |" << "\n";


    std::cout << "\n";
    std::cout << "B = " << "\n";
    std::cout << "| 5, 6 |" << "\n";
    std::cout << "| 7, 8 |" << "\n";


    std::cout << "\n";
    std::cout << "w = " << "\n";
    std::cout << "[ 1, 1 ]" << "\n";


    Vector w(2);
    w[0] = 1; w[1] = 1;

    Vector u = evaluate(A * B * w);
    std::cout << "(A*B)*w = [" << u[0] << ", " << u[1] << "]\n";
    std::cout << "(expected [41, 93])\n";


    //-----------
    // Vec^T * (Mat * Mat)  -- reuses A, B from the matmul section above.
    // Nothing gets materialized between the A*B and the p^T*(...) --
    // this is the new thing the RHS generalization enables.
    //-----------

    Vector p(2);
    p[0] = 1; p[1] = 1;

    std::cout << "\n";
    std::cout << "p = " << "\n";
    std::cout << "| 1, 1 |" << "\n";

    std::cout << "\n";
    Vector q = evaluate(p * (A * B));
    std::cout << "p^T*(A*B) = [" << q[0] << ", " << q[1] << "]\n";
    std::cout << "(expected [62, 72])\n";


    //-----------
    // Matrix + scalar
    //-----------
    Vector rv2 = evaluate(a + 5.0);
    std::cout << "\na+5 = [" << rv2[0] << ", " << rv2[1] << ", " << rv2[2] << "]\n";
    std::cout << "(expected [6, 7, 8])\n";

    
    //-----------
    // scalar - Matrix
    //-----------
    Vector rv3 = evaluate(5.0 - a);
    std::cout << "5-a = [" << rv3[0] << ", " << rv3[1] << ", " << rv3[2] << "]\n";
    std::cout << "(expected [4, 3, 2])\n";


    //---------
    // dot (vec, vec)
    //
    // and operator* for Vector*Vector
    //-----------

    /*
    a[0] = 1; a[1] = 2; a[2] = 3;
    b[0] = 10; b[1] = 20; b[2] = 30;
    c[0] = 100; c[1] = 200; c[2] = 300;
    */

    std::cout << "\n";
    std::cout << "a = [ 1, 2, 3 ]" << "\n";
    std::cout << "b =  [ 10, 20, 30 ]" << "\n";
    std::cout << "c =  [ 100, 200, 300 ]" << "\n";

    double d1 = dot(a, b);
    std::cout << "\ndot(a, b) = " << d1 << "\n";
    std::cout << "(expected 140)\n";

    double d2 = a * b;
    std::cout << "a * b     = " << d2 << "\n";
    std::cout << "(expected 140, same as dot(a,b) -- operator* just delegates to dot())\n";

    // The FEM-style case: a contraction appearing mid-expression,
    // producing a scalar that then broadcasts against another vector.
    Vector contraction_result = evaluate(a * b + c);
    std::cout << "\na*b + c = [" << contraction_result[0] << ", "
                                  << contraction_result[1] << ", "
                                  << contraction_result[2] << "]\n";
    std::cout << "(expected [240, 340, 440])\n";



    //---------
    // automatic differentiation (reverse mode)
    // vector + vector only
    //-----------

    Vector pp(3);
    pp[0] = 1; pp[1] = 2; pp[2] = 3;
    Vector qq(3);
    qq[0] = 10; qq[1] = 20; qq[2] = 30;

    auto expr = pp + qq;
    Vector fwd = evaluate(expr);
    std::cout << "\nforward: pp+qq = [" << fwd[0] << ", " << fwd[1] << ", " << fwd[2] << "]\n";

    Vector seed(3);
    seed[0] = 1; seed[1] = 1; seed[2] = 1; // pretend d(loss)/d(result) = [1,1,1]
    expr.backward(seed);

    std::cout << "pp.grad() = [" << pp.grad()[0] << ", " << pp.grad()[1] << ", " << pp.grad()[2] << "]\n";
    std::cout << "(expected [1, 1, 1])\n";
    std::cout << "qq.grad() = [" << qq.grad()[0] << ", " << qq.grad()[1] << ", " << qq.grad()[2] << "]\n";
    std::cout << "(expected [1, 1, 1])\n";
    return 0;
}