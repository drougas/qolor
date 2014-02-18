#ifndef QOLOR_FUNCTIONAL_TRAITS_HPP__
#define QOLOR_FUNCTIONAL_TRAITS_HPP__

#include <functional>
#include <type_traits>
#include <tuple>

namespace qolor
{

namespace utils
{

// http://en.cppreference.com/w/cpp/types/is_function
// http://stackoverflow.com/questions/8053694/c-stdfunction-cannot-find-correct-overload
// http://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda
// http://stackoverflow.com/questions/2562320/specializing-a-template-on-a-lambda-in-c0x
// http://stackoverflow.com/questions/9530928/checking-a-member-exists-possibly-in-a-base-class-c11-version
// http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence

template<typename Func>
struct functional_traits
{
private:
	template<typename C>
	struct impl {
		static constexpr size_t arity = 0;
		static constexpr bool is_functional = false;
		typedef C result_type;
		typedef void args;
		typedef C func_type;
	};

	template <typename R, typename... Args>
	struct impl<R(Args...)> {
		static constexpr size_t arity = sizeof...(Args); // arity is the number of arguments.
		static constexpr bool is_functional = true;
		typedef R result_type;
		typedef typename std::conditional<(arity > 0), std::tuple<Args...>, void>::type args;
		typedef std::function<R(Args...)> func_type;
	};

	// we specialize for pointers to standalone function and static member function
	template <typename R, typename... Args>
	struct impl<R(*)(Args...)> : public impl<R(Args...)> {};

	// we specialize for pointers to member operator(), lambdas and whater is assignable to std::function
	template <typename C, typename R, typename... Args>
	struct impl<R(C::*)(Args...) const> : public impl<R(Args...)> {};

	template <typename C, typename R, typename... Args>
	struct impl<R(C::*)(Args...)> : public impl<R(Args...)> {};

	// Not needed - we specialize for pointers to std::function
	//template <typename R, typename... Args>
	//struct impl<std::function<R(Args...)>> : public impl<R(Args...)> {};

	// For generic types, directly use the result of the signature of its 'operator()'
	template <typename C> static impl<decltype(&C::operator())> deduce( decltype(&C::operator()) );
	//template <typename C> static impl<decltype(&C::operator())> deduce( typename std::add_pointer<C>::type );
	template <typename C> static impl<C> deduce( ... );
	//template <typename C> static impl<std::function<C>> deduce( std::function<C> );

	typedef typename std::remove_reference<Func>::type func_t;
	typedef decltype(deduce<func_t>(nullptr)) signature;

public:
	static constexpr size_t arity = signature::arity;
	typedef typename signature::result_type result_type;
	typedef typename signature::args args;
	typedef typename signature::func_type func_type;
	static constexpr bool is_functional = signature::is_functional;
	static constexpr bool is_predicate = std::is_arithmetic<result_type>() || std::is_assignable<bool&, result_type>();

	// the i-th argument is equivalent to the i-th tuple element of a tuple composed of those arguments.
	template<size_t i>
	struct arg {
		static_assert(is_functional, "Func is not a functional");
		static_assert(i < arity, "The functional does not have that many arguments");
		typedef typename std::conditional<is_functional, typename std::tuple_element<i, args>::type, void>::type type;
	};

	typedef bool type;
	static constexpr bool value = is_functional;
	constexpr operator bool() const { return value; }
};

template<typename Func, size_t i>
struct functional_argument
{
private:
	typedef functional_traits<Func> traits;

public:
	static_assert(traits::is_functional, "Func is not a functional");
	static_assert(i < traits::arity, "The functional does not have that many arguments");
	typedef typename std::conditional<
		traits::is_functional && (i < traits::arity),
		typename std::tuple_element<i, typename traits::args>::type,
		void>::type type;
};

template<typename ArgT, typename T> struct is_arg_compatible                           : public std::is_assignable<ArgT&,T> {};
template<typename ArgT, typename T> struct is_arg_compatible<ArgT&,       T          > : public std::false_type {};
template<typename ArgT, typename T> struct is_arg_compatible<ArgT const&, T          > : public std::is_assignable<ArgT&,T> {};
template<typename ArgT>             struct is_arg_compatible<ArgT&,       ArgT&      > : public std::true_type {};
template<typename ArgT>             struct is_arg_compatible<ArgT const&, ArgT const&> : public std::true_type {};

template<typename R, typename T> struct is_ret_compatible
: public std::integral_constant<bool, std::is_void<R>() || is_arg_compatible<R,T>()> {};

template <typename T>
struct is_functional : public std::integral_constant<bool, functional_traits<T>::is_functional> {};

class functional_args_assignment
{
protected:
	template<size_t i, typename RequiredTuple, typename InputTuple>
	struct eq_args {
		typedef typename std::tuple_element<i - 1, RequiredTuple>::type rtype;
		typedef typename std::tuple_element<i - 1, InputTuple>::type itype;
		typedef bool type;
		static constexpr bool value = eq_args<i-1, RequiredTuple, InputTuple>() && is_arg_compatible<rtype,itype>();
		constexpr operator bool() const { return value; }
	};

