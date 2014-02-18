#ifndef QOLOR_SELECT_ITERATOR_HPP__
#define QOLOR_SELECT_ITERATOR_HPP__

#include "utilities.h"
#include "functional_traits.hpp"
#include <iterator>
#include <type_traits>
#include <memory>

namespace qolor
{

namespace internal
{

template <typename InnerType, typename FuncType>
class select_iterator
{
private:
	using inner_type = typename std::decay<InnerType>::type;
	using func_type = typename std::decay<FuncType>::type;

	inner_type cur_;
	func_type func_;
	
public:
	typedef typename std::iterator_traits<inner_type>::difference_type difference_type;
	typedef typename std::decay<decltype(func_(*cur_))>::type value_type;
	typedef typename std::add_rvalue_reference<value_type>::type reference;
	typedef typename std::add_pointer<value_type>::type pointer;
	typedef std::input_iterator_tag iterator_category;

	select_iterator() = delete;
	select_iterator(select_iterator const&) = default;
	select_iterator(select_iterator&&) = default;

	template <typename I, typename F>
	select_iterator(I&& it, F&& f) : cur_(std::forward<I>(it)), func_(std::forward<F>(f)) {}

	bool operator==(inner_type const& o) const { return cur_ == o; }
	bool operator!=(inner_type const& o) const { return cur_ != o; }
	bool operator==(select_iterator const& o) const { return cur_ == o.cur_; }
	bool operator!=(select_iterator const& o) const { return cur_ != o.cur_; }

	select_iterator & operator++() { ++cur_; return *this; }
	select_iterator operator++(int) { return select_iterator(cur_++, func_); }
	select_iterator & operator--() { --cur_; return *this; }
	select_iterator operator--(int) { return select_iterator(cur_--, func_); }
	select_iterator & operator+=(difference_type const& diff) { std::advance(cur_, diff); return *this; }
	select_iterator & operator-=(difference_type const& diff) { std::advance(cur_, -diff); return *this; }

	select_iterator operator+(difference_type const& diff) const {
		select_iterator ret(*this);
		std::advance(ret.cur_, diff);
		return std::move(ret);
	}

	select_iterator operator-(difference_type const& diff) const {
		select_iterator ret(*this);
		std::advance(ret.cur_, -diff);
		return std::move(ret);
	}

	difference_type operator-(select_iterator const& other) const { return cur_ - other.cur_; }
	value_type operator*() { return std::move(func_(*cur_)); }

	std::unique_ptr<value_type> operator->() {
		return std::unique_ptr<value_type>(new value_type(std::move(func_(*cur_))));
	}
};

} // namespace internal

} // namespace qolor

#endif // QOLOR_SELECT_ITERATOR_HPP__
