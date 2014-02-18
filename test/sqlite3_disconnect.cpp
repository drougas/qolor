#include <iostream>
#include <string>
#include <qolor/sqlite3_driver.h>

using namespace std;
using namespace qolor::internal;

std::string get_exe_dir(const char* const& path)
{
	if (!path || !path[0])
		return "";

	std::string ret(path);
	auto pos = ret.find_last_of("/\\");
	return (pos < ret.length())? ret.substr(0, pos + 1) : "";
}

int main(int argc, const char* argv[])
{
	std::string path = get_exe_dir(argv[0]) + "test.db";
	try {
		sqlite3pp::database db(path);
		db.execute("CREATE TABLE IF NOT EXISTS contacts (id int primary key, name varchar(65) primary key, phone varchar(65))");
		{
			sqlite3pp::transaction xct(db);
			{
				sqlite3pp::command cmd(db, "INSERT INTO contacts (name, phone) VALUES ('AAAA', '1234')");

				cmd.execute();
			}
		}
		db.disconnect();

	}
	catch (exception& ex) {
		cout << ex.what() << endl;
	}

	return 0;
}
