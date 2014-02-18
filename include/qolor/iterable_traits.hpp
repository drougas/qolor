#ifndef QOLOR_ITERABLE_TRAITS_HPP__
#define QOLOR_ITERABLE_TRAITS_HPP__

#include "iterator_utilities.hpp"

namespace qolor
{

namespace utils
{

// http://stackoverflow.com/questions/4335962/how-to-check-if-a-template-parameter-is-an-iterator-type-or-not
//
// http://stackoverflow.com/questions/9530928/checking-a-member-exists-possibly-in-a-base-class-c11-version
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions

template<typename Iterable>
struct iterable_traits
{
private:
	struct anyx { template<typename A> anyx(A const&); };

	template<typename T> static auto test_begin(T&& t) -> decltype(t.begin());
	template<typename T, size_t N> T* test_begin(T (&arr)[N]);
	static void test_begin(anyx const&);

	template<typename T> static auto test_end(T&& t) -> decltype(t.end());
	template<typename T, size_t N> T* test_end(T (&arr)[N]);
	static void test_end(anyx const&);

	typedef decltype(test_begin(std::declval<Iterable>())) begin_type;
	typedef decltype(test_end(std::declval<Iterable>())) end_type;

	static constexpr bool types_match = std::is_same<begin_type,end_type>();
	static constexpr bool end_is_generic = std::is_same<generic_end_iterator,end_type>();

	typedef typename std::conditional<types_match || end_is_generic, begin_type, void>::type iterator;
	typedef iterator_traits_ex<iterator> itraits;

public:
	static constexpr bool is_iterable = itraits::is_iterator;

	typedef typename itraits::deref_type deref_type;
	typedef typename itraits::difference_type difference_type;
	typedef typename itraits::value_type value_type;
	typedef typename itraits::reference reference;
	typedef typename itraits::pointer pointer;
	typedef typename itraits::iterator_category iterator_category;

	static constexpr bool is_readable = itraits::is_readable;
	static constexpr bool is_writable = itraits::is_writable;
	static constexpr bool is_bidirectional = itraits::is_bidirectional;
	static constexpr bool is_resetable = itraits::is_resetable;
};


template<typename T>
struct is_iterable : public std::integral_constant<bool, iterable_traits<T>::is_iterable> {};

template<typename T>
struct is_readable_iterable : public std::integral_constant<bool, iterable_traits<T>::is_readable> {};

template<typename T>
struct is_writable_iterable : public std::integral_constant<bool, iterable_traits<T>::is_writable> {};

template<typename T>
struct is_bidirectional_iterable : public std::integral_constant<bool, iterable_traits<T>::is_bidirectional> {};

template<typename T>
struct is_resetable_iterable : public std::integral_constant<bool, iterable_traits<T>::is_resetable> {};


template<typename Iterable>
struct reverse_iterable_traits
{
private:
	struct anyx { template<typename A> anyx(A const&); };

	template<typename T> static auto test_rbegin(T&& t) -> decltype(t.rbegin());
	template<typename T, size_t N> T* test_rbegin(T (&arr)[N]);
	static void test_rbegin(anyx const&);

	template<typename T> static auto test_rend(T&& t) -> decltype(t.rend());
	template<typename T, size_t N> T* test_rend(T (&arr)[N]);
	static void test_rend(anyx const&);

	typedef decltype(test_rbegin(std::declval<Iterable>())) begin_type;
	typedef decltype(test_rend(std::declval<Iterable>())) end_type;

	static constexpr bool types_match = std::is_same<begin_type,end_type>();
	static constexpr bool end_is_generic = std::is_same<generic_end_iterator,end_type>();

	typedef typename std::conditional<types_match || end_is_generic, begin_type, void>::type iterator;
	typedef iterator_traits_ex<iterator> itraits;

public:
	static constexpr bool is_iterable = itraits::is_iterator;

	typedef typename itraits::deref_type deref_type;
	typedef typename itraits::difference_type difference_type;
	typedef typename itraits::value_type value_type;
	typedef typename itraits::reference reference;
	typedef typename itraits::pointer pointer;
	typedef typename itraits::iterator_category iterator_category;

	static constexpr bool is_readable = itraits::is_readable;
	static constexpr bool is_writable = itraits::is_writable;
	static constexpr bool is_bidirectional = itraits::is_bidirectional;
	static constexpr bool is_resetable = itraits::is_resetable;
};


template<typename T>
struct is_reverse_iterable : public std::integral_constant<bool, reverse_iterable_traits<T>::is_iterable> {};

template<typename T>
struct is_reverse_readable_iterable : public std::integral_constant<bool, reverse_iterable_traits<T>::is_readable> {};

template<typename T>
struct is_reverse_writable_iterable : public std::integral_constant<bool, reverse_iterable_traits<T>::is_writable> {};

template<typename T>
struct is_reverse_bidirectional_iterable : public std::integral_constant<bool, reverse_iterable_traits<T>::is_bidirectional> {};

template<typename T>
struct is_reverse_resetable_iterable : public std::integral_constant<bool, reverse_iterable_traits<T>::is_resetable> {};


} // namespace utils

} // namespace qolor

#endif // QOLOR_ITERABLE_TRAITS_HPP__
