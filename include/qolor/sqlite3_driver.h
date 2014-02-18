// sqlite3pp.h
//
// The MIT License
//
// Copyright (c) 2012 Wongoo Lee (iwongu at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef SQLITE3PP_H
#define SQLITE3PP_H

#include <array>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <qolor/step_iterator.h>


namespace qolor
{

namespace internal
{

namespace sqlite3pp
{

namespace ext
{
	class function;
	class aggregate;
}

class database;

typedef decltype(std::ignore) null_type;

class sqlite3_error : public std::runtime_error
{
private:
	static const char* get_msg(sqlite3* const& db) { return sqlite3_errmsg(db); }
	static const char* get_msg(std::shared_ptr<sqlite3> const& db) { return sqlite3_errmsg(db.get()); }
	static const char* get_msg(sqlite3_stmt* const& stmt) { return get_msg(sqlite3_db_handle(stmt)); }
	static const char* get_msg(std::shared_ptr<sqlite3_stmt> const& stmt) { return get_msg(stmt.get()); }
public:
	explicit sqlite3_error(char const* msg) : std::runtime_error(msg) {}
	explicit sqlite3_error(std::string const& msg) : std::runtime_error(msg.c_str()) {}

	explicit sqlite3_error(sqlite3* const& db) : std::runtime_error(get_msg(db)) {}
	explicit sqlite3_error(std::shared_ptr<sqlite3> const& db) : std::runtime_error(get_msg(db)) {}

	explicit sqlite3_error(sqlite3_stmt* const& stmt) : std::runtime_error(get_msg(stmt)) {}
	explicit sqlite3_error(std::shared_ptr<sqlite3_stmt> const& stmt) : std::runtime_error(get_msg(stmt)) {}

	explicit sqlite3_error(database const& db);
};


class database
{
	friend class ext::function;
	friend class ext::aggregate;

public:
	typedef std::function<int (int)> busy_handler;
	typedef std::function<int ()> commit_handler;
	typedef std::function<void ()> rollback_handler;
	typedef std::function<void (int, char const*, char const*, int64_t)> update_handler;
	typedef std::function<int (int, char const*, char const*, char const*, char const*)> authorize_handler;

	database(database const&) = delete;
	database & operator=(database const&) = delete;

	explicit database(char const* dbname = nullptr,
		const bool& enable_shared_cache = false,
		const bool& readonly = false,
		const bool& create_if_not_exists = true)
	{
		if (dbname && dbname[0])
			connect(dbname, enable_shared_cache, readonly, create_if_not_exists);
	}

	explicit database(std::string const& dbname) : database(dbname.c_str()) {}
	~database() {}

	void connect(char const* const& dbname,
		const bool& enable_shared_cache = false,
		const bool& readonly = false,
		const bool& create_if_not_exists = true,
		char const* const& vfs = nullptr)
	{
		int flg = readonly? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
		if (create_if_not_exists)
			flg |= SQLITE_OPEN_CREATE;
		if (enable_shared_cache)
			flg |= SQLITE_OPEN_SHAREDCACHE;

		connect_v2(dbname, flg, vfs);
	}

	void connect_v2(char const* const& dbname, int const& flags, char const* const& vfs = nullptr) {
		db_.reset();

		sqlite3* db(nullptr);
		if (sqlite3_open_v2(dbname, &db, flags, vfs) != SQLITE_OK)
			throw sqlite3_error(std::string("can't connect database \"") + dbname + "\"");

		db_.reset(db, deleter);
	}

	void disconnect() { db_.reset(); }
	void attach(char const* dbname, char const* name) { executef("ATTACH '%s' AS '%s'", dbname, name); }
	void detach(char const* name) { executef("DETACH '%s'", name); }

	int64_t last_insert_rowid() const { return sqlite3_last_insert_rowid(db_.get()); }
	int error_code() const { return sqlite3_errcode(db_.get()); }
	char const* error_msg() const { return sqlite3_errmsg(db_.get()); }

