#ifndef QOLOR_UTILITIES_H__
#define QOLOR_UTILITIES_H__

#include <cstdint>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <tuple>
#include "iterator_utilities.hpp"

namespace qolor
{

namespace utils
{
	std::string type_name(const char* name, bool withOriginal = false);
	std::string type_name(const std::type_info& type, const bool& withOriginal = false);

	// Tests if T is a specialization of Template. Taken from:
	// https://bitbucket.org/martinhofernandes/wheels/src/default/include/wheels/meta/type_traits.h%2B%2B#cl-161
	// http://stackoverflow.com/questions/13101061/detect-if-a-type-is-a-stdtuple

	template <typename T, template <typename...> class Template>
	struct is_specialization_of : public std::false_type
	{
		typedef Template<> template_type;
		typedef void args_type;
	};

	template <template <typename...> class Template, typename... Args>
	struct is_specialization_of<Template<Args...>, Template> : public std::true_type
	{
		typedef Template<Args...> template_type;
		typedef std::tuple<Args...> args_type;
	};


	// http://channel9.msdn.com/Forums/TechOff/Templated-STL-container-pretty-printer
	// http://stackoverflow.com/questions/4850473/pretty-print-c-stl-containers
	template<typename T>
	struct is_iterable
	{
	private:
		template<typename C> static char test(typename C::iterator*);
		template<typename C> static int  test(...);
	public:
		typedef bool type;
		typedef typename std::decay<T>::type iterable_type;
		static const bool value = (sizeof(test<iterable_type>(nullptr)) == sizeof(char));
	};

	template <typename T>
	struct is_input : std::integral_constant<bool, std::is_same<
			typename std::iterator_traits<T>::iterator_category,
			std::input_iterator_tag
		>::value
	> {};

	template<typename T1, typename T2>
	struct is_same_decay : std::integral_constant<bool,
		std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::value
	> {};

	template<size_t S, bool Signed> struct stdint_type { typedef bool type; };
	template<> struct stdint_type<1,false> { typedef  uint8_t type; };
	template<> struct stdint_type<1, true> { typedef   int8_t type; };
	template<> struct stdint_type<2,false> { typedef uint16_t type; };
	template<> struct stdint_type<2, true> { typedef  int16_t type; };
	template<> struct stdint_type<4,false> { typedef uint32_t type; };
	template<> struct stdint_type<4, true> { typedef  int32_t type; };
	template<> struct stdint_type<8,false> { typedef uint64_t type; };
	template<> struct stdint_type<8, true> { typedef  int64_t type; };

	template<size_t MinS, size_t MaxS, bool Signed1, bool Signed2>
	struct comb_int { using type = uint64_t; };

	template<size_t MinS, size_t MaxS>
	struct comb_int<MinS,MaxS,true,false> { using type = typename stdint_type<2*MaxS, true>::type; };

	template<size_t S, bool Signed1, bool Signed2>
	struct comb_int<S,S,Signed1,Signed2> { using type = typename stdint_type<2*S,true>::type; };
	
	template<size_t S, bool Signed>
	struct comb_int<S,S,Signed,Signed> { using type = typename stdint_type<S,Signed>::type; };

	template<size_t MaxS, bool Signed1, bool Signed2>
	struct comb_int<8,MaxS,Signed1,Signed2> { using type = stdint_type<8,Signed1>; };

	template<size_t MinS> struct comb_int<MinS,8,true,false> { using type = uint64_t; };
	template<> struct comb_int<8,8,false,true> { using type = uint64_t; };
	template<> struct comb_int<8,8,true,false> { using type = uint64_t; };
	template<> struct comb_int<8,8,true,true> { using type = int64_t; };
	
	template<typename T1, typename T2>
	struct comb_any
	{
	private:
		static const bool signed1 = (std::is_arithmetic<T1>::value && std::is_signed<T1>::value);
		static const bool signed2 = (std::is_arithmetic<T2>::value && std::is_signed<T2>::value);
		static const size_t min = (sizeof(T1) < sizeof(T2))? sizeof(T1) : sizeof(T2);
		static const size_t max = (sizeof(T1) > sizeof(T2))? sizeof(T1) : sizeof(T2);
		static const bool smin = (min == sizeof(T1))? signed1 : signed2;
		static const bool smax = (smin == signed1)? signed2 : signed1;
	public:
		typedef typename comb_int<min,max,smin,smax>::type type;
	};

	template<typename T> struct comb_any<T,T> { typedef T type; };
	template<typename T> struct comb_any<T,bool> { typedef T type; };
	template<typename T> struct comb_any<bool,T> { typedef T type; };

	template<> struct comb_any<bool,bool> { typedef bool type; };
	template<> struct comb_any<float,double> { typedef double type; };
	template<> struct comb_any<double,float> { typedef double type; };
	template<> struct comb_any<float,long double> { typedef long double type; };
	template<> struct comb_any<long double,float> { typedef long double type; };
	template<> struct comb_any<double,long double> { typedef long double type; };
	template<> struct comb_any<long double,double> { typedef long double type; };

	template<typename T1, typename T2>
	struct get_compatible
	{
		static const bool value = (std::is_integral<T1>::value && std::is_integral<T2>::value)
			|| (std::is_floating_point<T1>::value && std::is_floating_point<T2>::value);

		typedef typename comb_any<T1,T2>::type type;

		static bool is_not_unit(    bool const& x) { return !x; }
		static bool is_not_unit(  int8_t const& x) { return (x != 1) && (x != -1); }
		static bool is_not_unit( uint8_t const& x) { return (x != 1); }
		static bool is_not_unit( int16_t const& x) { return (x != 1) && (x != -1); }
		static bool is_not_unit(uint16_t const& x) { return (x != 1); }
		static bool is_not_unit( int32_t const& x) { return (x != 1) && (x != -1); }
		static bool is_not_unit(uint32_t const& x) { return (x != 1); }
		static bool is_not_unit( int64_t const& x) { return (x != 1) && (x != -1); }
		static bool is_not_unit(uint64_t const& x) { return (x != 1); }
	};
	
	template<typename Iterator, typename IteratorTag>
	struct iterator_utils
	{
		static void advance(Iterator & i, Iterator const& end, size_t n) {
			for (; i != end && n; --n) ++i;
		}
	};

	template<typename Iterator>
	struct iterator_utils<Iterator, std::random_access_iterator_tag>
	{
		static void advance(Iterator & i, Iterator const& end, size_t const& n) {
			i += n;
			if (end < i) i = end;
		}
	};


} // namespace utils

} // namespace qolor

#endif // QOLOR_UTILITIES_H__
