#include <exception>
#include <iostream>
#include <qolor/sqlite3_driver.h>

using namespace std;
using namespace qolor::internal;

struct handler
{
	handler() : cnt_(0) {}

	void handle_update(int opcode, char const* dbname, char const* tablename, long long int rowid) {
		cout << "handle_update(" << opcode << ", " << dbname << ", " << tablename << ", " << rowid << ") - " << cnt_++ << endl;
	}
	int cnt_;
};


//int handle_authorize(int evcode, char const* p1, char const* p2, char const* dbname, char const* tvname) {
int handle_authorize(int evcode, char const*, char const*, char const*, char const*) {
	cout << "handle_authorize(" << evcode << ")" << endl;
	return 0;
}


struct rollback_handler
{
	void operator()() { cout << "handle_rollback" << endl; }
};


int main()
{
	try {
		sqlite3pp::database db("test.db");

		{
			db.set_commit_handler([]()->int { cout << "handle_commit\n"; return 0; });
			db.set_rollback_handler(rollback_handler());
		}

		handler h;

		{
			using namespace std::placeholders;
			db.set_update_handler(std::bind(&handler::handle_update, h, _1, _2, _3, _4));
		}

		db.set_authorize_handler(&handle_authorize);

		db.execute("INSERT INTO contacts (name, phone) VALUES ('AAAA', '1234')");

		{
			sqlite3pp::transaction xct(db);

			sqlite3pp::command cmd(db, "INSERT INTO contacts (name, phone) VALUES (?, ?)");
			cmd.bind(1, "BBBB");
			cmd.bind(2, "1234");
			cmd.execute();

			cmd.reset();

			cmd.binder() << "CCCC" << "1234";

			cmd.execute();

			xct.commit();
		}

		{
			sqlite3pp::transaction xct(db);

			sqlite3pp::command cmd(db, "INSERT INTO contacts (name, phone) VALUES (:name, :name)");

			cmd.bind(":name", "DDDD");

			cmd.execute();
		}
	}
	catch (std::exception& ex) {
		cout << ex.what() << endl;
	}

	return 0;
}
