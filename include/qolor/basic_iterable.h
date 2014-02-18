#ifndef QOLOR_BASIC_ITERABLE_H__
#define QOLOR_BASIC_ITERABLE_H__

#include "utilities.h"
#include "functional_traits.hpp"
#include "select_iterator.hpp"
#include "predicate_iterator.hpp"
#include "join_iterator.hpp"
#include <vector>

namespace qolor
{

namespace internal
{

template <typename IteratorType>
class iterable
{
private:
	typedef qolor::utils::iterator_traits_ex<IteratorType> iter_traits;

	static_assert(iter_traits::is_iterator, "IteratorType is not an iterator");

public:
	typedef IteratorType                            iterator;
	typedef typename iter_traits::difference_type   difference_type;
	typedef typename iter_traits::value_type        value_type;
	typedef typename iter_traits::pointer           pointer;
	typedef typename iter_traits::reference         reference;
	typedef typename iter_traits::iterator_category iterator_category;

	static constexpr bool is_readable   = iter_traits::is_readable;
	static constexpr bool is_writable   = iter_traits::is_writable;
	static constexpr bool is_bidirectional = iter_traits::is_bidirectional;
	static constexpr bool is_resetable  = iter_traits::is_resetable;

private:
	struct count_func
	{
		size_t n_;
		count_func() = delete;
		count_func(count_func const&) = default;
		count_func(count_func&&) = default;
		count_func(size_t const& n) : n_(n) {}

		bool operator() (value_type const&) {
			if (!n_) return false;
			--n_;
			return true;
		}
	};

	struct max_advance_other
	{
		iterator operator() (iterator b, iterator const& e, size_t c) const {
			while (c && (b != e)) { ++b; --c; }
			return std::move(b);
		}
	};

	struct max_advance_bidir
	{
		iterator operator() (iterator const& b, iterator const& e, size_t const& c) const {
			return (e - b < c)? std::move(b + c) : e;
		}
	};

	max_advance_bidir max_advance(std::random_access_iterator_tag);
	max_advance_other max_advance(...);

	struct get_last_other
	{
		iterator operator() (iterator b, iterator const& e) const {
			if (b == e) return std::move(b);
			for (iterator i(b); (++i) != e; ) b = i;
			return std::move(b);
		}
	};

	struct get_last_bidir
	{
		iterator operator() (iterator const& b, iterator e) const {
			if (b < e) --e;
			return std::move(e);
		}
	};

	get_last_bidir get_last(std::random_access_iterator_tag);
	get_last_other get_last(...);

	iterator begin_, end_;

	typedef decltype(*begin_) deref_type;

public:
	iterable() = delete;
	iterable(iterable const&) = default;
	iterable(iterable&&) = default;
	iterable& operator=(iterable const&) = default;
	iterable& operator=(iterable&&) = default;

	template<typename B, typename E>
	iterable(B&& b, E&& e) : begin_(std::forward<B>(b)), end_(std::forward<E>(e)) {}

	iterator const & begin() const { return begin_; }
	iterator const & end() const { return end_; }

	bool empty() const { return begin_ == end_; }

	template <typename F>
	iterable<select_iterator<iterator, F> > select(F&& f) {
		using iter_t = select_iterator<iterator, F>;
		iter_t b(begin_, std::forward<F>(f)), e(end_, std::forward<F>(f));
		return iterable<iter_t>(std::move(b), std::move(e));
	}

	template <typename F>
	iterable<where_iterator<iterator, typename std::decay<F>::type> > where(F&& f) {
		typedef typename std::decay<F>::type ftype;
		static_assert(utils::is_predicate<ftype, value_type const&>(), "Parameter is not an appropriate functional");
		typedef where_iterator<iterator, ftype> iter_t;
		iter_t b(begin_, end_, std::forward<F>(f));
		iter_t e(end_, end_, std::forward<F>(f));
		return iterable<iter_t>(std::move(b), std::move(e));
	}

	template <typename F>
	iterable<while_iterator<iterator, typename std::decay<F>::type> > take_while(F&& f) {
		typedef typename std::decay<F>::type ftype;
		static_assert(utils::is_predicate<ftype, value_type const&>(), "Parameter is not an appropriate functional");
		typedef while_iterator<iterator, ftype> iter_t;
		iter_t b(begin_, end_, std::forward<F>(f));
		iter_t e(end_, end_, std::forward<F>(f));
		return iterable<iter_t>(std::move(b), std::move(e));
	}

	template <typename F>
	iterable skip_while(F&& f) const {
		static_assert(utils::is_predicate<F, value_type const&>(), "Parameter is not an appropriate functional");
		iterator i(begin_);
		while (i != end_ && f(*i)) ++i;
		return iterable(std::move(i), end_);
	}

