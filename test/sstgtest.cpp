#include <fstream.h>
#include "String.h"

main ()
{
	String stg;

	stg = "you = this and that";
	cout << "at('=') = '" << stg.at('=') << "'" << endl;
	cout << "before('= ) = '" << stg.before('=') << "'" << endl;
	cout << "through('=') = '" << stg.through('=') << "'" << endl;
	cout << "after('=') = '" << stg.after('=') << "'" << endl;

	cout << endl;
	stg = "great program - prog.uU (1/11";
	cout << stg << endl;

	static Regex ex("[([ ][0-9]+\\(/\\|[ ]+of[ ]+\\)[0-9]+\\([]) ]\\|$\\)");

	cout << "index(ex) " << stg.index(ex) << endl;

	cout << "at(ex) = '" << stg.at(ex) << "'" << endl;
	cout << "before(ex) = '" << stg.before(ex) << "'" << endl;
	cout << "through(ex) = '" << stg.through(ex) << "'" << endl;
	cout << "after(ex) = '" << stg.after(ex) << "'" << endl;

	static Regex ex1 ("\\.[eE][xX][eE]"
                        "\\|\\.[zZ][iI][pP]"
                        "\\|\\.[rR][aA][rR]"
                        "\\|\\.[rR][0-9][0-9]"
                        "\\|\\.[uU][uU][eE]?");

	cout << "index(ex) " << stg.index(ex1) << endl;

	cout << "at(ex) = '" << stg.at(ex1) << "'" << endl;
	cout << "before(ex) = '" << stg.before(ex1) << "'" << endl;
	cout << "through(ex) = '" << stg.through(ex1) << "'" << endl;
	cout << "after(ex) = '" << stg.after(ex1) << "'" << endl;

	stg = "alt.binaries.animals";
	static Regex ex2 ("^alt\\.[^.]*");

	cout << "index(ex) " << stg.index(ex2) << endl;
	cout << "at(ex) = '" << stg.at(ex2) << "'" << endl;
	cout << "before(ex) = '" << stg.before(ex2) << "'" << endl;
	cout << "through(ex) = '" << stg.through(ex2) << "'" << endl;
	cout << "after(ex) = '" << stg.after(ex2) << "'" << endl;
}

