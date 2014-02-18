// Problems from http://projecteuler.net
// Problem 1: Find the sum of all the multiples of 3 or 5 below 1000.

#include <qolor/all.hpp>
#include <qolor/debug.h>
#include "testfn.h"

size_t classic_problem1()
{
	size_t sum = 0;

	for (size_t i = 3; i < 1000; i += 3)
		sum += i;

	for (size_t i = 5; i < 1000; i += 5)
		if (i % 3 != 0)
			sum += i;

	return sum;
}

int main()
{
	auto classic = classic_problem1();

	auto withqolor = qolor::range(1, 1000)
		.where([](const int&x)->bool { return (x%3) == 0 || (x%5) == 0; })
		.sum();

	ECHO_IF_FAILED2("Problem 1", (withqolor == classic));
	//std::cout << TESTFN2("Problem 1", (withqolor == classic)) << std::endl;
	//qolor::debug::test(withqolor, "With qolor");
	//qolor::debug::test_type(withqolor, "With qolor");
	
	// Alternative, to check select():
	// Find the double of the sum of the multiples of 3 and 5 below 1000.
	auto withqolor2 = qolor::range(1, 1000)
		.where([](const int&x) { return (x%3) == 0 || (x%5) == 0; })
		.select([](const int& x){ return 2*x; })
		.sum();
	
	ECHO_IF_FAILED2("Problem 1 (*2)", (withqolor2 == 2 * classic));
	//std::cout << TESTFN2("Problem 1 (*2)", (withqolor2 == 2 * classic)) << std::endl;
	//qolor::debug::test(withqolor2, "With qolor (*2)");
	//qolor::debug::test_type(withqolor2, "With qolor (*2)");

	return 0;
}
