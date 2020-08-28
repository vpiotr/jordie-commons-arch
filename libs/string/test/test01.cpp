/*
 * test01.cpp
 *
 *  Created on: Jul 28, 2018
 *      Author: piotr
 */

#include <iostream>
#include <array>
#include "cpp_string_io.h"

using namespace std;
using namespace csi;

// class derived from printable for output to string & stream
class foo: public csi::printable {
	int a;
	int b;
public:
	foo(int pa, int pb): a(pa), b(pb) {}
	virtual std::string as_string() const {
		return "foo {" + csi::as_string(a) + ", " + csi::as_string(b) + "}";
	}

};

// class using ostream for output to string & stream
class boo {
	int x;
	int y;
public:
	boo(int pa, int pb): x(pa), y(pb) {}

	friend std::ostream& operator<<(std::ostream &out, const boo &b)
	{
		out << "boo {" << b.x << ", " << b.y << "}";
		return out;
	}

};

int main() {
	int a = 5;
	std::string s1 = as_string(a);
	cout << "s1: " << s1 << endl;

	float b = 3.1;
	std::string s2 = as_string(b);
	cout << "s2: " << s2 << endl;

	foo c(5,3);
	cout << "foo-as-string-global: " << as_string(c) << endl;

	cout << "foo-as-string-method: " << c.as_string() << endl;

	cout << "foo-to-stream: " << c << endl;

    std::string s3 = as_string(c);

	std::string s4 = as_string(s3);
	cout << "s4: " << s4 << endl;

	const char *d = "test char*";
	std::string s5 = as_string(d);
	cout << "s5: " << s5 << endl;

	int e[] = {3, 4, 5};
	std::string s6 = as_string(e);
	cout << "s6: " << s6 << endl;

	std::string s7 = as_string(begin(e), end(e));
	cout << "s7: " << s7 << endl;

	std::array<int, 3> f = {1, 2, 3};
	std::string s8 = as_string(begin(f), end(f));
	cout << "s8: " << s8 << endl;

	boo g(7, 8);
	std::string s9 = as_string(g);
	cout << "s9: " << s9 << endl;

}

