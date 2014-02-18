#ifndef QOLOR_RANGE_DRIVER_H__
#define QOLOR_RANGE_DRIVER_H__

#include "basic_iterable.h"
#include <cmath>

namespace qolor
{

namespace internal
{


template<typename T>
struct range_equality_comparer
{
	range_equality_comparer() = default;
	range_equality_comparer(range_equality_comparer&&) = default;
	range_equality_comparer(range_equality_comparer const&) = default;
	range_equality_comparer(T const&) {}
	bool operator()(T const& a, T const& b) const { return a == b; }
};

template<>
struct range_equality_comparer<float>
{
private:
	float e;
public:
	range_equality_comparer() : e(0) {}
	range_equality_comparer(range_equality_comparer&&) = default;
	range_equality_comparer(range_equality_comparer const&) = default;
	range_equality_comparer(float const& step) : e(step / 2) { if (e < 0) e = -e; }

	bool operator()(float const& a, float const& b) const {
		float diff = b - a;
		if (diff < 0) diff = -diff;
		return diff <= e;
	}
};

template<>
struct range_equality_comparer<double>
{
private:
	double e;
public:
	range_equality_comparer() : e(0) {}
	range_equality_comparer(range_equality_comparer&&) = default;
	range_equality_comparer(range_equality_comparer const&) = default;
	range_equality_comparer(double const& step) : e(step / 2) { if (e < 0) e = -e; }

	bool operator()(double const& a, double const& b) const {
		double diff = b - a;
		if (diff < 0) diff = -diff;
		return diff <= e;
	}
};

template<>
struct range_equality_comparer<long double>
{
private:
	long double e;
public:
	range_equality_comparer() : e(0) {}
	range_equality_comparer(range_equality_comparer&&) = default;
	range_equality_comparer(range_equality_comparer const&) = default;
	range_equality_comparer(long double const& step) : e(step / 2) { if (e < 0) e = -e; }

	bool operator()(long double const& a, long double const& b) const {
		long double diff = b - a;
		if (diff < 0) diff = -diff;
		return diff <= e;
	}
};


template<typename NumT>
class range_iterator
{
public:
	using value_type = typename std::decay<NumT>::type;
	using difference_type = value_type;
	using reference = value_type const&;
	using pointer = value_type const*;
	using iterator_category = std::input_iterator_tag;

private:

	value_type value_, step_;
	range_equality_comparer<NumT> eq_;

public:
	range_iterator() = delete;
	range_iterator(range_iterator const&) = default;
	range_iterator(range_iterator&&) = default;
	range_iterator(value_type const& v, value_type const& s = 1) : value_(v), step_(s), eq_(s) {}

	range_iterator & operator++() { value_ += step_; return *this; }
	range_iterator & operator--() { value_ -= step_; return *this; }

	range_iterator operator++(int) {
		range_iterator i(value_, step_);
		value_ += step_;
		return std::move(i);
	}

	range_iterator operator--(int) {
		range_iterator i(value_, step_);
		value_ -= step_;
		return std::move(i);
	}

	range_iterator & operator+=(difference_type const& n) { value_ += n * step_; return *this; }
	range_iterator & operator-=(difference_type const& n) { value_ -= n * step_; return *this; }
	
	range_iterator operator+(difference_type const& n) const {
		return std::move(range_iterator(value_ + n * step_, step_));
	}

	range_iterator operator-(difference_type const& n) const {
		return std::move(range_iterator(value_ - n * step_, step_));
	}

	bool operator==(value_type const& o) const { return  eq_(value_, o); }
	bool operator!=(value_type const& o) const { return !eq_(value_, o); }
	bool operator< (value_type const& o) const { return value_ <  o; }
	bool operator> (value_type const& o) const { return value_ >  o; }
	bool operator<=(value_type const& o) const { return value_ <= o; }
	bool operator>=(value_type const& o) const { return value_ >= o; }

	bool operator==(range_iterator const& o) const { return  eq_(value_, o.value_); }
	bool operator!=(range_iterator const& o) const { return !eq_(value_, o.value_); }
	bool operator< (range_iterator const& o) const { return value_ <  o.value_; }
	bool operator> (range_iterator const& o) const { return value_ >  o.value_; }
	bool operator<=(range_iterator const& o) const { return value_ <= o.value_; }
	bool operator>=(range_iterator const& o) const { return value_ >= o.value_; }

	reference operator*() const { return value_; }
	value_type operator[](difference_type const& n) const { return std::move(value_ + n * step_); }
	pointer operator->() const { return &value_; }
};

} // namespace internal


template <typename NumT1, typename NumT2>
typename std::enable_if<
	std::is_integral<NumT1>::value && std::is_integral<NumT2>::value
	,internal::iterable< internal::range_iterator<typename utils::get_compatible<NumT1,NumT2>::type> >
>::type
range(NumT1 const& from, NumT2 const& to,
	typename utils::get_compatible<NumT1,NumT2>::type const& step = 1,
	const bool& inclusive = false)
{
	using helper = utils::get_compatible<NumT1,NumT2>;
	using num_t = typename utils::get_compatible<NumT1,NumT2>::type;
	using iter_t = internal::range_iterator<num_t>;
	num_t end = to;
	if (helper::is_not_unit(step))
		end += step - ((end - from) % step);
	if (inclusive) end += step;
	return internal::iterable<iter_t>(iter_t(from, step), iter_t(end, step));
}


template <typename NumT1, typename NumT2>
typename std::enable_if<
	std::is_floating_point<NumT1>::value && std::is_floating_point<NumT2>::value
	,internal::iterable< internal::range_iterator<typename utils::get_compatible<NumT1,NumT2>::type> >
>::type
range(NumT1 const& from, NumT2 const& to,
	typename utils::get_compatible<NumT1,NumT2>::type const& step = 1,
	const bool& inclusive = false)
{
	using num_t = typename utils::get_compatible<NumT1,NumT2>::type;
	using iter_t = internal::range_iterator<num_t>;
	internal::range_equality_comparer<num_t> eq(step);
	num_t end = num_t(from) + step * std::floor((num_t(to) - from) / step);
	if (inclusive || !eq(end,to))
		end += step;
	return internal::iterable<iter_t>(iter_t(from, step), iter_t(end, step));
}


} // namespace qolor

#endif // QOLOR_RANGE_DRIVER_H__