	void execute(char const* const& sql) {
		char* errmsg = nullptr;
		int rc = sqlite3_exec(db_.get(), sql, nullptr, nullptr, &errmsg);
		std::shared_ptr<char> merrmsg(errmsg, sqlite3_free);
		if (errmsg) throw sqlite3_error(errmsg);
		check_rc(rc);
	}

	void executef(char const* const& sql, ...) {
		va_list ap;
		va_start(ap, sql);
		std::shared_ptr<char> msql(sqlite3_vmprintf(sql, ap), sqlite3_free);
		va_end(ap);
		execute(msql.get());
	}

	void set_busy_timeout(int ms) { check_rc(sqlite3_busy_timeout(db_.get(), ms)); }

	void set_busy_handler(busy_handler const& h);
	void set_commit_handler(commit_handler const& h);
	void set_rollback_handler(rollback_handler const& h);
	void set_update_handler(update_handler const& h);
	void set_authorize_handler(authorize_handler const& h);
	std::shared_ptr<sqlite3> const& get_ptr() const { return db_; }

private:
	std::shared_ptr<sqlite3> db_;

	inline void check_rc(int const& rc) const
	{ if (rc != SQLITE_OK) throw sqlite3_error(db_); }

	static inline void deleter(sqlite3* const& db) {
		if (sqlite3_close(db) != SQLITE_OK)
			throw sqlite3_error(db);
	};

	busy_handler bh_;
	commit_handler ch_;
	rollback_handler rh_;
	update_handler uh_;
	authorize_handler ah_;
};


class binder_base
{
protected:
	binder_base() = default;
	binder_base(binder_base const&) = default;
	binder_base(binder_base&&) = default;
	binder_base & operator=(binder_base const&) = default;
	binder_base & operator=(binder_base&&) = default;

	static int bind(sqlite3_stmt* const& s, int const& i, int32_t const& v) { return sqlite3_bind_int(s,i,v); }
	static int bind(sqlite3_stmt* const& s, int const& i,  double const& v) { return sqlite3_bind_double(s,i,v); }
	static int bind(sqlite3_stmt* const& s, int const& i, int64_t const& v) { return sqlite3_bind_int64(s,i,v); }