	iterable<while_iterator<iterator, count_func> > take(size_t const& count) {
		return std::move(take_while(count_func(count)));
	}

	iterable<iterator> skip(size_t const& count) {
		typedef decltype(max_advance(std::declval<iterator_category>())) adv_t;
		static const adv_t adv;
		return iterable<iterator>(std::move(adv(begin_, end_, count)), end_);
	}

	typename std::enable_if<is_readable, deref_type>::type
	first() {
		return std::forward<value_type>(*begin_);
	}

	typename std::enable_if<is_readable, deref_type>::type
	last() {
		typedef decltype(get_last(std::declval<iterator_category>())) getlast_t;
		static const getlast_t getlast;
		return std::forward<value_type>(*(getlast(begin_, end_)));
	}

	template<typename J, typename P>
	typename std::enable_if<
		is_readable && iterable<J>::is_readable
		,join_iterator<iterator, J, P>
	>::type
	join(iterable<J>&& o, P&& pred) {
		using rtype = join_iterator<iterator, J, P>;
		return rtype(begin_, end_, std::move(o.begin()), std::move(o.end()), std::forward<P>(pred));
	}

	template <typename Predicate>
	typename std::enable_if<is_readable,bool>::type
	contains(value_type const& value, Predicate&& pred) {
		for (auto& v : *this)
			if (pred(value,v)) return true;
		return false;
	}

	template <typename Predicate>
	typename std::enable_if<is_readable,bool>::type
	contains(value_type const& value, Predicate&& pred) const {
		for (auto const& v : *this)
			if (pred(value,v)) return true;
		return false;
	}

	typename std::enable_if<is_readable,bool>::type
	contains(value_type const& value) {
		for (auto& v : *this)
			if (value == v) return true;
		return false;
	}

	typename std::enable_if<is_readable,bool>::type
	contains(value_type const& value) const {
		for (auto const& v : *this)
			if (value == v) return true;
		return false;
	}

	template <typename F>
	typename std::enable_if<is_readable,value_type>::type
	aggregate(F&& f) {
		static_assert(std::is_trivial<value_type>::value || std::is_default_constructible<value_type>::value,
			"value_type must be default constructible.");
		if (begin_ == end_) return value_type();
		iterator i(begin_);
		value_type acc(*i);
		while ((++i) != end_) acc = f(acc, *i);
		return std::move(acc);
	}

	typename std::enable_if<is_readable,value_type>::type
	sum() {
		typedef value_type const& ref;
		return aggregate([](ref a, ref b){ return a + b; });
	}

	typename std::enable_if<is_readable,std::vector<value_type>>::type
	to_vector() {
		std::vector<value_type> ret;
		for (auto& v : *this) ret.push_back(v);
		return std::move(ret);
	}

	template <typename I2>
	typename std::enable_if<is_readable && qolor::utils::is_writable_iterator<I2>(), I2&>::type
	update(I2& out) {
		return std::copy(begin_, end_, out);
	}
};


} // namespace internal


template<typename IteratorType1, typename IteratorType2>
typename std::enable_if<
	utils::is_same_decay<IteratorType1,IteratorType2>::value
	&& utils::is_iterator<IteratorType1>()
	&& utils::is_iterator<IteratorType2>()
	,internal::iterable<typename std::decay<IteratorType1>::type>
>::type from(IteratorType1&& begin, IteratorType2&& end)
{
	using t1 = typename std::decay<IteratorType1>::type;
	return internal::iterable<t1>(std::forward<IteratorType1>(begin), std::forward<IteratorType2>(end));
}


template<typename ValueT>
internal::iterable<ValueT*> from(ValueT* const& begin, typename std::iterator_traits<ValueT*>::difference_type const& n)
{
	return internal::iterable<ValueT*>(begin, begin + n);
}


template<typename ValueT1, typename ValueT2>
typename std::enable_if<
	utils::is_same_decay<ValueT1,ValueT2>::value
	,internal::iterable<ValueT1*>
>::type from(ValueT1* const& begin, ValueT2* const& end)
{
	return internal::iterable<ValueT1*>(begin, begin + (end - begin));
}


template<typename IterableType>
auto from(IterableType&& c) -> typename std::enable_if<
	utils::is_iterable<IterableType>::value
	,internal::iterable<decltype(c.begin())>
>::type
{
	return internal::iterable<decltype(c.begin())>(c.begin(), c.end());
}


} // namespace qolor

#endif // QOLOR_BASIC_ITERABLE_H__
