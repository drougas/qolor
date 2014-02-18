#ifndef QOLOR_DEBUG_H__
#define QOLOR_DEBUG_H__

#include "utilities.h"
#include "functional_traits.hpp"
#include <iostream>

namespace qolor
{

namespace debug
{

template <typename T>
typename std::enable_if<qolor::utils::is_iterable<T>::value, void>::type
test(T exp, const char* header, const bool& run = true, std::ostream& s = std::cout)
{
	typedef typename T::value_type vtype;

	s << "================= " << header << " =======================\n\n";
	s << "Expression type: " << qolor::utils::type_name(typeid(T)) << std::endl;
	s << "Result value_type: " << qolor::utils::type_name(typeid(vtype)) << std::endl;

	if (run) {
		s << "Iteration:";
		for (auto v : exp)
			std::cout << ' ' << v;
		std::cout << std::endl;
	}

	s << std::endl;
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
test(T const& value, const char* header, std::ostream& s = std::cout)
{
	s << "================= " << header << " =======================\n\n";
	s << "value_type: " << qolor::utils::type_name(typeid(value)) << std::endl;
	s << "Value: " << value << std::endl << std::endl;
}

template<typename T>
void test_type(T, const char* header, std::ostream & s = std::cout)
{
	s << std::boolalpha;
	s << "========================= " << header << " =============================\n\n";
	s << "                              Type: " << qolor::utils::type_name(typeid(T)) << std::endl;
	s << "                              size: " << sizeof(T) << std::endl;
	s << "------------------------------------------\n";
	s << "                       is_iterator: " << qolor::utils::is_iterator<T>::value << std::endl;
	s << "                       is_iterable: " << qolor::utils::is_iterable<T>::value << std::endl;
	s << "                     is_functional: " << qolor::utils::is_functional<T>::value << std::endl;
	s << "------------------------------------------\n";
	s << "                          is_const: " << std::is_const<T>::value << std::endl;
	s << "                       is_volatile: " << std::is_volatile<T>::value << std::endl;
	s << "               is_lvalue_reference: " << std::is_lvalue_reference<T>::value << std::endl;
	s << "               is_rvalue_reference: " << std::is_rvalue_reference<T>::value << std::endl;
	s << "                     is_arithmetic: " << std::is_arithmetic<T>::value << std::endl;
	s << "                       is_integral: " << std::is_integral<T>::value << std::endl;
	s << "                 is_floating_point: " << std::is_integral<T>::value << std::endl;
	s << "                          is_class: " << std::is_class<T>::value << std::endl;
	s << "                          is_union: " << std::is_union<T>::value << std::endl;
	s << "                    is_fundamental: " << std::is_fundamental<T>::value << std::endl;
	s << "                       is_compound: " << std::is_compound<T>::value << std::endl;
	s << "                            is_pod: " << std::is_pod<T>::value << std::endl;
	s << "                        is_trivial: " << std::is_trivial<T>::value << std::endl;
	s << "                       is_function: " << std::is_function<T>::value << std::endl;
	s << "------------------------------------------\n";
	s << "                  is_constructible: " << std::is_constructible<T>::value << std::endl;
	s << "                   is_destructible: " << std::is_destructible<T>::value << std::endl;
	s << "          is_default_constructible: " << std::is_default_constructible<T>::value << std::endl;
	s << "             is_copy_constructible: " << std::is_copy_constructible<T>::value << std::endl;
	s << "             is_move_constructible: " << std::is_move_constructible<T>::value << std::endl;
	s << "------------------------------------------\n";
	//s << "        is_trivially_constructible: " << std::is_trivially_constructible<T>::value << std::endl;
	//s << "             is_trivially_copyable: " << std::is_trivially_copyable<T>::value << std::endl;
	//s << "         is_trivially_destructible: " << std::is_trivially_destructible<T>::value << std::endl;
	//s << "is_trivially_default_constructible: " << std::is_trivially_default_constructible<T>::value << std::endl;
	//s << "   is_trivially_copy_constructible: " << std::is_trivially_copy_constructible<T>::value << std::endl;
	//s << "   is_trivially_move_constructible: " << std::is_trivially_move_constructible<T>::value << std::endl;
	//s << "------------------------------------------\n";
	s << "                is_copy_assignable: " << std::is_copy_assignable<T>::value << std::endl;
	s << "                is_move_assignable: " << std::is_move_assignable<T>::value << std::endl;
	s << std::noboolalpha << std::endl;
}

template<typename T>
struct type_class
{
	static void test(const char* header, std::ostream & s = std::cout)
	{
		s << std::boolalpha;
		s << "========================= " << header << " =============================\n\n";
		s << "                              Type: " << qolor::utils::type_name(typeid(T)) << std::endl;
		s << "                              size: " << sizeof(T) << std::endl;
		s << "------------------------------------------\n";
		s << "                       is_iterator: " << qolor::utils::is_iterator<T>::value << std::endl;
		s << "                       is_iterable: " << qolor::utils::is_iterable<T>::value << std::endl;
		s << "                     is_functional: " << qolor::utils::is_functional<T>::value << std::endl;
		s << "------------------------------------------\n";
		s << "                          is_const: " << std::is_const<T>::value << std::endl;
		s << "                       is_volatile: " << std::is_volatile<T>::value << std::endl;
		s << "               is_lvalue_reference: " << std::is_lvalue_reference<T>::value << std::endl;
		s << "               is_rvalue_reference: " << std::is_rvalue_reference<T>::value << std::endl;
		s << "                     is_arithmetic: " << std::is_arithmetic<T>::value << std::endl;
		s << "                       is_integral: " << std::is_integral<T>::value << std::endl;
		s << "                 is_floating_point: " << std::is_integral<T>::value << std::endl;
		s << "                          is_class: " << std::is_class<T>::value << std::endl;
		s << "                          is_union: " << std::is_union<T>::value << std::endl;
		s << "                    is_fundamental: " << std::is_fundamental<T>::value << std::endl;
		s << "                       is_compound: " << std::is_compound<T>::value << std::endl;
		s << "                            is_pod: " << std::is_pod<T>::value << std::endl;
		s << "                        is_trivial: " << std::is_trivial<T>::value << std::endl;
		s << "                       is_function: " << std::is_function<T>::value << std::endl;
		s << "------------------------------------------\n";
		s << "                  is_constructible: " << std::is_constructible<T>::value << std::endl;
		s << "                   is_destructible: " << std::is_destructible<T>::value << std::endl;
		s << "          is_default_constructible: " << std::is_default_constructible<T>::value << std::endl;
		s << "             is_copy_constructible: " << std::is_copy_constructible<T>::value << std::endl;
		s << "             is_move_constructible: " << std::is_move_constructible<T>::value << std::endl;
		s << "------------------------------------------\n";
		s << "        is_trivially_constructible: " << std::is_trivially_constructible<T>::value << std::endl;
		s << "             is_trivially_copyable: " << std::is_trivially_copyable<T>::value << std::endl;
		s << "         is_trivially_destructible: " << std::is_trivially_destructible<T>::value << std::endl;
		s << "is_trivially_default_constructible: " << std::is_trivially_default_constructible<T>::value << std::endl;
		s << "   is_trivially_copy_constructible: " << std::is_trivially_copy_constructible<T>::value << std::endl;
		s << "   is_trivially_move_constructible: " << std::is_trivially_move_constructible<T>::value << std::endl;
		s << "------------------------------------------\n";
		s << "                is_copy_assignable: " << std::is_copy_assignable<T>::value << std::endl;
		s << "                is_move_assignable: " << std::is_move_assignable<T>::value << std::endl;
		s << std::noboolalpha << std::endl;
	}
};

} // namespace debug

} // namespace qolor

#endif //QOLOR_DEBUG_H__
