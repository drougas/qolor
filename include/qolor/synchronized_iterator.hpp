#ifndef QOLOR_SYNCHRONIZED_ITERATOR_HPP__
#define QOLOR_SYNCHRONIZED_ITERATOR_HPP__

#include "utilities.h"
#include "functional_traits.hpp"
#include <iterator>
#include <type_traits>
#include <memory>
#include <mutex>

namespace qolor
{

namespace internal
{

template <typename InnerType>
class synchronized_iterator
{
private:
	typedef std::iterator_traits<InnerType> traits;
	typedef std::lock_guard<std::mutex> guard_t;

	InnerType cur_;
	std::shared_ptr<std::mutex> mutex_;
	
public:
	typedef typename traits::difference_type difference_type;
	typedef typename traits::value_type value_type;
	typedef typename traits::reference reference;
	typedef typename traits::pointer pointer;
	typedef typename traits::iterator_category iterator_category;
	typedef decltype(*cur_) deref_type;

	static constexpr bool isref = std::is_reference<deref_type>();

	synchronized_iterator() = delete;
	synchronized_iterator(synchronized_iterator const&) = default;
	synchronized_iterator(synchronized_iterator&&) = default;

	template <typename I, typename M>
	synchronized_iterator(I&& it, std::shared_ptr<std::mutex> const& m)
		: cur_(std::forward<I>(it)), mutex_(m) {}

	bool operator==(synchronized_iterator const& o) const {
		std::mutex *mutex1 = mutex_.get(), *mutex2 = o.mutex_.get();
		if (mutex1->native_handle() > mutex2->native_handle())
			std::swap(mutex1, mutex2);

		guard_t guard1(*mutex1), guard2(*mutex2);
		return cur_ == o.cur_;
	}

	bool operator!=(synchronized_iterator const& o) const {
		std::mutex *mutex1 = mutex_.get(), *mutex2 = o.mutex_.get();
		if (mutex1->native_handle() > mutex2->native_handle())
			std::swap(mutex1, mutex2);

		guard_t guard1(*mutex1), guard2(*mutex2);
		return cur_ != o.cur_;
	}

	synchronized_iterator & operator++() { guard_t g(*mutex_); ++cur_; return *this; }
	synchronized_iterator & operator--() { guard_t g(*mutex_); --cur_; return *this; }
	synchronized_iterator operator++(int) { synchronized_iterator i(*this); ++(*this); return i; }
	synchronized_iterator operator--(int) { synchronized_iterator i(*this); --(*this); return i; }
	synchronized_iterator & operator+=(difference_type const& diff) { guard_t g(*mutex_); std::advance(cur_, diff); return *this; }
	synchronized_iterator & operator-=(difference_type const& diff) { guard_t g(*mutex_); std::advance(cur_, -diff); return *this; }

	synchronized_iterator operator+(difference_type const& diff) const {
		return std::move(synchronized_iterator(*this) += d);
	}

	synchronized_iterator operator-(difference_type const& diff) const {
		return std::move(synchronized_iterator(*this) -= d);
	}

	difference_type operator-(synchronized_iterator const& other) const {
		return cur_ - other.cur_;
	}
	deref_type operator*() { guard_t g(*mutex_); return *cur_; }

	typename std::conditional<isref, std::unique_ptr<value_type>, pointer> operator->() {
		std::shared_ptr<std::mutex> m(mutex_);
		m->lock();
		auto unlocker = [m](pointer const&) { m->unlock(); };
		return std::unique_ptr<value_type>();
	}
};

} // namespace internal

} // namespace qolor

#endif // QOLOR_SYNCHRONIZED_ITERATOR_HPP__
