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

	void execute(char const* const& sql) { check_rc(sqlite3_exec(db_.get(), sql, 0, 0, 0)); }
	void executef(char const* const& sql, ...);

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

	static int bind(sqlite3_stmt* const& s, int const& i, char const* const& v, bool const& fstatic = true) {
		return sqlite3_bind_text(s, i, v, v? strlen(v) : 0, fstatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
	}

	static int bind(sqlite3_stmt* const& s, int const& i, void const* const& v, int const& n, bool const& fstatic = true) {
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
	statement & operator=(statement const&) = delete;

	void prepare(char const* const& stmt) {
		finish();

		std::shared_ptr<sqlite3> db(db_);
		auto deleter = [db](sqlite3_stmt* s)->void {
			if (sqlite3_finalize(s) != SQLITE_OK)
				throw sqlite3_error(db);
		};

		prepare_impl(stmt, std::move(deleter));
	}

	void finish() { stmt_.reset(); tail_ = nullptr; }

	int get_index(char const* const& name) const { return sqlite3_bind_parameter_index(stmt_.get(), name); }
	int get_index(std::string const& name) const { return sqlite3_bind_parameter_index(stmt_.get(), name.c_str()); }

	template<typename T>
	void bind(int const& idx, T const& value) { check_rc(binder_base::bind(stmt_.get(), idx, value)); }
	
	void bind(int const& idx, char const* const& value, bool const& fstatic) {
		check_rc(binder_base::bind(stmt_.get(), idx, value, fstatic));
	}

	void bind(int const& idx, void const* const& value, int n, bool const& fstatic = true) {
		check_rc(binder_base::bind(stmt_.get(), idx, value, n, fstatic));
	}

	template <typename T>
	void bind(char const* const& name, T const& value) {
		bind(get_index(name), value);
	}

	void bind(char const* const& name, char const* const& value, bool const& fstatic) {
		bind(get_index(name), value, fstatic);
	}

	void bind(char const* const& name, void const* const& value, int const& n, bool const& fstatic = true) {
		bind(get_index(name), value, n, fstatic);
	}

	void bind(int const&) {}
	void bind(char const* const&) {}

	bool step() {
		switch (sqlite3_step(stmt_.get())) {
			case SQLITE_ROW: return true;
			case SQLITE_OK:
			case SQLITE_DONE: return false;
			default: throw sqlite3_error(db_);
		}
	}
	void reset() { check_rc(sqlite3_reset(stmt_.get())); }

protected:
	explicit statement(const database& db, char const* const& stmt = nullptr)
		: db_(db.get_ptr()), tail_(nullptr) { if (stmt && stmt[0]) prepare(stmt); }

	~statement() {}

	inline void check_rc(int const& rc) const
	{ if (rc != SQLITE_OK) throw sqlite3_error(db_); }

	template <typename Deleter>
	void prepare_impl(char const* stmt, Deleter&& deleter) {
		sqlite3_stmt* stmt2(nullptr);
		check_rc(sqlite3_prepare(db_.get(), stmt, stmt? strlen(stmt) : 0, &stmt2, &tail_));
		stmt_.reset(stmt2, deleter);
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

	explicit command(const database& db, char const* stmt = nullptr)
		: statement(db, stmt) {}

	bindstream binder(int const& idx = 1) { return bindstream(*this, idx); }
	inline bool execute() { return step(); }
	bool execute_all();
}; // class command


class getter_base
{
protected:
	getter_base() = default;
	getter_base(getter_base const&) = default;
	getter_base(getter_base&&) = default;
	getter_base & operator=(getter_base const&) = default;
	getter_base & operator=(getter_base&&) = default;

	static void get(int const& idx, sqlite3_stmt* const& stmt, int32_t& d)       { d = sqlite3_column_int(stmt, idx); }
	static void get(int const& idx, sqlite3_stmt* const& stmt,  double& d)       { d = sqlite3_column_double(stmt, idx); }
	static void get(int const& idx, sqlite3_stmt* const& stmt, int64_t& d)       { d = sqlite3_column_int64(stmt, idx); }
	static void get(int const& idx, sqlite3_stmt* const& stmt, char const*& d)   { d = reinterpret_cast<char const*>(sqlite3_column_text(stmt, idx)); }
	static void get(int const& idx, sqlite3_stmt* const& stmt, std::string& d)   { d = reinterpret_cast<char const*>(sqlite3_column_text(stmt, idx)); }
	static void get(int const& idx, sqlite3_stmt* const& stmt, void const*& d)   { d = sqlite3_column_blob(stmt, idx); }
	static void get(int const&    , sqlite3_stmt* const&     , null_type const&) {}
};

template<size_t CurIndex, typename... Args>
struct query_helper : private getter_base
{
	enum { arity = sizeof...(Args) };

	static_assert(arity > 0, "At least one template parameter is needed.");
	static_assert(arity > CurIndex, "Invalid Index");

	static void get(std::array<int,arity> const& params, sqlite3_stmt* const& stmt, std::tuple<Args...>& dest) {
		query_helper<CurIndex - 1,Args...>::get(params, stmt, dest);
		getter_base::get(params[CurIndex], stmt, std::get<CurIndex>(dest));
	}

	static void get(std::initializer_list<int>::const_iterator& iter, sqlite3_stmt* const& stmt, std::tuple<Args...>& dest) {
		query_helper<CurIndex - 1,Args...>::get(iter, stmt, dest);
		getter_base::get(*(++iter), stmt, std::get<CurIndex>(dest));
	}
};


template<typename... Args>
struct query_helper<0, Args...> : private getter_base
{
	enum { arity = sizeof...(Args) };

	static_assert(arity > 0, "At least one template parameter is needed.");

	static void get(std::array<int,arity> const& params, sqlite3_stmt* const& stmt, std::tuple<Args...>& dest) {
		getter_base::get(params[0], stmt, std::get<0>(dest));
	}

	static void get(std::initializer_list<int>::const_iterator& iter, sqlite3_stmt* const& stmt, std::tuple<Args...>& dest) {
		getter_base::get(*iter, stmt, std::get<0>(dest));
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

	template <class T>
	getstream& operator >> (T& value) {
		getter_base::get(idx_, stmt_.get(), value);
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
	rows() = delete;
	rows(rows const&) = delete;
	rows(rows&&) = delete;

	int num_columns() const { return sqlite3_data_count(stmt_.get()); }
	int data_count() const { return sqlite3_data_count(stmt_.get()); }
	int column_type(int const& idx) const { return sqlite3_column_type(stmt_.get(), idx); }
	int column_bytes(int const& idx) const { return sqlite3_column_bytes(stmt_.get(), idx); }

	template <typename T> T get(int const& idx) const {
		T value;
		getter_base::get(idx, stmt_.get(), value);
		return std::move(value);
	}

	template <typename T> void get(int const& idx, T& value) const {
		getter_base::get(idx, stmt_.get(), value);
	}

	template<typename... Ts>
	void get_columns(std::array<int,sizeof...(Ts)> const& indexes, std::tuple<Ts...>& values) const {
		enum { arity = sizeof...(Ts) };
		static_assert(arity > 0, "At least one template parameter is needed.");
		query_helper<arity - 1, Ts...>::get(indexes, stmt_.get(), values);
	}

	template<typename... Ts>
	void get_columns(std::initializer_list<int> const& indexes, std::tuple<Ts...>& values) const {
		enum { arity = sizeof...(Ts) };

		static_assert(arity > 0, "At least one template parameter is needed.");
		if (indexes.size() != arity)
			throw sqlite3_error("Wrong number of indexes.");

		std::initializer_list<int>::const_iterator iter(indexes.begin());
		query_helper<arity - 1, Ts...>::get(iter, stmt_.get(), values);
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


class query : public statement
{
public:
	typedef input_step_iterator<rows, true, true> iterator;

	explicit query(database& db, char const* stmt = 0) : statement(db, stmt) {}

	int column_count() const { return sqlite3_column_count(stmt_.get()); }
	char const* column_name(int const& idx) const     { return sqlite3_column_name(stmt_.get(), idx); }
	char const* column_decltype(int const& idx) const { return sqlite3_column_decltype(stmt_.get(), idx); }

	iterator begin() { return iterator(stepfun, nullptr, stmt_); }
	iterator end() { return iterator(); }

private:

	static bool stepfun(rows& r) {
		int rc = sqlite3_step(r.stmt_.get());
		if (rc == SQLITE_ROW) return true;
		if (rc == SQLITE_DONE) { r.stmt_.reset(); return false; }
		throw sqlite3_error(r.stmt_);
	}

}; // class query


class transaction
{
public:
	transaction() = delete;
	transaction(transaction const&) = delete;
	transaction & operator=(transaction const&) = delete;

	explicit transaction(database const& db, bool const& fcommit = false, bool const& freserve = false)
		: db_(db.get_ptr()), fcommit_(fcommit)
	{
		execute(freserve? "BEGIN IMMEDIATE" : "BEGIN");
	}

	~transaction() { if (db_) execute(fcommit_? "COMMIT" : "ROLLBACK"); }
	void commit() { if (db_) { execute("COMMIT"); db_.reset(); } }
	void rollback() { if (db_) { execute("ROLLBACK"); db_.reset(); } }

private:
	void execute(const char* const& sql) {
		if (sqlite3_exec(db_.get(), sql, 0, 0, 0) != SQLITE_OK)
			throw sqlite3_error(db_);
	}

	std::shared_ptr<sqlite3> db_;
	bool fcommit_;
};

} // namespace sqlite3pp

} // namespace internal

} // namespace qolor

#endif
