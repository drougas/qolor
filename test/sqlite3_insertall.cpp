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
			db.execute("CREATE TABLE IF NOT EXISTS contacts ( id int primary key, name varchar(65), phone varchar(65))");
			sqlite3pp::transaction xct(db);

			{
				sqlite3pp::command cmd(db,
						"INSERT INTO contacts (name, phone) VALUES (:name, '1234');"
						"INSERT INTO contacts (name, phone) VALUES (:name, '5678');"
						"INSERT INTO contacts (name, phone) VALUES (:name, '9012');");
				{
					cmd.bind(":name", "user");
					cmd.execute_all();
				}
			}

			xct.commit();
		}
	}
	catch (exception ex) {
		TEST2(ex.what(), false);
	}

	return 0;
}
