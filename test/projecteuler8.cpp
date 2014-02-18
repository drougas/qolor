// http://projecteuler.net/problem=8
// Find the greatest product of five consecutive digits in the 1000-digit number.

#include <iostream>
#include <vector>
#include <qolor/all.hpp>
#include <qolor/debug.h>

static const char digits[] =
	"73167176531330624919225119674426574742355349194934"
	"96983520312774506326239578318016984801869478851843"
	"85861560789112949495459501737958331952853208805511"
	"12540698747158523863050715693290963295227443043557"
	"66896648950445244523161731856403098711121722383113"
	"62229893423380308135336276614282806444486645238749"
	"30358907296290491560440772390713810515859307960866"
	"70172427121883998797908792274921901699720888093776"
	"65727333001053367881220235421809751254540594752243"
	"52584907711670556013604839586446706324415722155397"
	"53697817977846174064955149290862569321978468622482"
	"83972241375657056057490261407972968652414535100474"
	"82166370484403199890008895243450658541227588666881"
	"16427171479924442928230863465674813919123162824586"
	"17866458359124566529476545682848912883142607690042"
	"24219022671055626321111109370544217506941658960408"
	"07198403850962455444362981230987879927244284909188"
	"84580156166097919133875499200524063689912560717606"
	"05886116467109405077541002256983155200055935729725"
	"71636269561882670428252483600823257530420752963450";


int main()
{
	auto range1 = qolor::range(0, sizeof(digits) - 5);
	//qolor::debug::test(range1, "range1", false, false);

	auto products = range1
		.select([&](size_t i)->const char*{ return digits+i; })
		.where([&](const char* s){ return !qolor::from(s, s+5).contains('0'); })
		.select(
			[&](const char* s) {
				return qolor::from(s, s+5)
					.select([](char c) { return c-'0'; })
					.aggregate(std::multiplies<int>());
			}
		)
		.aggregate([](const int& a, const int& b){ return a<b? b : a; });
	//qolor::debug::test(products, "products", true, true);
	qolor::debug::test(products, "result");

	//qolor::debug::test_type(digits + 5, "digits + 5");
	//qolor::debug::test_type(std::vector<int>().begin(), "vector.begin()");
	//qolor::debug::test_type([](const int& x) { return x+5; }, "lambda");
	//qolor::debug::test_type(5, "int");

	//auto prod2 = qolor::from(digits+0, digits+5)
		//.select([](char c) { return c-'0'; });
	//qolor::debug::test(prod2, "separate aggregate", true, true);

	//auto prod3 = qolor::from(digits+0, digits+5)
		//.select([](char c) { return c-'0'; })
		//.aggregate(std::multiplies<int>());
	//qolor::debug::test(prod3, "integrated aggregate");
	
	//auto r2 = qolor::range(1, 4,2).where([](const int&){ return true; });
	//qolor::debug::test(r2, "separate aggregate", true, true);
	
	qolor::debug::test(qolor::range(0.0, 2.0,0.3), "float range 0.3");
	qolor::debug::test(qolor::range(0.0, 2.0,0.3,true), "float range 0.3 inclusive");

	qolor::debug::test(qolor::range(0.0, 2.0,0.5), "float range 0.5");
	qolor::debug::test(qolor::range(0.0, 2.0,0.5,true), "float range 0.5 inclusive");

	//auto r3 = qolor::range(1, 4)
		//.where([](const int&){ return true; })
		//.aggregate(std::multiplies<int>());
	//qolor::debug::test(r3, "integrated aggregate");

	return 0;
}
