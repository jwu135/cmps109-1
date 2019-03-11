// $Id: cix.cpp,v 1.7 2019-02-07 15:14:37-08 - - $

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"get" , cix_command::GET },
   {"ls"  , cix_command::LS  },
   {"put" , cix_command::PUT },
   {"rm"  , cix_command::RM  },
};

static const string help = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      log << "sent LS, server did not return LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_put (client_socket& server, string name) {
   cix_header header;
   header.command = cix_command::PUT;
   snprintf(header.filename, name.length()+1, name.c_str());

   ifstream is(name, ifstream::binary);
   if (!is.is_open()) {
      log << "put: " << name << ": " << strerror(errno) << endl;
   } else {
      is.seekg(0, is.end);
      int len = is.tellg();
      is.seekg(0, is.beg);
      char* buffer = new char[len];
      is.read(buffer, len);
      is.close();
      header.nbytes = len;

      log << "sending header " << header << endl;
      send_packet (server, &header, sizeof header);
      send_packet(server, buffer, len);
      recv_packet (server, &header, sizeof header);
      log << "received header " << header << endl;
   }
   

   if (header.command != cix_command::ACK) {
      log << "sent PUT, server did not return ACK" << endl;
      log << "server returned " << header << endl;
   } else {  
      log << name << " successfully sent" << endl;
   }
}

void cix_rm (client_socket& server, string name) {
   cix_header header;
   header.command = cix_command::RM;
   snprintf(header.filename, name.length()+1, name.c_str());

   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::ACK) {
      log << "sent RM, server did not return ACK" << endl;
      log << "server returned " << header << endl;
   } else {
      log << name << " successfully deleted" << endl;
   }
}

void cix_get (client_socket& server, string name) {
   cix_header header;
   header.command = cix_command::GET;

   snprintf(header.filename, name.length()+1, name.c_str());

   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::FILEOUT) {
      log << "sent GET, server did not return FILEOUT" << endl;
      log << "server returned " << header << endl;
   } else { 
      char* buffer = new char[header.nbytes + 1];
      buffer[header.nbytes] = '\0';
      if (header.nbytes != 0) {
          recv_packet (server, buffer, header.nbytes);
      }

      log << "received " << header.nbytes << " bytes" << endl;
      ofstream os (name, ofstream::binary);
      os.write(buffer, header.nbytes);
      os.close();
      log << name << " successfully received" << endl;
   }
}


void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         log << "command " << line << endl;
         //
         istringstream iss(line);
         vector<string> v;
         string word;
         while (getline(iss, word, ' ')) v.push_back(word);
         string file = v[1];
         //
         const auto& itor = command_map.find (v[0]);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::PUT:
               cix_put(server, file);
               break;
            case cix_command::RM:
               cix_rm(server, file);
               break;
            case cix_command::GET:
               cix_get(server, file);
               break;
            default:
               log << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

