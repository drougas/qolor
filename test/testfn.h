#ifndef QOLOR_UTILS_TESTFN_HPP__
#define QOLOR_UTILS_TESTFN_HPP__

#include <iostream>
#include <ostream>
#include <string>
#if __cplusplus > 199711L
#include <utility>
#endif

#ifdef MSVC_VER
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

template <typename _Elem, typename _Traits = std::char_traits<_Elem>>
struct is_console2 : std::false_type
{
	is_console2(std::basic_ostream<_Elem,_Traits> const&) {}
};

template <>
struct is_console2<char, std::char_traits<char>>
{
	const bool value;
	is_console2(std::ostream const& s) : value(s.rdbuf() == std::cout.rdbuf()) {}
	operator bool() const { return value; }
};

template <>
struct is_console2<wchar_t, std::char_traits<wchar_t>>
{
	const bool value;
	is_console2(std::wostream const& s) : value(s.rdbuf() == std::wcout.rdbuf()) {}
	operator bool() const { return value; }
};

template <typename _Elem, typename _Traits = std::char_traits<_Elem>>
bool is_console(std::basic_ostream<_Elem,_Traits> const&) { return false; }

template <>
bool is_console(std::basic_ostream<char, std::char_traits<char>> const& s)
{
	return (s.rdbuf() == std::cout.rdbuf()) && ::isatty(::fileno(stdout));
}

template <>
bool is_console(std::basic_ostream<wchar_t, std::char_traits<wchar_t>> const& s)
{
	return (s.rdbuf() == std::wcout.rdbuf()) && ::isatty(::fileno(stdout));
}

template <typename CharT>
class basic_testfn
{
private:

#ifdef MSVC_VER

