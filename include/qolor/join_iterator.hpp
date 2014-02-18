#ifndef QOLOR_JOIN_ITERATOR_HPP__
#define QOLOR_JOIN_ITERATOR_HPP__

#include "iterator_utilities.hpp"
#include "functional_traits.hpp"
#include <type_traits>

namespace qolor
{

namespace internal
{

template <typename InnerType1, typename InnerType2, typename Predicate>
class join_iterator
{
private:
	typedef ::qolor::utils::iterator_traits_ex<InnerType1> traits1;
	typedef ::qolor::utils::iterator_traits_ex<InnerType2> traits2;
	typedef typename traits1::deref_type deref_t1;
	typedef typename traits2::deref_type deref_t2;
	typedef typename traits1::value_type value_t1;
	typedef typename traits2::value_type value_t2;

	static_assert(traits1::is_readable, "First template argument is not a readable iterator.");
	static_assert(traits2::is_readable, "Second template argument is not a readable iterator.");
	static_assert(traits2::is_resetable, "Second template argument is not a resetable iterator. You may want to use iterable::to_vector().");
	static_assert(::qolor::utils::is_predicate<Predicate, deref_t1, deref_t2>(), "Invalid Predicate type.");

	InnerType1 cur1_, end1_;
	InnerType2 begin2_, end2_, cur2_;
	Predicate pred_;

	//typename std::enable_if<is_input1, value_t1>::type value1_;
	template<typename T> struct is_tuple : public std::false_type {};
	template<typename... Ts> struct is_tuple<std::tuple<Ts...>> : public std::true_type {};

	static constexpr bool is_tuple1 = is_tuple<value_t1>();
	static constexpr bool is_tuple2 = is_tuple<value_t2>();

	
public:
	typedef typename InnerType1::difference_type difference_type;
	typedef decltype(std::tuple_cat(std::declval<deref_t1>(), std::declval<deref_t2>())) value_type;
	typedef value_type&& reference;
	typedef std::shared_ptr<value_type> pointer;
	typedef std::input_iterator_tag iterator_category;

	join_iterator() = delete;
	join_iterator(join_iterator const&) = default;
	join_iterator(join_iterator&&) = default;

	template <typename I1, typename I2, typename P>
	join_iterator(I1&& begin1, I1&& end1, I2&& begin2, I2&& end2, P&& pred)
		: cur1_(std::forward<I1>(begin1)), end1_(std::forward<I1>(end1)),
		begin2_(std::forward<I2>(begin2)), end2_(std::forward<I2>(end2)),
		pred_(std::forward<P>(pred)) {
		cur2_ = begin2_;
	}

	bool operator==(join_iterator const& o) const { return (cur1_ == o.cur1_) && (cur2_ == o.cur2_); }
	bool operator!=(join_iterator const& o) const { return (cur1_ != o.cur1_) || (cur2_ != o.cur2_); }
	
	join_iterator& operator++() {
		do {
			++cur2_;
			while ((cur2_ == end2_) && (cur1_ != end1_)) {
				++cur1_;
				cur2_ = begin2_;
			}
		} while (cur1_ != end1_ && !pred_(*cur1_,*cur2_));
		return *this;
	}

	/*typename std::enable_if<is_input1, join_iterator &>::type operator++() {
		bool check_q;
		do {
			++cur2_;
			check_q = (cur1_ != end1_);
			while ((cur2_ == end2_) && check_q) {
				check_q = ((++cur1_) != end1_);
				if (check_q) {
					value1_ = *cur1_;
					cur2_ = begin2_;
				}
			}
		} while (check_q && !pred_(value1_, *cur2_));
		return *this;
	}*/

	join_iterator operator++(int) { join_iterator i(*this); ++(*this); return i; }

	value_type operator*() const { return std::tuple_cat(*cur1_, *cur2_); }
	pointer operator->() const { return pointer(new value_type(*this)); }
};

} // namespace internal

} // namespace qolor

#endif // QOLOR_JOIN_ITERATOR_HPP__
