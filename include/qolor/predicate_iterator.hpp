#ifndef QOLOR_PREDICATE_ITERATOR_HPP__
#define QOLOR_PREDICATE_ITERATOR_HPP__

#include "iterator_utilities.hpp"
#include "functional_traits.hpp"
#include <iterator>
#include <type_traits>
#include <utility>

namespace qolor
{

namespace internal
{

template <typename InnerType, typename Pred>
class generic_predicate_iterator
{
public:
	typedef typename std::decay<InnerType>::type inner_type;
	typedef std::iterator_traits<inner_type> inner_traits;

	typedef decltype(*(std::declval<inner_type>())) deref_type;
	typedef typename std::decay<Pred>::type pred_type;
	typedef qolor::utils::is_functional_predicate<pred_type,deref_type> is_pred;

	static_assert(is_pred(), "Pred is not an appropriate functional predicate!");

	mutable inner_type cur_;
	inner_type end_;
	pred_type pred_;
	mutable bool started_;

	generic_predicate_iterator() = delete;
	generic_predicate_iterator(generic_predicate_iterator const&) = default;
	generic_predicate_iterator(generic_predicate_iterator&&) = default;

	template <typename B, typename E, typename P>
	generic_predicate_iterator(B&& b, E&& e, P&& p)
	: cur_(std::forward<B>(b)), end_(std::forward<E>(e)), pred_(std::forward<P>(p)), started_(false) {}

	typedef typename inner_traits::difference_type difference_type;
	typedef typename inner_traits::value_type value_type;
	typedef typename inner_traits::reference reference;
	typedef typename inner_traits::pointer pointer;

	typedef typename inner_traits::iterator_category inner_cat;
	typedef typename std::conditional<
		std::is_same<inner_cat, std::bidirectional_iterator_tag>() || std::is_same<inner_cat, std::random_access_iterator_tag>(),
		std::forward_iterator_tag, inner_cat>::type iterator_category;

}; // class generic_predicate_iterator


template <typename InnerType, typename Pred>
class where_iterator : public generic_predicate_iterator<InnerType,Pred>
{
private:
	typedef generic_predicate_iterator<InnerType,Pred> base_t;

	void ignite() const {
		while ((base_t::cur_ != base_t::end_) && !base_t::pred_(*base_t::cur_))
			++base_t::cur_;
		base_t::started_ = true;
	}

public:
	typedef typename base_t::difference_type difference_type;
	typedef typename base_t::value_type value_type;
	typedef typename base_t::reference reference;
	typedef typename base_t::pointer pointer;
	typedef typename base_t::iterator_category iterator_category;

	where_iterator() = delete;
	where_iterator(where_iterator const&) = default;
	where_iterator(where_iterator&&) = default;

	template <typename BeginIter, typename EndIter, typename Predicate>
	where_iterator(BeginIter&& b, EndIter&& e, Predicate&& p)
	: base_t(std::forward<BeginIter>(b), std::forward<EndIter>(e), std::forward<Predicate>(p)) {}

	where_iterator & operator++() {
		if (!base_t::started_) ignite();
		do
			++base_t::cur_;
		while (base_t::cur_ != base_t::end_ && !base_t::pred_(*base_t::cur_));
		return *this;
	}

	where_iterator operator++(int) {
		where_iterator w(*this);
		++(*this);
		return std::move(w);
	}

	bool operator==(where_iterator const& o) const {
		if (!base_t::started_) ignite();
		if (!((const base_t&)o).started_) o.ignite();
		return base_t::cur_ == o.cur_;
	}

	bool operator!=(where_iterator const& o) const { return !(*this == o); }

	bool operator==(qolor::utils::generic_end_iterator const&) {
		if (!base_t::started_) ignite();
		return base_t::cur_ == base_t::end_;
	}

	bool operator!=(qolor::utils::generic_end_iterator const&) {
		if (!base_t::started_) ignite();
		return base_t::cur_ != base_t::end_;
	}

	typename base_t::deref_type operator*() {
		if (!base_t::started_) ignite();
		return std::forward<typename base_t::deref_type>(*base_t::cur_);
	}
	
	pointer operator->() {
		if (!base_t::started_) ignite();
		return base_t::cur_.operator->();
	}

	friend bool operator==(qolor::utils::generic_end_iterator const&, where_iterator const& b) {
		if (!b.started_) b.ignite();
		return b.cur_ == b.end_;
	}

	friend bool operator!=(qolor::utils::generic_end_iterator const&, where_iterator const& b) {
		if (!b.started_) b.ignite();
		return b.cur_ != b.end_;
	}

}; // class where_iterator


template <typename InnerType, typename Pred>
class while_iterator : public generic_predicate_iterator<InnerType,Pred>
{
private:
	typedef generic_predicate_iterator<InnerType,Pred> base_t;

	void ignite() const {
		if ((base_t::cur_ != base_t::end_) && !base_t::pred_(*base_t::cur_))
			base_t::cur_ = base_t::end_;
		base_t::started_ = true;
	}

public:
	typedef typename base_t::difference_type difference_type;
	typedef typename base_t::value_type value_type;
	typedef typename base_t::reference reference;
	typedef typename base_t::pointer pointer;
	typedef typename base_t::iterator_category iterator_category;

	while_iterator() = delete;
	while_iterator(while_iterator const&) = default;
	while_iterator(while_iterator&&) = default;

	template <typename BeginIter, typename EndIter, typename Predicate>
	while_iterator(BeginIter&& b, EndIter&& e, Predicate&& p)
	: base_t(std::forward<BeginIter>(b), std::forward<EndIter>(e), std::forward<Predicate>(p)) {}

	while_iterator & operator++() {
		if (!base_t::started_) ignite();
		++base_t::cur_;
		if ((base_t::cur_ != base_t::end_) && !base_t::pred_(*base_t::cur_))
			base_t::cur_ = base_t::end_;
		return *this;
	}

	while_iterator operator++(int) {
		while_iterator w(*this);
		++(*this);
		return std::move(w);
	}

	bool operator==(while_iterator const& o) const {
		if (!base_t::started_) ignite();
		if (!((const base_t&)o).started_) o.ignite();
		return base_t::cur_ == o.cur_;
	}

	bool operator!=(while_iterator const& o) const { return !(*this == o); }

	bool operator==(qolor::utils::generic_end_iterator const&) {
		if (!base_t::started_) ignite();
		return base_t::cur_ == base_t::end_;
	}

	bool operator!=(qolor::utils::generic_end_iterator const&) {
		if (!base_t::started_) ignite();
		return base_t::cur_ != base_t::end_;
	}

	typename base_t::deref_type operator*() {
		if (!base_t::started_) ignite();
		return std::forward<typename base_t::deref_type>(*base_t::cur_);
	}
	
	pointer operator->() {
		if (!base_t::started_) ignite();
		return base_t::cur_.operator->();
	}

	friend bool operator==(qolor::utils::generic_end_iterator const&, while_iterator const& b) {
		if (!b.started_) b.ignite();
		return b.cur_ == b.end_;
	}

	friend bool operator!=(qolor::utils::generic_end_iterator const&, while_iterator const& b) {
		if (!b.started_) b.ignite();
		return b.cur_ != b.end_;
	}

}; // class while_iterator


} // namespace internal

} // namespace qolor

#endif // QOLOR_PREDICATE_ITERATOR_HPP__
