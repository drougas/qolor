#ifndef QOLOR_DELIMITED_TEXT_DRIVER_H__
#define QOLOR_DELIMITED_TEXT_DRIVER_H__

// Based on code from:
// http://cm.bell-labs.com/cm/cs/tpop/csvgetlinec++.c

#include "basic_iterable.h"
#include <istream>
#include <string>
#include <vector>
#include <tuple>

namespace qolor
{

namespace internal
{

namespace csv
{

typedef decltype(std::ignore) null_type;

template<typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
struct getter
{
	typedef std::basic_string<CharT, Traits, Alloc> string;
	typedef std::basic_stringstream<CharT, Traits, Alloc> sstream;
	typedef std::basic_istream<CharT> stream;

	static void get(stream& s,    CharT& d)      { s >> d; }
	static void get(stream& s,   int8_t& d)      { s >> d; }
	static void get(stream& s,  uint8_t& d)      { s >> d; }
	static void get(stream& s,  int16_t& d)      { s >> d; }
	static void get(stream& s, uint16_t& d)      { s >> d; }
	static void get(stream& s,  int32_t& d)      { s >> d; }
	static void get(stream& s, uint32_t& d)      { s >> d; }
	static void get(stream& s,  int64_t& d)      { s >> d; }
	static void get(stream& s, uint64_t& d)      { s >> d; }
	static void get(stream& s,   double& d)      { s >> d; }
	static void get(stream& s,    float& d)      { s >> d; }
	static void get(stream& s,   string& d)      { s >> d; }
	static void get(stream&  , null_type const&) {}

	sstream s_;

	void get(string const& s,    CharT& d)      { d = s[0]; }
	void get(string const& s,   int8_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s,  uint8_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s,  int16_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s, uint16_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s,  int32_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s, uint32_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s,  int64_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s, uint64_t& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s,   double& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s,    float& d)      { s_.str() = s; s_ >> d; }
	void get(string const& s, CharT const*& d)  { d = s.c_str(); }
	void get(string const& s, string& d)        { d = s; }
	void get(string const&  , null_type const&) {}
};

// read and parse comma-separated values
// sample input: "LU",86.25,"11/4/1998","2:19PM",+4.0625
template<typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
class delimited_text_iterator
{
public:
	typedef CharT char_type;
	typedef typename std::basic_string<CharT, Traits, Alloc> string_type;
	typedef typename std::basic_istream<CharT, Traits> istream_type;
	typedef getter<CharT, Traits, Alloc> getter_type;
	//typedef typename std::basic_ostream<CharT> ostream_type;

	typedef std::ptrdiff_t difference_type;
	typedef typename std::vector<string_type> value_type;
	typedef value_type const& reference;
	typedef const value_type* pointer;
	typedef std::input_iterator_tag iterator_category;

	typedef typename value_type::size_type nfields;

private:
	istream_type& is_;      // input stream
	string_type cur_line_;  // input line
	value_type fields_;     // field strings
	string_type field_sep_; // separator characters

	typedef typename std::basic_string<CharT>::size_type strsize_t;

	// unquoted field; return index of next separator
	static inline strsize_t advplain(string_type const& line, string_type const& sep, strsize_t const& i, string_type& fld) {
		strsize_t j = line.find_first_of(sep, i);
		if (j > line.length()) j = line.length();
		fld = line.substr(i, j - i);
		return j;
	}

	// Quoted field; return index of next separator
	static inline strsize_t advquoted(string_type const& line, string_type const& sep, strsize_t const& i, string_type& fld) {
		fld.clear();
		strsize_t retIndex = line.length();
		for (strsize_t j = i; j < retIndex; j++) {
			if (line[j] != '"' || j >= retIndex - 1 || line[j + 1] == '"') {
				fld += line[j];
				continue;
			}
			retIndex = line.find_first_of(sep, j);
			if (retIndex > line.length()) retIndex = line.length();
			for (; j < retIndex; ++j)
				fld += line[j];
		}
		return retIndex;
	}