	static int bind(sqlite3_stmt* const& s, int const& i, char const* const& v, bool const& fstatic = false) {
		return sqlite3_bind_text(s, i, v, v? strlen(v) : 0, fstatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
	}

	static int bind(sqlite3_stmt* const& s, int const& i, void const* const& v, int const& n, bool const& fstatic = false) {
		return sqlite3_bind_blob(s, i, v, n, fstatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
	}

	static int bind(sqlite3_stmt* const& s, int const& i) { return sqlite3_bind_null(s,i); }
	static int bind(sqlite3_stmt* const& s, int const& i, null_type const&) { return bind(s,i); }
};


class statement : protected binder_base
{
public:
	statement() = default;
	statement(statement const&) = delete;
	statement(statement&&) = default;

	void prepare(char const* const& sql) {
		finish();
		prepare_impl(sql);
	}

	void finish() { stmt_.reset(); tail_ = nullptr; }

	int get_bind_index(char const* const& name) const { return sqlite3_bind_parameter_index(stmt_.get(), name); }
	int get_bind_index(std::string const& name) const { return get_bind_index(name.c_str()); }

	void bind(int const& i, int32_t const& v) { check_rc(sqlite3_bind_int   (stmt_.get(), i, v)); }
	void bind(int const& i, int64_t const& v) { check_rc(sqlite3_bind_int64 (stmt_.get(), i, v)); }
	void bind(int const& i,  double const& v) { check_rc(sqlite3_bind_double(stmt_.get(), i, v)); }
	void bind(int const& i, null_type const&) { check_rc(sqlite3_bind_null  (stmt_.get(), i   )); }

	void bind(int const& i, char const* const& v, int const& n, bool const& fstatic = false) {
		auto deleter = fstatic ? SQLITE_STATIC : SQLITE_TRANSIENT;
		check_rc(sqlite3_bind_text(stmt_.get(), i, v, n, deleter));
	}

	void bind(int const& i, void const* const& v, int const& n, bool const& fstatic = false) {
		auto deleter = fstatic ? SQLITE_STATIC : SQLITE_TRANSIENT;
		check_rc(sqlite3_bind_blob(stmt_.get(), i, v, n, deleter));
	}

	void bind(int const& i, char const* const& v, bool const& fstatic = false) {
		bind(i, v, v? ((int)::strlen(v)) : 0, fstatic);
	}

	void bind(int const& i, std::string const& v) {
		bind(i, v.c_str(), v.length(), false);
	}

	template <typename T>
	void bind(char const* const& name, T const& value) {
		bind(get_bind_index(name), value);
	}

	void bind(char const* const& name, char const* const& value, bool const& fstatic) {
		bind(get_bind_index(name), value, fstatic);
	}

	void bind(char const* const& name, void const* const& value, int const& n, bool const& fstatic) {
		bind(get_bind_index(name), value, n, fstatic);
	}

	bool step() { return stepfun(stmt_); }
	void reset() { check_rc(sqlite3_reset(stmt_.get())); }

protected:
	explicit statement(const database& db, char const* const& sql = nullptr)
		: db_(db.get_ptr()), tail_(nullptr) { if (sql && sql[0]) prepare(sql); }

	~statement() {}

	inline void check_rc(int const& rc) const
	{ if (rc != SQLITE_OK) throw sqlite3_error(db_); }

	static inline bool stepfun(std::shared_ptr<sqlite3_stmt> const& stmt) {
		switch (::sqlite3_step(stmt.get())) {
			case SQLITE_ROW: return true;
			case SQLITE_OK:
			case SQLITE_DONE: return false;
			default: throw sqlite3_error(stmt);
		}
	}
	
	void prepare_impl(char const* const& sql) {
		sqlite3_stmt* stmt(nullptr);
		check_rc(sqlite3_prepare(db_.get(), sql, sql? strlen(sql) : 0, &stmt, &tail_));
		stmt_.reset(stmt, sqlite3_finalize);
	}

	std::shared_ptr<sqlite3_stmt> stmt_;
	std::shared_ptr<sqlite3> db_;
	char const* tail_;
};

class command : public statement
{
public:
	class bindstream
	{
	public:
		bindstream(command& cmd, int const& idx) : cmd_(cmd), idx_(idx) {}

		template <class T> bindstream& operator << (T const& value) {
			cmd_.bind(idx_, value);
			++idx_;
			return *this;
		}

	private:
		command& cmd_;
		int idx_;
	}; // class command::bindstream

	explicit command(const database& db, char const* sql = nullptr)
		: statement(db, sql) {}

	bindstream binder(int const& idx = 1) { return bindstream(*this, idx); }
	inline bool execute() { return step(); }

	size_t execute_all() {
		std::shared_ptr<sqlite3_stmt> old_stmt;
		size_t retval = 1;

		while (execute()) ;
		while (tail_ && tail_[0]) { // sqlite3_complete() is broken.
			old_stmt = stmt_;
			prepare_impl(tail_);

			if (sqlite3_transfer_bindings(old_stmt.get(), stmt_.get()) != SQLITE_OK)
				throw sqlite3_error(db_);

			old_stmt.reset();
			++retval;
			while (execute());
		}

		return retval;
	}
}; // class command


class getter_base
{
private:
	template<typename Cont, typename GetFun, typename LenFun>
	static void get_generic(sqlite3_stmt* const& stmt, int const& idx, Cont& d, GetFun&& getfun, LenFun&& lenfun) {
		typedef typename Cont::value_type val_t;
		d.clear();
		const val_t* s = reinterpret_cast<const val_t*>(getfun(stmt, idx));
		int len = lenfun(stmt, idx) / sizeof(val_t);
		if (len > 0) {
			d.reserve(len);
			std::copy(s, s + len, std::back_inserter(d));
		}
	}

protected:
	static void get(sqlite3_stmt* const& stmt, int const& idx, int32_t& d) { d = ::sqlite3_column_int(stmt, idx); }
	static void get(sqlite3_stmt* const& stmt, int const& idx,  double& d) { d = ::sqlite3_column_double(stmt, idx); }
	static void get(sqlite3_stmt* const& stmt, int const& idx, int64_t& d) { d = ::sqlite3_column_int64(stmt, idx); }

	static void get(sqlite3_stmt* const&, int const&, null_type const&) {}

	static void get(sqlite3_stmt* const& stmt, int const& idx, const char*& d) {
		d = reinterpret_cast<const char*>(::sqlite3_column_text(stmt, idx));
	}

	static void get(sqlite3_stmt* const& stmt, int const& idx, const char16_t*& d) {
		d = reinterpret_cast<const char16_t*>(::sqlite3_column_text16(stmt, idx));
	}

	static void get(sqlite3_stmt* const& stmt, int const& idx, std::string& d) {
		get_generic(stmt, idx, d, ::sqlite3_column_text, ::sqlite3_column_bytes);
	}

	static void get(sqlite3_stmt* const& stmt, int const& idx, std::u16string& d) {
		get_generic(stmt, idx, d, ::sqlite3_column_text16, ::sqlite3_column_bytes16);
	}

	static void get(sqlite3_stmt* const& stmt, int const& idx, std::vector<uint8_t>& d) {
		get_generic(stmt, idx, d, ::sqlite3_column_blob, ::sqlite3_column_bytes);
	}
};

template<size_t CurIndex, typename... Args>
struct query_helper : private getter_base
{
	enum { arity = sizeof...(Args) };

	static_assert(arity > 0, "At least one template parameter is needed.");
	static_assert(arity > CurIndex, "Invalid Index");

	static void get(sqlite3_stmt* const& stmt, std::array<int,arity> const& params, std::tuple<Args...>& dest) {
		query_helper<CurIndex - 1,Args...>::get(stmt, params, dest);
		getter_base::get(stmt, params[CurIndex], std::get<CurIndex>(dest));
	}

	static void get(sqlite3_stmt* const& stmt, std::initializer_list<int>::const_iterator& iter, std::tuple<Args...>& dest) {
		query_helper<CurIndex - 1,Args...>::get(stmt, iter, dest);
		getter_base::get(stmt, *(++iter), std::get<CurIndex>(dest));
	}
};


template<typename... Args>
struct query_helper<0, Args...> : private getter_base
{
	enum { arity = sizeof...(Args) };

	static_assert(arity > 0, "At least one template parameter is needed.");

	static void get(sqlite3_stmt* const& stmt, std::array<int,arity> const& params, std::tuple<Args...>& dest) {
		getter_base::get(stmt, params[0], std::get<0>(dest));
	}

	static void get(sqlite3_stmt* const& stmt, std::initializer_list<int>::const_iterator& iter, std::tuple<Args...>& dest) {
		getter_base::get(stmt, *iter, std::get<0>(dest));
	}
};

class rows;
class query;

class getstream : private getter_base
{
public:
	getstream() = delete;
	getstream(getstream const&) = default;
	getstream(getstream&&) = default;

	template <typename T>
	getstream& operator >> (T& value) {
		getter_base::get(stmt_.get(), idx_, value);
		++idx_;
		return *this;
	}

private:
	getstream(std::shared_ptr<sqlite3_stmt> const& stmt, int const& idx) : stmt_(stmt), idx_(idx) {}

	std::shared_ptr<sqlite3_stmt> stmt_;
	int idx_;

	friend class rows;
}; // class getstream


class rows : private getter_base
{
public:
	rows() = default;
	rows(rows const&) = delete;
	rows(rows&&) = default;

	int column_count() const { return sqlite3_column_count(stmt_.get()); }
	int cur_column_count() const { return sqlite3_data_count(stmt_.get()); }

	int column_type(int const& idx) const { return sqlite3_column_type(stmt_.get(), idx); }
	int column_bytes(int const& idx) const { return sqlite3_column_bytes(stmt_.get(), idx); }
	char const* column_name(int const& idx) const     { return sqlite3_column_name(stmt_.get(), idx); }
	char const* column_decltype(int const& idx) const { return sqlite3_column_decltype(stmt_.get(), idx); }

	template <typename T> T get(int const& idx) const {
		T value;
		getter_base::get(stmt_.get(), idx, value);
		return std::move(value);
	}

	template <typename T> void get(int const& idx, T& value) const {
		getter_base::get(stmt_.get(), idx, value);
	}

	template<typename... Ts>
	void get_columns(std::array<int,sizeof...(Ts)> const& indexes, std::tuple<Ts...>& values) const {
		enum { arity = sizeof...(Ts) };
		static_assert(arity > 0, "At least one template parameter is needed.");
		query_helper<arity - 1, Ts...>::get(stmt_.get(), indexes, values);
	}

	template<typename... Ts>
	void get_columns(std::initializer_list<int> const& indexes, std::tuple<Ts...>& values) const {
		enum { arity = sizeof...(Ts) };

		static_assert(arity > 0, "At least one template parameter is needed.");
		if (indexes.size() != arity)
			throw sqlite3_error("Wrong number of indexes.");

		std::initializer_list<int>::const_iterator iter(indexes.begin());
		query_helper<arity - 1, Ts...>::get(stmt_.get(), iter, values);
	}

	template<typename... Ts>
	std::tuple<Ts...> get_columns(std::array<int,sizeof...(Ts)> const& indexes) const {
		std::tuple<Ts...> ret;
		get_columns(indexes, ret);
		return std::move(ret);
	}

	template<typename... Ts>
	std::tuple<Ts...> get_columns(std::initializer_list<int> const& indexes) const {
		std::tuple<Ts...> ret;
		get_columns(indexes, ret);
		return std::move(ret);
	}

	getstream getter(int const& idx = 0) const { return getstream(stmt_, idx); }
	void release() { stmt_.reset(); }

private:
	explicit rows(std::shared_ptr<sqlite3_stmt> const& stmt) : stmt_(stmt) {}
	std::shared_ptr<sqlite3_stmt> stmt_;
	friend class query;
}; // class rows


class query : public statement, private getter_base
{
public:
	typedef input_step_iterator<rows, true, true> iterator;

	query() = delete;
	query(query const&) = delete;
	query(query&&) = default;
	explicit query(database& db, char const* sql = nullptr) : statement(db, sql) {}

	int column_count() const { return sqlite3_column_count(stmt_.get()); }
	int cur_column_count() const { return sqlite3_data_count(stmt_.get()); }

	int column_type(int const& idx) const { return sqlite3_column_type(stmt_.get(), idx); }
	int column_bytes(int const& idx) const { return sqlite3_column_bytes(stmt_.get(), idx); }
	char const* column_name(int const& idx) const     { return sqlite3_column_name(stmt_.get(), idx); }
	char const* column_decltype(int const& idx) const { return sqlite3_column_decltype(stmt_.get(), idx); }

	iterator begin() {
		static const auto getter = [](rows& r) { return stepfun(r.stmt_); };
		return iterator(getter, nullptr, rows(stmt_));
	}

	iterator end() { return iterator(nullptr, nullptr, rows(nullptr)); }

}; // class query


class transaction
{
public:
	transaction() = delete;
	transaction(transaction const&) = delete;
	transaction(transaction&&) = delete;
	transaction & operator=(transaction const&) = delete;
	transaction & operator=(transaction&&) = delete;

	explicit transaction(database const& db, bool const& fcommit = false, bool const& freserve = false)
		: db_(db.get_ptr()), fcommit_(fcommit)
	{
		execute(freserve? "BEGIN IMMEDIATE" : "BEGIN");
	}

	~transaction() { execute(fcommit_? "COMMIT" : "ROLLBACK"); }
	void commit() { execute("COMMIT"); db_.reset(); }
	void rollback() { execute("ROLLBACK"); db_.reset(); }

private:
	void execute(const char* const& sql) {
		if (db_ && (sqlite3_exec(db_.get(), sql, 0, 0, 0) != SQLITE_OK))
			throw sqlite3_error(db_);
	}

	std::shared_ptr<sqlite3> db_;
	bool fcommit_;
};

} // namespace sqlite3pp

} // namespace internal

} // namespace qolor

#endif
