#include <iostream>
#include <qolor/sqlite3_driver.h>
#include "testfn.h"

using namespace std;
using namespace qolor::internal;

int main()
{
	try {
		sqlite3pp::database db("test.db");

		{
			//db.execute("CREATE TABLE IF NOT EXISTS contacts ( name varchar(65) primary key, phone varchar(65))");
			db.execute("CREATE TABLE IF NOT EXISTS contacts ( id int primary key, name varchar(65), phone varchar(65))");
			db.execute("INSERT INTO contacts (name, phone) VALUES ('AAAA', '1234')");
		}

		{
			sqlite3pp::transaction xct(db);

			sqlite3pp::command cmd(db, "INSERT INTO contacts (name, phone) VALUES (?, ?)");

			cmd.bind(2, "BBBB");
			cmd.bind(3, "2345");
			cmd.execute();

			cmd.reset();

			cmd.binder() << "CCCC" << "3456";

			cmd.execute();

			xct.commit();
		}

		{
			sqlite3pp::transaction xct(db, true);

			sqlite3pp::command cmd(db, "INSERT INTO contacts (name, phone) VALUES (:name, :name)");

			cmd.bind(":name", "DDDD");
			cmd.bind(":phone", "123-456");

			cmd.execute();
		}
	}
	catch (exception& ex) {
		cout << ex.what() << endl;
	}

}
