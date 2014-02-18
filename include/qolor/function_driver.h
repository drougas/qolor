#ifndef QOLOR_FUNCTION_DRIVER_HPP__
#define QOLOR_FUNCTION_DRIVER_HPP__

#include "basic_iterable.h"
#include "functional_traits.hpp"

namespace qolor
{

namespace internal
{

template<typename Func, typename Condition>
class custom_func_iterator
{
public:
	typedef typename std::decay<typename qolor::utils::resultof<Func>::type>::type value_type;
	typedef typename std::ptrdiff_t difference_type;
	typedef value_type const& reference;
	typedef value_type const* pointer;
	typedef std::input_iterator_tag iterator_category;

private:
	mutable Func func_;
	mutable Condition cond_;
	mutable value_type buf_;
	mutable bool cond_result_;
	mutable bool needs_ignition_;

	inline void ignite() const {
		if (needs_ignition_) {
			buf_ = func_();
			cond_result_ = cond_(buf_) && true;
			needs_ignition_ = false;
		}
	}

public:
	custom_func_iterator() = delete;
	custom_func_iterator(custom_func_iterator const&) = default;
	custom_func_iterator(custom_func_iterator&&) = default;

	template<typename F, typename C>
	custom_func_iterator(F&& f, C&& c, bool const& do_iterate)
		: func_(std::forward<F>(f)), cond_(std::forward<C>(c)),
		cond_result_(do_iterate), needs_ignition_(do_iterate) {}

	custom_func_iterator & operator++() {
		ignite();
		if (cond_result_) {
			buf_ = func_();
			cond_result_ = cond_(buf_) && true;
		}
		return *this;
	}

	custom_func_iterator operator++(int) {
		custom_func_iterator i(*this);
		++(*this);
		return std::move(i);
	}
	
	bool operator==(custom_func_iterator const& o) const { ignite(); o.ignite(); return cond_result_ == o.cond_result_; }
	bool operator!=(custom_func_iterator const& o) const { ignite(); o.ignite(); return cond_result_ != o.cond_result_; }

	bool operator==(qolor::utils::generic_end_iterator const&) const { ignite(); return !cond_result_; }
	bool operator!=(qolor::utils::generic_end_iterator const&) const { ignite(); return  cond_result_; }

	friend bool operator==(qolor::utils::generic_end_iterator const& a, custom_func_iterator const& b) { return b == a; }
	friend bool operator!=(qolor::utils::generic_end_iterator const& a, custom_func_iterator const& b) { return b != a; }

	reference operator*() const { ignite(); return buf_; }
	pointer operator->() const { ignite(); return &buf_; }
};


template<typename Func, typename Condition>
class custom_func_void_iterator
{
public:
	typedef void value_type;
	typedef typename std::ptrdiff_t difference_type;
	typedef void reference;
	typedef void pointer;
	typedef std::input_iterator_tag iterator_category;

private:
	mutable Func func_;
	mutable Condition cond_;
	mutable bool cond_result_;
	mutable bool needs_ignition_;

	inline void ignite() const {
		if (needs_ignition_) {
			func_();
			cond_result_ = cond_() && true;
			needs_ignition_ = false;
		}
	}

public:
	custom_func_void_iterator() = delete;
	custom_func_void_iterator(custom_func_void_iterator const&) = default;
	custom_func_void_iterator(custom_func_void_iterator&&) = default;

	template<typename F, typename C>
	custom_func_void_iterator(F&& f, C&& c, bool const& do_iterate)
		: func_(std::forward<F>(f)), cond_(std::forward<C>(c)),
		cond_result_(do_iterate), needs_ignition_(do_iterate) {}

	custom_func_void_iterator & operator++() {
		ignite();
		if (cond_result_) {
			func_();
			cond_result_ = cond_() && true;
		}
		return *this;
	}

	custom_func_void_iterator operator++(int) {
		custom_func_void_iterator i(*this);
		++(*this);
		return std::move(i);
	}
	
	bool operator==(custom_func_void_iterator const& o) const { ignite(); o.ignite(); return cond_result_ == o.cond_result_; }
	bool operator!=(custom_func_void_iterator const& o) const { ignite(); o.ignite(); return cond_result_ != o.cond_result_; }

	bool operator==(qolor::utils::generic_end_iterator const&) const { ignite(); return !cond_result_; }
	bool operator!=(qolor::utils::generic_end_iterator const&) const { ignite(); return  cond_result_; }

	friend bool operator==(qolor::utils::generic_end_iterator const& a, custom_func_void_iterator const& b) { return b == a; }
	friend bool operator!=(qolor::utils::generic_end_iterator const& a, custom_func_void_iterator const& b) { return b != a; }

	reference operator*() const {}
	pointer operator->() const {}
};

} // namespace internal


template<typename Func, typename Condition>
typename std::enable_if<
	qolor::utils::is_functional<Func>::value &&
	!std::is_void<typename qolor::utils::resultof<Func>::type>::value &&
	qolor::utils::is_functional_predicate<Condition, typename qolor::utils::resultof<Func>::type>::value,
	internal::iterable<internal::custom_func_iterator<Func, Condition>>
>::type from(Func&& f, Condition&& c)
{
	typedef internal::custom_func_iterator<Func,Condition> iter_t;

	iter_t b(std::forward<Func>(f), std::forward<Condition>(c), true);
	iter_t e(std::forward<Func>(f), std::forward<Condition>(c), false);

	return internal::iterable<iter_t>(std::move(b), std::move(e));
}


template<typename Func, typename Condition>
typename std::enable_if<
	qolor::utils::is_functional<Func>::value &&
	std::is_void<typename qolor::utils::resultof<Func>::type>::value &&
	qolor::utils::is_functional_predicate<Condition>::value,
	internal::iterable<internal::custom_func_iterator<Func, Condition>>
>::type from(Func&& f, Condition&& c)
{
	typedef internal::custom_func_void_iterator<Func,Condition> iter_t;

	iter_t b(std::forward<Func>(f), std::forward<Condition>(c), true);
	iter_t e(std::forward<Func>(f), std::forward<Condition>(c), false);

	return internal::iterable<iter_t>(std::move(b), std::move(e));
}


} // namespace qolor

#endif // QOLOR_FUNCTION_DRIVER_HPP__