	// http://www.codeproject.com/Articles/16431/Add-color-to-your-std-cout
	static void set_color(WORD const& c) {
		static HANDLE hStdout(::GetStdHandle(STD_OUTPUT_HANDLE));
		::SetConsoleTextAttribute(hStdout, c);
	}

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & red (std::basic_ostream<_Elem,_Traits>& s)
	{ set_color(FOREGROUND_RED | FOREGROUND_INTENSITY); return s; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & green (std::basic_ostream<_Elem,_Traits>& s)
	{ set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY); return s; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & blue (std::basic_ostream<_Elem,_Traits>& s)
	{ set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY); return s; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & yellow(std::basic_ostream<_Elem,_Traits>& s)
	{ set_color(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY); return s; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & white (std::basic_ostream<_Elem,_Traits>& s)
	{ set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED ); return s; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & def (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << white; }

#else // MSVC_VER

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & red (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[1;31m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & green (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[1;32m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & blue (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[1;34m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & grey (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[0;37m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & white (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[1;37m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & def (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[0m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & underline (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[4m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & blink (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[5m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & inverse (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[7m"; }

	template <typename _Elem, typename _Traits>
	static std::basic_ostream<_Elem,_Traits> & concealed (std::basic_ostream<_Elem,_Traits>& s)
	{ return s << "\033[8m"; }

#endif // MSVC_VER

	std::basic_string<CharT> text_;
	std::string file_;
	int line_;
	bool success_;

	basic_testfn();

public:
	basic_testfn(basic_testfn const& o) : text_(o.text_), file_(o.file_), line_(o.line_), success_(o.success_) {}
#if __cplusplus > 199711L
	basic_testfn(basic_testfn&&) = default;
#endif

	template<typename T>
	basic_testfn(const CharT* const& text, T const& success, const char* const& file = "", const int& line = 0)
	: text_(text? text : ""), file_(file? file : ""), line_(line), success_(success? true : false) {}

	template<typename T>
	basic_testfn(const std::basic_string<CharT>& text, T const& success, const char* const& file = "", const int& line = 0)
	: text_(text), file_(file? file : ""), line_(line), success_(success? true : false) {}

	template<typename T>
	basic_testfn(T const& success, const char* const& file = "", const int& line = 0)
	: file_(file? file : ""), line_(line), success_(success? true : false) {}

	const std::basic_string<CharT> & text() const { return text_; }
	const bool & success() const { return success_; }
	const std::string & file() const { return file_; }
	const int & line() const { return line_; }

	void set_status(const CharT* const& text, bool const& success) {
		text_ = text? text : "";
		success_ = success;
	}

	void set_status(std::basic_string<CharT> const& text, bool const& success) {
		text_ = text;
		success_ = success;
	}

	void set_location(const char* const& file, int const& line) {
		file_ = file? file : "";
		line_ = line;
	}

	basic_testfn & operator=(const basic_testfn& o) {
		text_ = o.text_;
		file_ = o.file_;
		line_ = o.line_;
		success_ = o.success_;
		return *this;
	}

	void swap(basic_testfn& o) {
		text_.swap(o.text_);
		file_.swap(o.file_);
		line_ ^= o.line_;
		o.line_ ^= line_;
		line_ ^= o.line_;
		success_ ^= o.success_;
		o.success_ ^= success_;
		success_ ^= o.success_;
	}

private:
	template <typename _Elem, typename _Traits>
	static void print(std::basic_ostream<_Elem,_Traits>& s, const CharT* const& text,
		const bool& success, const char* const& file, const int& line)
	{
		const bool is_cons = is_console(s);
		if (is_cons) {
			if (success) s << green;
			else s << red;
		}
		if (file && *file)
			s << file << '.' << line << ": ";
		s << (success? "SUCCESS" : "FAILURE");
		if (text && *text)
			s << " - " << text;
		if (is_cons) s << def;
	}

public:
	template <typename _Elem, typename _Traits>
	friend std::basic_ostream<_Elem,_Traits> & operator << (
		std::basic_ostream<_Elem,_Traits>& s, const basic_testfn & t)
	{
		print(s, t.text().c_str(), t.success(), t.file().c_str(), t.line());
		return s;
	}

	template <typename ExceptionT, typename FuncT>
	static basic_testfn<CharT> test_throw(const std::basic_string<CharT>& text, FuncT&& func, const char* const& file = "", const int& line = 0)
	{
		basic_testfn<CharT> t(text, true, file, line);
		try { func(); t.set_status(text + " failed to throw", false); }
		catch (ExceptionT const&) { /*OK*/ }
		catch (...) { t.set_status(text + " threw wrong thing", false); }
		return t;
	}

	template <typename T>
	static bool echo_if_failed(const CharT* const& text, T const& res, const char* const& file = "", const int& line = 0)
	{
		bool success = res? true : false;
		if (!success) {
			print(std::cout, text, false, file, line);
			std::cout << std::endl;
		}
		return success;
	}

	template <typename T>
	static bool echo_if_failed(const std::basic_string<CharT>& text, T const& res, const char* const& file = "", const int& line = 0)
	{
		return echo_if_failed(text.c_str(), res, file, line);
	}

};

typedef basic_testfn<char>     testfn;
typedef basic_testfn<wchar_t> wtestfn;

#define TESTFN(s)        testfn(#s,    (s), __FILE__, __LINE__)
#define TESTFN2(txt,s)   testfn((txt), (s), __FILE__, __LINE__)
#define WTESTFN(s)      wtestfn(#s,    (s), __FILE__, __LINE__)
#define WTESTFN2(txt,s) wtestfn((txt), (s), __FILE__, __LINE__)

#define TESTFN_THROW(s, exception_type) testfn::test_throw<exception_type>(#s, [&]() { (s); }, __FILE__, __LINE__)

#define TEST(s)       (std::cout  << TESTFN(s)       << std::endl);
#define TEST2(txt,s)  (std::cout  << TESTFN2(txt,s)  << std::endl);
#define WTEST(s)      (std::wcout << WTESTFN(s)      << std::endl);
#define WTEST2(txt,s) (std::wcout << WTESTFN2(txt,s) << std::endl);

#define TEST_THROW(s, exception_type)  (std::cout  << TESTFN_THROW(s, exception_type) << std::endl)
#define WTEST_THROW(s, exception_type) (std::wcout << TESTFN_THROW(s, exception_type) << std::endl)

#define ECHO_IF_FAILED(s) testfn::echo_if_failed(#s, (s), __FILE__, __LINE__)
#define ECHO_IF_FAILED2(txt,s) testfn::echo_if_failed(txt, (s), __FILE__, __LINE__)
#define WECHO_IF_FAILED(s) wtestfn::echo_if_failed(#s, (s), __FILE__, __LINE__)
#define WECHO_IF_FAILED2(txt,s) wtestfn::echo_if_failed(txt, (s), __FILE__, __LINE__)

#define DEBUG() (std::cerr << __FILE__ << "." << __LINE__ << std::endl)
#define DEBUG1(s) (std::cerr << __FILE__ << "." << __LINE__ << ": " << s << std::endl)

#endif // QOLOR_UTILS_TESTFN_HPP__
