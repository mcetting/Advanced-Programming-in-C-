// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 2/19/2018
*/
#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unistd.h>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string, string>;
using str_str_pair = str_str_map::value_type;

void scan_options(int argc, char **argv)
{
   opterr = 0;
   for (;;)
   {
      int option = getopt(argc, argv, "@:");
      if (option == EOF)
         break;
      switch (option)
      {
      case '@':
         debugflags::setflags(optarg);
         break;
      default:
         complain() << "-" << char(optopt) << ": invalid option"
                    << endl;
         break;
      }
   }
}

// trims the white space
string trim_white(string str)
{
   // trim the white space on the ends of both sides
   string temp = str;
   // check back
   int i = 0;
   for (;;)
   {
      if (temp[i] != ' ')
      {
         temp = temp.substr(i, temp.length() - i);
         break;
      }
      else
      {
         // get rid of it
         ++i;
      }
   }
   i = temp.length() - 1;
   // check front
   for (;;)
   {
      if (temp[i] != ' ')
      {
         temp = temp.substr(0, i + 1);
         break;
      }
      else
      {
         // get rid of it
         --i;
      }
   }
   return temp;
}

int main(int argc, char **argv)
{
   sys_info::execname(argv[0]);
   scan_options(argc, argv);
   str_str_map test;
   fstream f;

   // check
   bool stdin_bool = false;
   if (isatty(fileno(stdin)))
   {
      stdin_bool = true;
   }
   if (stdin_bool)
   {
      for (char **argp = &argv[optind]; argp != &argv[argc]; ++argp)
      {
         regex equals{"="};
         f.open(*argp);

         if (f.is_open())
         {
            // make it alphabetical
            // parse the line using regex search into key, =, and value
            string line;
            int line_num = 1;
            for (;;)
            {
               getline(f, line);
               if (f.eof())
               {
                  break;
               }
               else
               {
                  cout << *argp << ": " << line_num << ": " << line << endl;
                  if (line == "" || line == " ")
                  {
                     ++line_num;
                     continue;
                  }
                  if (line[0] == '#')
                  {
                     ++line_num;
                     continue;
                  }
                  smatch result;
                  if (regex_search(line, result, equals))
                  {
                     if ((result.prefix() == "" || result.prefix() == " ") &&
                         (result.suffix() == "" || result.suffix() == " "))
                     {
                        // just the equals sign
                        for (str_str_map::iterator itor = test.begin();
                             itor != test.end(); ++itor)
                        {
                           cout << itor->first << " = " << itor->second << endl;
                        }
                     }
                     else if ((result.prefix() == "" || result.prefix() == " "))
                     {
                        // only the first one is nothing
                        for (str_str_map::iterator itor = test.begin();
                             itor != test.end(); ++itor)
                        {
                           if (itor->second == result.suffix())
                           {
                              cout << itor->first << " = " << itor->second << endl;
                           }
                        }
                     }
                     else if ((result.suffix() == "" || result.suffix() == " "))
                     {
                        test.erase(test.find(result.prefix()));
                     }
                     else
                     {
                        string temp = trim_white(result.prefix());

                        str_str_pair pair(trim_white(result.prefix()), trim_white(result.suffix()));
                        test.insert(pair);

                        cout << trim_white(result.prefix()) << " " << result[0]
                             << " " << trim_white(result.suffix()) << endl;
                     }
                  }
                  else
                  {
                     if (test.find(line) == test.end())
                     {
                        cout << line << ": key not found" << endl;
                     }
                     else
                     {
                        cout << test.find(line)->first << " = " << test.find(line)->second << endl;
                     }
                  }
               }
               ++line_num;
            }
         }
         else
         {
            cout << "keyvalue: " << *argp << ": ";
            cout << "No such file or directory" << endl;
            continue;
         }
         f.close();
      }
   }
   else
   {
      // make it alphabetical
      // parse the line using regex search into key, =, and value
      regex equals{"="};
      string line;
      int line_num = 1;
      for (;;)
      {
         getline(cin, line);
         if (cin.eof())
         {
            break;
         }
         else
         {
            cout << "-: " << line_num << ": " << line << endl;
            if (line == "" || line == " ")
            {
               ++line_num;
               continue;
            }
            if (line[0] == '#')
            {
               ++line_num;
               continue;
            }
            smatch result;
            if (regex_search(line, result, equals))
            {
               if ((result.prefix() == "" || result.prefix() == " ") &&
                   (result.suffix() == "" || result.suffix() == " "))
               {
                  // just the equals sign
                  for (str_str_map::iterator itor = test.begin();
                       itor != test.end(); ++itor)
                  {
                     cout << itor->first << " = " << itor->second << endl;
                  }
               }
               else if ((result.prefix() == "" || result.prefix() == " "))
               {
                  // only the first one is nothing
                  for (str_str_map::iterator itor = test.begin();
                       itor != test.end(); ++itor)
                  {
                     if (itor->second == result.suffix())
                     {
                        cout << itor->first << " = " << itor->second << endl;
                     }
                  }
               }
               else if ((result.suffix() == "" || result.suffix() == " "))
               {
                  test.erase(test.find(result.prefix()));
               }
               else
               {
                  string temp = trim_white(result.prefix());

                  str_str_pair pair(trim_white(result.prefix()), trim_white(result.suffix()));
                  test.insert(pair);

                  cout << trim_white(result.prefix()) << " " << result[0]
                       << " " << trim_white(result.suffix()) << endl;
               }
            }
            else
            {
               if (test.find(line) == test.end())
               {
                  cout << line << ": key not found" << endl;
               }
               else
               {
                  cout << test.find(line)->first << " = " << test.find(line)->second << endl;
               }
            }
         }
         ++line_num;
      }
   }

   return EXIT_SUCCESS;
}
