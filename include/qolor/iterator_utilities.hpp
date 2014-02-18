#ifndef QOLOR_ITERATOR_UTILITIES_HPP__
#define QOLOR_ITERATOR_UTILITIES_HPP__

#include <iterator>

namespace qolor
{

namespace utils
{

// http://stackoverflow.com/questions/4335962/how-to-check-if-a-template-parameter-is-an-iterator-type-or-not
//
// http://stackoverflow.com/questions/9530928/checking-a-member-exists-possibly-in-a-base-class-c11-version
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions


template<typename Iter>
struct iterator_traits_ex
{
private:
	template<class C> static bool deduce(std::iterator_traits<C>);     // Iterator traits
	template<class C> static void* deduce(C*);                         // Pointer
	template<class C> static typename C::iterator_category* deduce(C); // Iterator
	static bool deduce(...);                                           // Common case

	// http://stackoverflow.com/questions/4434569/is-it-possible-to-use-sfinae-templates-to-check-if-an-operator-exists
	struct anyx { template<typename A> anyx(A const&); };
	template<typename I> static auto can_increment(I&& i) -> typename std::decay<decltype(++i)>::type;
	static void can_increment(anyx const&);

	template<typename I, bool v>
	struct ttraits {
		typedef std::iterator_traits<I> traits;
		typedef decltype(*(std::declval<I>())) deref_type;

		typedef typename traits::difference_type difference_type;
		typedef typename traits::value_type value_type;
		typedef typename traits::reference reference;
		typedef typename traits::pointer pointer;
		typedef typename traits::iterator_category iterator_category;
	};

	template<typename I>
	struct ttraits<I, false> {
		typedef void traits;
		typedef void deref_type;

		typedef void difference_type;
		typedef void value_type;
		typedef void reference;
		typedef void pointer;
		typedef void iterator_category;
	};

	static constexpr bool test_deduce = (sizeof(deduce(std::declval<Iter>())) == sizeof(void*));
	static constexpr bool test_increment = std::is_same<Iter, decltype(can_increment(std::declval<Iter>()))>();

	typedef ttraits<Iter, (test_deduce && test_increment)> vtraits;

public:
	static constexpr bool is_iterator = test_deduce && test_increment;

	typedef typename vtraits::traits inner_traits;
	typedef typename vtraits::deref_type deref_type;

	typedef typename vtraits::difference_type difference_type;
	typedef typename vtraits::value_type value_type;
	typedef typename vtraits::reference reference;
	typedef typename vtraits::pointer pointer;
	typedef typename vtraits::iterator_category iterator_category;

	static constexpr bool is_input = std::is_same<iterator_category, std::input_iterator_tag>();
	static constexpr bool is_output = std::is_same<iterator_category, std::output_iterator_tag>();

	static constexpr bool is_readable = is_iterator && !is_output;

	static constexpr bool is_writable = is_iterator && !is_input
		&& std::is_lvalue_reference<deref_type>()
		&& !std::is_const<typename std::remove_reference<deref_type>::type>();

	static constexpr bool is_bidirectional = is_iterator
		&& (std::is_same<iterator_category, std::bidirectional_iterator_tag>()
		|| std::is_same<iterator_category, std::random_access_iterator_tag>());

	static constexpr bool is_resetable = is_iterator && !is_input && !is_output;
};

template<typename T>
struct is_iterator : public std::integral_constant<bool, iterator_traits_ex<T>::is_iterator> {};

template<typename T>
struct is_readable_iterator : public std::integral_constant<bool, iterator_traits_ex<T>::is_readable> {};

template<typename T>
struct is_writable_iterator : public std::integral_constant<bool, iterator_traits_ex<T>::is_writable> {};

template<typename T>
struct is_bidirectional_iterator : public std::integral_constant<bool, iterator_traits_ex<T>::is_bidirectional> {};

template<typename T>
struct is_resetable_iterator : public std::integral_constant<bool, iterator_traits_ex<T>::is_resetable> {};

struct generic_end_iterator {};

} // namespace utils

} // namespace qolor

#endif // QOLOR_ITERATOR_UTILITIES_HPP__
