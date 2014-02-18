// sqlite3pp.cpp
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

#include "qolor/sqlite3_driver.h"
#include <memory>

using namespace qolor::internal::sqlite3pp;

sqlite3_error::sqlite3_error(database const& db)
	: std::runtime_error(get_msg(db.get_ptr())) {}

//////////////////////////////////////////////////////////////////////////////

static int busy_handler_impl(void* p, int cnt)
{
	database::busy_handler* h(static_cast<database::busy_handler*>(p));
	return (*h)(cnt);
}

static int commit_hook_impl(void* p)
{
	database::commit_handler* h(static_cast<database::commit_handler*>(p));
	return (*h)();
}

static void rollback_hook_impl(void* p)
{
	database::rollback_handler* h(static_cast<database::rollback_handler*>(p));
	(*h)();
}

static void update_hook_impl(void* p, int opcode, char const* dbname, char const* tablename, long long int rowid)
{
	database::update_handler* h(static_cast<database::update_handler*>(p));
	(*h)(opcode, dbname, tablename, rowid);
}

static int authorizer_impl(void* p, int evcode, char const* p1, char const* p2, char const* dbname, char const* tvname)
{
	database::authorize_handler* h(static_cast<database::authorize_handler*>(p));
	return (*h)(evcode, p1, p2, dbname, tvname);
}

//////////////////////////////////////////////////////////////////////////////

void database::set_busy_handler(busy_handler const& h)
{
	bh_ = h;
	sqlite3_busy_handler(db_.get(), bh_ ? busy_handler_impl : 0, &bh_);
}

void database::set_commit_handler(commit_handler const& h)
{
	ch_ = h;
	sqlite3_commit_hook(db_.get(), ch_ ? commit_hook_impl : 0, &ch_);
}

void database::set_rollback_handler(rollback_handler const& h)
{
	rh_ = h;
	sqlite3_rollback_hook(db_.get(), rh_ ? rollback_hook_impl : 0, &rh_);
}

void database::set_update_handler(update_handler const& h)
{
	uh_ = h;
	sqlite3_update_hook(db_.get(), uh_ ? update_hook_impl : 0, &uh_);
}

void database::set_authorize_handler(authorize_handler const& h)
{
	ah_ = h;
	sqlite3_set_authorizer(db_.get(), ah_ ? authorizer_impl : 0, &ah_);
}
