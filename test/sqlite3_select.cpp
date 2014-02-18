#include <string>
#include <iostream>
#include <qolor/sqlite3_driver.h>

using namespace std;
using namespace qolor::internal;

int main()
{
	try {
		sqlite3pp::database db("test.db", false, true, false);
		sqlite3pp::query qry(db, "SELECT id, name, phone FROM contacts");

		for (int i = 0; i < qry.column_count(); ++i) {
			cout << qry.column_name(i) << "\t";
		}
		cout << endl;

		qry.reset();

		for (sqlite3pp::query::iterator i = qry.begin(); i != qry.end(); ++i) {
			int id;
			char const* name, *phone;
			std::tie(id, name, phone) = (*i).get_columns<int, char const*, char const*>({{0, 1, 2}});
			cout << id << "\t" << name << "\t" << phone << endl;
		}
		cout << endl;

		qry.reset();

		for (sqlite3pp::query::iterator i = qry.begin(); i != qry.end(); ++i) {
			int id(-1);
			std::string name, phone;
			(*i).getter() >> std::ignore >> name >> phone;
			cout << id << "\t" << name << "\t" << phone << endl;
		}
	}
	catch (exception& ex) {
		cout << ex.what() << endl;
	}

	return 0;
}
