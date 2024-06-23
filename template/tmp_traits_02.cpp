/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
#include <iostream>
#include <cmath>


template<typename T> struct traits;

template<typename F, typename G>
double derivative_of(double x) {
  return traits<F>::derivative_of( traits<G>::of(x) ) * traits<G>::derivative_of(x);
}

class Square {};
class Exp {};

// x^2
template<>
struct traits<Square> {
  static double of(double x) { return x * x; }
  static double derivative_of(double x) { return 2 * x; }
};

// e^x
template<>
struct traits<Exp> {
  static double of(double x) { return exp(x); }
  static double derivative_of(double x) { return of(x); }
};

int main()
{
    // Square ( Exp (x) )   (e^x)^2  的导数 2(e^x)
  std::cout << "derivative of exp(x)^2 at x=0: "
            << derivative_of<Square, Exp>(0) << '\n';

  // Exp ( Square (x) )  e^(x^2)  的导数 e^(x^2)*2x
  std::cout << "derivative of exp(x^2) at x=0: "
            << derivative_of<Exp, Square>(0) << '\n';


    return 0;
}
