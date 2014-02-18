#include <iostream>
#include <functional>
#include "qolor/functional_traits.hpp"
#include "testfn.h"

///////////////////////////////////////////////////////////////////////////////

struct A {};

struct s0
{
	int operator() () const { return 0; }
};

struct s1
{
	double operator() (const double &x) const { return x; }
};

struct s2
{
	double operator() (const double &x, const double& y) const { return x + y; }
	static double smemfun0() { return 1; }
	static double smemfun1(const int& x) { return x; }
	static double smemfun2(const int& x, const int& y) { return x + y; }
	double memfun(const int& x) { return x; }
};

int f0() { return 1; }
double f1(const double& x) { return x; }
double f2(const double& x, const double& y) { return x + y; }

///////////////////////////////////////////////////////////////////////////////

template<typename F>
void test(F const&, const char* const& message, bool is_functional, bool is_predicate, size_t arity)
{
	typedef qolor::utils::functional_traits<F> traits;
	std::cout << message
		<< ": " << testfn("is_functional", traits::is_functional == is_functional)
		<< ", " << testfn("is_predicate", traits::is_predicate == is_predicate)
		<< ", " << testfn("arity", traits::arity == arity) << " (== " << traits::arity << ')'
		<< std::endl;
}

///////////////////////////////////////////////////////////////////////////////

int main()
{
	test(A(), "struct A", false, false, 0);
	test(int(), "int", false, true, 0);
	
	test(f0, "int f0()", true, true, 0);
	test(f1, "double f1(const double&)", true, true, 1);
	test(f2, "double f2(const double&, const double&)", true, true, 2);

	test(s0(), "struct s0: ", true, true, 0);
	test(s1(), "struct s1: ", true, true, 1);
	test(s2(), "struct s2: ", true, true, 2);

	auto l0 = []() { return 0.5; };
	std::function<double()> sf0 = l0;
	test(l0, "lambda double()", true, true, 0);
	test(sf0, "std::function<double()>", true, true, 0);

	auto l1 = [](int x) { return double(x) / 2; };
	std::function<double(int)> sf1 = l1;
	test(l1, "lambda double(int)", true, true, 1);
	test(sf1, "std::function<double(int)>", true, true, 1);

	auto l2 = [](int x, short y) { return double(x + y) / 2; };
	std::function<double(int,short)> sf2 = l2;
	test(l2, "lambda double(int,short)", true, true, 2);
	test(sf2, "std::function<double(int,short)>", true, true, 2);

	test(s2::smemfun0, "static member function 0", true, true, 0);
	test(s2::smemfun1, "static member function 1", true, true, 1);
	test(s2::smemfun2, "static member function 2", true, true, 2);
	//test(s2().memfun, "member function", false, false, 0);

	return 0;
}
