// Problems from http://projecteuler.net
// Problem 2:
// By considering the terms in the Fibonacci sequence whose values
// do not exceed four million, find the sum of the even-valued terms.

#include <iostream>
#include <map>
#include "testfn.h"

std::ostream & operator << (std::ostream& s, std::pair<int64_t,uint64_t> const& p)
{
	return s << '<' << p.first << ',' << p.second << '>';
}

#include <qolor/all.hpp>
#include <qolor/debug.h>

class Problem2Func
{
private:
	std::pair<uint64_t,uint64_t> ret = std::make_pair(int64_t(-1), uint64_t(0));
	uint64_t a = 0, b = 1, c = 1;

public:
	Problem2Func() = default;
	Problem2Func(Problem2Func const&) = default;
	Problem2Func(Problem2Func&&) = default;

	std::pair<uint64_t,uint64_t> const& operator()() {
		++ret.first;
		ret.second = a;
		a = b;
		b = c;
		c = a + b;
		return ret;
	}
};

Problem2Func problem2func;
Problem2Func get_value() { return Problem2Func(); }
Problem2Func& get_ref() { return problem2func; }
Problem2Func const& get_const_ref() { return problem2func; }
Problem2Func&& get_rvalue_ref() { return std::move(problem2func); }

class Problem2BFunc
{
private:
	uint64_t a = 0, b = 1, c = 1;

public:
	Problem2BFunc() = default;
	Problem2BFunc(Problem2BFunc const&) = default;

	uint64_t const& operator()() {
		a = b;
		b = c;
		c = a + b;
		return a;
	}
};

class Problem2OptimizedFunc
{
private:
	uint64_t a = 0, b = 1, c = 1;

public:
	Problem2OptimizedFunc() = default;
	Problem2OptimizedFunc(Problem2OptimizedFunc const&) = default;

	uint64_t const& operator()() {
		a = b + c;
		b = c + a;
		c = b + a;
		return a;
	}
};

uint64_t problem2slow()
{
	// http://blog.narsis.gr/archives/82
	uint64_t sum = 0, fib1 = 1, fib2 = 1, fib3 = 2;

	while (fib3 < 4000000) {
		fib3 = fib1 + fib2;
		if (!(fib3 & 1))
			sum += fib3;
		fib1 = fib2;
		fib2 = fib3;
	}

	return sum;
}

uint64_t problem2optimized()
{
	uint64_t sum = 0, fib1 = 1, fib2 = 1, fib3 = 2;

	while (fib3 < 4000000) {
		sum += fib3;
		fib1 = fib2 + fib3;
		fib2 = fib3 + fib1;
		fib3 = fib1 + fib2;
	}

	return sum;
}

int main()
{
	auto classic_slow = problem2slow();
	auto classic_opt = problem2optimized();

	auto p2extended = qolor::from(
			Problem2Func(),
			[](std::pair<uint64_t,uint64_t> const& x){ return x.second < 4000000; })
		.where([](std::pair<uint64_t,uint64_t> const& x){ return !(x.second & 1); })
		.select([](std::pair<uint64_t,uint64_t> const& x){ return x.second; })
		.sum();

	//qolor::debug::test(p2types, "Problem 2 Extended");
	//qolor::debug::test_type(Problem2Func(),"Function Object");

	auto p2 = qolor::from(
			Problem2BFunc(),
			[](uint64_t const& x){ return x < 4000000; })
		.where([](uint64_t const& x){ return !(x & 1); })
		.sum();
	//qolor::debug::test(p2, "Problem 2");

	auto p2optimized = qolor::from(Problem2OptimizedFunc(),
			[](uint64_t const& x){ return x < 4000000; }).sum();

	std::cout << TESTFN2("Compare classic slow with optimized", (classic_slow == classic_opt)) << std::endl;
	std::cout << TESTFN2("Compare classic with first", (classic_slow == p2extended)) << std::endl;
	std::cout << TESTFN2("Compare classic with second", (classic_slow == p2)) << std::endl;
	std::cout << TESTFN2("Compare classic with optimized", (classic_slow == p2optimized)) << std::endl;

	return 0;
}
