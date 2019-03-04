// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cctype>
#include <regex>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

void print_map(str_str_map& m) {
   str_str_map::iterator i = m.begin();
   for (; i != m.end(); ++i) {   //cant reach
      cout << i->first << " = " << i->second << endl;
   }
}

void implement_map(str_str_map& m, string s) {
   string key, value;
   size_t t1 = s.find_first_not_of(" ");
   size_t t2 = s.find_first_of("=");
   //size_t t3 = s.find_last_not_of(" ");

   //if no = in the line
   size_t found = s.find("=");
   if (found == string::npos) {  //only key
      bool check = false;
      key = s;
      str_str_map::iterator i = m.begin();
      for (; i != m.end(); ++i) {
         if (i->first.compare(key) == 0) {
            cout << i->first << " = " << i->second << endl;
            check = true;
            break;
         }
      }
      if (not check) 
         cout << s << ": key not found" << endl;
      
   } else { //= in the line
      if (s.size() == 1) { //"="
         print_map(m);
      } else if (s.find_first_not_of(" ", t2+1) == string::npos) {
         //"key="
         bool check = false;
         key = s.substr(t1,t2);
         str_str_map::iterator i = m.begin();
         for (; i != m.end(); ++i) {
            if (i->first.compare(key) == 0) {
               check = true;
               m.erase(i);
               break;
            }
         }
         if (not check) 
            cout << s << ": key not found" << endl;

      } else if (s[0] == '=') {  //"=value"
         value = s.substr(1,s.size()-1);
         str_str_map::iterator i = m.begin();
         for (; i != m.end(); ++i) {
            if (i->second.compare(value) == 0) 
               cout << i->first << " = " << i->second << endl;
         }
      } else { //"key=value"
         key = s.substr(t1,t2);
         value = s.substr(t2+1);
         cout << key << " = " << value << endl;
         m.insert(str_str_pair(key, value));
      }
   }

}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);

   regex empty_regex {R"(^\s*?$)"};
   regex comment_regex {R"(^\s*(#.*)?$)"}; 
   smatch result;

   str_str_map m;
   if (argc < 2) {   //keyvalue
      int count(0);
      while (true) {
         if (cin.eof()) break; 
         string line;
         getline(cin, line);
         if(regex_search(line, result, empty_regex)) {
            cout << "-: " << ++count << ": " << endl;
         } else if(regex_search(line, result, comment_regex)) {
            cout << "-: " << ++count << ": " << line << endl;
         } else {
            cout << "-: " << ++count << ": " << line << endl;
            implement_map(m, line);  
         }    
      }

   }

   for (int i = 1; i < argc; ++i) { //input files
      int count(0);
      fstream files;
      files.open(argv[i]);
      if (files.is_open()) {
         while (true) {
            if (files.eof()) break; 
            string line;
            getline(files, line);
            if(regex_search(line, result, empty_regex)) { 
               cout << argv[i] << ": " << ++count << ": " << endl;
            } else if(regex_search(line, result, comment_regex)) { 
               cout << argv[i] << ": " << ++count << ": " << line 
                  << endl;
            } else {
               cout << argv[i] << ": " << ++count << ": " << line 
                  << endl;
               implement_map(m, line); 
            }
       
         }
         
      } else {
         cout << "keyvalue: " << argv[i] 
            << ": No such file or directory" << endl;
      }
   }
/*   
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      str_str_pair pair (*argp, to_string<int> (argp - argv));
      cout << "Before insert: " << pair << endl;
      test.insert (pair);
   }

   for (str_str_map::iterator itor = test.begin();
        itor != test.end(); ++itor) {
      cout << "During iteration: " << *itor << endl;
   }

   str_str_map::iterator itor = test.begin();
   test.erase (itor);

   cout << "EXIT_SUCCESS" << endl;*/
   return EXIT_SUCCESS;

   return 0;
}