	template<typename RequiredTuple, typename InputTuple>
	struct eq_args<1, RequiredTuple, InputTuple> {
		typedef typename std::tuple_element<0, RequiredTuple>::type rtype;
		typedef typename std::tuple_element<0, InputTuple>::type itype;
		typedef bool type;
		static constexpr bool value = is_arg_compatible<rtype,itype>();
		constexpr operator bool() const { return value; }
	};

	template<size_t i, typename T> struct eq_args<i, void, T> : public std::false_type {};
	template<size_t i, typename T> struct eq_args<i, T, void> : public std::false_type {};
	template<size_t i> struct eq_args<i, void, void> : public std::true_type {};

	template<typename T> struct eq_args<1, void, T> : public std::false_type {};
	template<typename T> struct eq_args<1, T, void> : public std::false_type {};

	functional_args_assignment() = default;
	functional_args_assignment(functional_args_assignment const&) = default;
	functional_args_assignment(functional_args_assignment&&) = default;
};

template <typename RequiredFunc, typename InputFunc>
struct functional_match : private functional_args_assignment
{
private:
	typedef functional_traits<RequiredFunc> ftr;
	typedef functional_traits<InputFunc> fti;

	static_assert(ftr::is_functional, "RequiredFunc is not functional");

public:
	typedef bool type;

	static constexpr bool value = (ftr::arity == fti::arity) &&
		is_ret_compatible<typename ftr::result_type, typename fti::result_type>() &&
		eq_args<ftr::arity, typename ftr::args, typename fti::args>();

	constexpr operator bool() const { return value; }
};

//template <typename InputFunc, typename RequiredArgsTuple>
//struct is_predicate_targs : private functional_args_assignment
//{
	//typedef functional_traits<InputFunc> f_traits;
	//typedef typename f_traits::func_type func_type;
	//typedef bool type;

	//static constexpr bool value = f_traits::is_predicate &&
		//(f_traits::arity == sizeof...(RequiredArgs)) &&
		//eq_args<f_traits::arity, typename f_traits::args, RequiredArgsTuple>();

	//constexpr operator bool() const { return value; }
//};


template <typename InputFunc, typename... RequiredArgs>
struct is_predicate : private functional_args_assignment
{
	typedef functional_traits<InputFunc> f_traits;
	typedef typename f_traits::func_type func_type;
	typedef bool type;

	static constexpr bool value = f_traits::is_predicate &&
		(f_traits::arity == sizeof...(RequiredArgs)) &&
		eq_args<f_traits::arity, typename f_traits::args, std::tuple<RequiredArgs...>>();

	constexpr operator bool() const { return value; }
};


template <typename InputFunc>
struct is_predicate<InputFunc, void>
{
	typedef functional_traits<InputFunc> f_traits;
	typedef typename f_traits::func_type func_type;
	typedef bool type;

	static constexpr bool value = f_traits::is_predicate && (f_traits::arity == 0);
	constexpr operator bool() const { return value; }
};


template <typename InputFunc>
struct is_predicate<InputFunc> : public is_predicate<InputFunc, void> {};


template <typename InputFunc, typename... RequiredArgs>
struct is_functional_predicate
{
	typedef is_predicate<InputFunc, RequiredArgs...> is_predicate_type;
	typedef typename is_predicate_type::f_traits f_traits;
	typedef typename f_traits::func_type func_type;
	typedef bool type;
	static constexpr bool value = f_traits::is_functional && is_predicate_type();
	constexpr operator bool() const { return value; }
};

template<typename Func>
struct resultof
{
	typedef typename functional_traits<Func>::result_type type;
};


template<typename Func>
struct num_args : public std::integral_constant<size_t, functional_traits<Func>::arity> {};

template<typename Func, size_t NArgs>
struct count_args : std::enable_if<functional_traits<Func>::arity == NArgs, std::integral_constant<size_t, NArgs>> {};

} // utils

} // namespace qolor

#endif // QOLOR_FUNCTIONAL_TRAITS_HPP__
