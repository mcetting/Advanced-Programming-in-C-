// $Id: debug.cpp,v 1.2 2018-01-25 14:12:59-08 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 2/19/2018
*/
#include <climits>
#include <iostream>
#include <vector>

using namespace std;

#include "debug.h"
#include "util.h"

debugflags::flagset debugflags::flags {};

void debugflags::setflags (const string& initflags) {
   for (const unsigned char flag: initflags) {
      if (flag == '@') flags.set();
                  else flags.set (flag, true);
   }
}

// getflag -
//    Check to see if a certain flag is on.

bool debugflags::getflag (char flag) {
   // WARNING: Don't TRACE this function or the stack will blow up.
   return flags.test (static_cast<unsigned char> (flag));
}

void debugflags::where (char flag, const char* file, int line,
                        const char* pretty_function) {
   cout << sys_info::execname() << ": DEBUG(" << flag << ") "
        << file << "[" << line << "] " << endl
        << "   " << pretty_function << endl;
}

