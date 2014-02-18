#ifndef QOLOR_STEP_ITERATOR_H__
#define QOLOR_STEP_ITERATOR_H__

#include <functional>
#include <iterator>
#include <memory>

namespace qolor
{

namespace internal
{

template<typename T, bool AdvanceOnInitialization = false, bool UseTempBuf = true>
class input_step_iterator
: public std::iterator<std::input_iterator_tag, T, std::ptrdiff_t, const T*, const T&>
{
public:
	typedef std::function<bool(T&)> step_func;
	typedef std::function<void(T&)> finish_func;

	input_step_iterator() = default;
	input_step_iterator(input_step_iterator const&) = default;
	input_step_iterator(input_step_iterator&&) = default;

	template <typename StepFunc>
	explicit input_step_iterator(StepFunc&& step) : step_(std::forward<StepFunc>(step)) {}

	template <typename StepFunc, typename FinishFunc>
	explicit input_step_iterator(StepFunc&& step, FinishFunc&& finish)
		: step_(std::forward<StepFunc>(step)), finish_(std::forward<FinishFunc>(finish)) {}

	template <typename StepFunc, typename FinishFunc, typename... Args>
	explicit input_step_iterator(StepFunc&& step, FinishFunc&& finish, Args&&... args)
		: step_(std::forward<StepFunc>(step)), finish_(std::forward<FinishFunc>(finish)), buf_(std::forward<Args>(args)...) {}

	input_step_iterator & operator++() {
		if (step_ && !step_(buf_)) {
			if (finish_) finish_(buf_);
			step_ = nullptr;
			finish_ = nullptr;
		}
		return *this;
	}

	input_step_iterator operator++(int) {
		input_step_iterator qi(*this);
		++(*this);
		return std::move(qi);
	}

	bool operator==(input_step_iterator const& o) const { return !step_ == !o.step_; }
	bool operator!=(input_step_iterator const& o) const { return !step_ != !o.step_; }
	const T& operator*() const { return buf_; }
	const T* operator->() const { return &buf_; }

private:
	step_func step_;
	finish_func finish_;
	T buf_;
};


template<typename T>
class input_step_iterator<T, false, false>
: public std::iterator<std::input_iterator_tag, T, std::ptrdiff_t, const T*, const T&>
{
public:
	typedef std::function<bool()> step_func;
	typedef std::function<T()> get_cur_func;
	typedef std::function<void()> finish_func;

	input_step_iterator() = default;
	input_step_iterator(input_step_iterator const&) = default;
	input_step_iterator(input_step_iterator&&) = default;

	template <typename StepFunc, typename GetCurFunc>
	explicit input_step_iterator(StepFunc&& step, GetCurFunc&& get_cur)
		: step_(std::forward<StepFunc>(step)), get_cur_(std::forward<GetCurFunc>(get_cur)) {}

	template <typename StepFunc, typename GetCurFunc, typename FinishFunc>
	explicit input_step_iterator(StepFunc&& step, GetCurFunc&& get_cur, FinishFunc&& finish)
		: step_(std::forward<StepFunc>(step)), get_cur_(std::forward<GetCurFunc>(get_cur)), finish_(std::forward<FinishFunc>(finish)) {}

	input_step_iterator & operator++() {
		if (step_ && !step_()) {
			if (finish_) finish_();
			step_ = nullptr;
			get_cur_ = nullptr;
			finish_ = nullptr;
		}
		return *this;
	}

	input_step_iterator operator++(int) {
		input_step_iterator qi(*this);
		++(*this);
		return std::move(qi);
	}

	bool operator==(input_step_iterator const& o) const { return !step_ == !o.step_; }
	bool operator!=(input_step_iterator const& o) const { return !step_ != !o.step_; }
	T operator*() const { return std::move(get_cur_()); }
	std::unique_ptr<T> operator->() const { return std::unique_ptr<T>(new T(std::move(get_cur_()))); }

private:
	step_func step_;
	get_cur_func get_cur_;
	finish_func finish_;
};

template<typename T>
class input_step_iterator<T, true, true> : public input_step_iterator<T, false, true>
{
private:
	typedef input_step_iterator<T, false, true> base_t;

public:
	input_step_iterator() = default;
	input_step_iterator(input_step_iterator const&) = default;
	input_step_iterator(input_step_iterator&&) = default;

	template <typename StepFunc>
	explicit input_step_iterator(StepFunc&& step)
		: base_t(std::forward<StepFunc>(step)) { ++(*this); }

	template <typename StepFunc, typename FinishFunc>
	explicit input_step_iterator(StepFunc&& step, FinishFunc&& finish)
		: base_t(std::forward<StepFunc>(step), std::forward<FinishFunc>(finish)) { ++(*this); }

	template <typename StepFunc, typename FinishFunc, typename... Args>
	explicit input_step_iterator(StepFunc&& step, FinishFunc&& finish, Args&&... args)
		: base_t(std::forward<StepFunc>(step), std::forward<FinishFunc>(finish), std::forward<Args>(args)...) { ++(*this); }
};


template<typename T>
class input_step_iterator<T, true, false> : public input_step_iterator<T, false, false>
{
private:
	typedef input_step_iterator<T, false, false> base_t;

public:
	input_step_iterator() = default;
	input_step_iterator(input_step_iterator const&) = default;
	input_step_iterator(input_step_iterator&&) = default;

	template <typename StepFunc, typename GetCurFunc>
	explicit input_step_iterator(StepFunc&& step, GetCurFunc&& get_cur)
		: base_t(std::forward<StepFunc>(step), std::forward<GetCurFunc>(get_cur)) { ++(*this); }

	template <typename StepFunc, typename GetCurFunc, typename FinishFunc>
	explicit input_step_iterator(StepFunc&& step, GetCurFunc&& get_cur, FinishFunc&& finish)
		: base_t(std::forward<StepFunc>(step), std::forward<GetCurFunc>(get_cur), std::forward<FinishFunc>(finish)) { ++(*this); }
};

} // namespace internal

} // namespace qolor


#endif // QOLOR_STEP_ITERATOR_H__