	// Split line into fields
	static inline value_type split(string_type const& line, string_type const& sep) {
		value_type fields;
		std::basic_string<CharT> tmp;
		for (strsize_t i = 0, len = line.length(); i < len; ++i) {
			fields.push_back(tmp);
			if (line[i] == '"')
				i = advquoted(line, sep, ++i, fields.back());
			else
				i = advplain(line, sep, i, fields.back());
		}
		return std::move(fields);
	}

public:
	delimited_text_iterator() = delete;
	delimited_text_iterator(delimited_text_iterator&&) = default;
	delimited_text_iterator(delimited_text_iterator const&) = default;

	delimited_text_iterator(istream_type& is, const string_type& field_sep)
		: is_(is), field_sep_(field_sep) {}

	delimited_text_iterator(istream_type& is) : is_(is), field_sep_(1, ',') {}

	// Get one line, grow as needed
	delimited_text_iterator & operator++() {
		fields_.clear();
		cur_line_.clear();

		while (is_.good() && cur_line_.empty()) {
			std::getline(is_, cur_line_);
			if ((!cur_line_.empty()) && (cur_line_.back() == '\r'))
				cur_line_.pop_back();
		}

		std::basic_string<CharT> tmp;
		for (strsize_t i = 0, len = cur_line_.length(); i < len; ++i) {
			fields_.push_back(tmp);
			if (cur_line_[i] == '"')
				i = advquoted(cur_line_, field_sep_, ++i, fields_.back());
			else
				i = advplain(cur_line_, field_sep_, i, fields_.back());
		}
	
		return *this;
	}

	reference operator*() const { return fields_; }
	pointer operator->() const { return &fields_; }

	bool operator==(const delimited_text_iterator& o) const { return cur_line_ == o.cur_line_; }
	bool operator!=(const delimited_text_iterator& o) const { return cur_line_ != o.cur_line_; }

	const string_type & cur_line() const { return cur_line_; }
	const nfields & num_fields() const { return fields_.size(); }

	// return i-th field
	const string_type& operator[](const nfields& i) const {
		return i < fields_.size() ? fields_[i] : std::basic_string<CharT>();
	}

};

template<typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
class getstream
{
public:
	typedef delimited_text_iterator<CharT, Traits, Alloc> iterator_type;
	typedef typename iterator_type::stream_type stream_type;
	typedef typename iterator_type::string_type string_type;
	typedef typename iterator_type::getter_type getter_type;
	typedef typename iterator_type::nfields nfields;

	getstream() = delete;
	getstream(getstream const&) = default;
	getstream(getstream&&) = default;
	
	explicit getstream(iterator_type& iter, nfields const& start_idx = 0)
		: iter_(iter), start_idx_(start_idx), idx_(start_idx), num_(iter.num_fields()) {}

	template <class T>
	getstream& operator >> (T& value) {
		if (idx_ < num_) {
			getter_.get(iter_[idx_], value);
			if ((++idx_) == num_) {
				++iter_;
				idx_ = start_idx_;
				num_ = iter_.num_fields();
			}
		}
		return *this;
	}

private:
	iterator_type& iter_;
	getter_type getter_;
	nfields start_idx_, idx_, num_;
}; // class getstream


} // namespace csv

} // namespace internal



template<typename CharT>
internal::iterable<internal::csv::delimited_text_iterator<CharT>>
from_delimited(std::basic_istream<CharT>& is, const std::basic_string<CharT>& field_sep)
{
	typedef internal::csv::delimited_text_iterator<CharT> iter_t;
	iter_t begin(is, field_sep);
	++begin;
	return internal::iterable<iter_t>(std::move(begin), iter_t(is));
}


template<typename CharT>
internal::iterable<internal::csv::delimited_text_iterator<CharT>>
from_csv(std::basic_istream<CharT>& is)
{
	typedef internal::csv::delimited_text_iterator<CharT> iter_t;
	iter_t begin(is);
	++begin;
	return internal::iterable<iter_t>(std::move(begin), iter_t(is));
}


} // namespace qolor

#endif // QOLOR_DELIMITED_TEXT_DRIVER_H__
