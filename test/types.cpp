#include <iostream>
#include <fstream>
#include <vector>
#include <qolor/debug.h>
#include <qolor/iterator_utilities.hpp>


template<typename IterableType>
auto test_iterable(IterableType&&) -> typename std::enable_if<
	qolor::utils::is_iterable<IterableType>::value
	,bool
>::type
{
	return true;
}

template<typename IterableType>
auto test_iterable(IterableType&&) -> typename std::enable_if<
	!qolor::utils::is_iterable<IterableType>::value
	,bool
>::type
{
	return false;
}

template<typename IterableType>
typename std::enable_if<
	qolor::utils::is_iterable<IterableType>::value
	,bool
>::type
test_iterable2(IterableType&& c)
{
	qolor::debug::type_class<IterableType>::test(c, "iterable true");
	return true;
}

template<typename IterableType>
typename std::enable_if<
	!qolor::utils::is_iterable<IterableType>::value
	,bool
>::type
test_iterable2(IterableType&& c)
{
	qolor::debug::type_class<IterableType>::test(c, "iterable false");
	return false;
}


int main()
{
	std::ofstream ofs("types_test.txt");

	qolor::debug::test_type([](){ return true; },"lambda", ofs);

	//std::vector<int> vec{1,2,3,4};
	//std::cout << "iterable test: " << test_iterable2(vec) << std::endl;

	qolor::debug::type_class<int>::test("int", ofs);
	qolor::debug::type_class<int const>::test("int const", ofs);
	qolor::debug::type_class<int&>::test("int &", ofs);
	qolor::debug::type_class<int const&>::test("int const &", ofs);
	qolor::debug::type_class<int&&>::test("int &&", ofs);

	return 0;
}
