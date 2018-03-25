// $Id: cix.cpp,v 1.4 2016-05-09 16:01:56-07 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 3/15/2018
*/
#include <iostream>
#include <sstream>
#include <fstream>

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log(cout);
struct cix_exit : public exception
{
};

unordered_map<string, cix_command> command_map{
    {"exit", cix_command::EXIT},
    {"help", cix_command::HELP},
    {"ls", cix_command::LS},
    {"put", cix_command::PUT},
    {"get", cix_command::GET},
    {"rm", cix_command::RM},
};

void cix_help()
{
   static const vector<string> help = {
       "exit         - Exit the program.  Equivalent to EOF.",
       "get filename - Copy remote file to local host.",
       "help         - Print help summary.",
       "ls           - List names of files on remote server.",
       "put filename - Copy local file to remote host.",
       "rm filename  - Remove file from remote server.",
   };
   for (const auto &line : help)
      cout << line << endl;
}

void cix_ls(client_socket &server)
{
   cix_header header;
   header.command = cix_command::LS;
   log << "sending header " << header << endl;
   send_packet(server, &header, sizeof header);
   recv_packet(server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::LSOUT)
   {
      log << "sent LS, server did not return LSOUT" << endl;
      log << "server returned " << header << endl;
   }
   else
   {
      char buffer[header.nbytes + 1];
      recv_packet(server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer;
   }
}
// write my own get put and rm functions
void cix_get(client_socket &server, string &name_of_file)
{
   cix_header header;
   header.command = cix_command::GET;
   // copy the data to the header
   strcpy(header.filename, name_of_file.c_str());

   log << "sending header" << header << endl;
   send_packet(server, &header, sizeof header);
   recv_packet(server, &header, sizeof header);
   log << "revieved packet" << endl;
   if (header.command != cix_command::FILE)
   {
      log << "sent GET, server did not return FILE" << endl;
      log << "server returned " << header << endl;
   }
   else
   {
      char buffer[header.nbytes + 1];
      recv_packet(server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      ofstream out(name_of_file);
      out << string(buffer);
      out.close();
   }
}

void cix_put(client_socket &server, string &name_of_file)
{
   cix_header header;
   header.command = cix_command::PUT;
   // copy the data to the header
   strcpy(header.filename, name_of_file.c_str());

   string content = read_from_file(header.filename);

   header.nbytes = content.size();
   // send the content
   send_packet(server, &header, sizeof header);
   send_packet(server, content.c_str(), content.size());
   recv_packet(server, &header, sizeof header);

   if (header.command == cix_command::ACK)
   {
      log << "File successfully put on server. ACK" << endl;
   }
   if (header.command == cix_command::NAK)
   {
      log << "File not put on server. NAK" << endl;
   }
}
void cix_rm(client_socket &server, string &name_of_file)
{
   cix_header header;
   // copy the data to the header
   strcpy(header.filename, name_of_file.c_str());

   // send the remove command to the server
   header.command = cix_command::RM;

   // make it 0 because not sending anything
   header.nbytes = 0;

   // send and recv
   send_packet(server, &header, sizeof header);
   recv_packet(server, &header, sizeof header);

   // check recv packet for flag
   if (header.command == cix_command::ACK)
   {
      log << "File successfully deleted." << endl;
   }
   if (header.command == cix_command::NAK)
   {
      log << "File was not deleted." << endl;
   }
}

void usage()
{
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main(int argc, char **argv)
{
   vector<string> param;
   log.execname(basename(argv[0]));
   log << "starting" << endl;
   vector<string> args(&argv[1], &argv[argc]);
   if (args.size() > 2)
      usage();
   string host = get_cix_server_host(args, 0);
   in_port_t port = get_cix_server_port(args, 1);
   log << to_string(hostinfo()) << endl;
   try
   {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server(host, port);
      log << "connected to " << to_string(server) << endl;
      for (;;)
      {
         string line;
         getline(cin, line);
         if (cin.eof())
            throw cix_exit();

         std::istringstream ss(line);
         std::string token;
         while (std::getline(ss, token, ' '))
            param.push_back(token);

         log << "command " << line << endl;
         const auto &itor = command_map.find(param[0]);
         cix_command cmd = itor == command_map.end()
                               ? cix_command::ERROR
                               : itor->second;
         switch (cmd)
         {
         case cix_command::EXIT:
            throw cix_exit();
            break;
         case cix_command::HELP:
            cix_help();
            break;
         case cix_command::LS:
            cix_ls(server);
            break;
         // ----------------------------------------------------- //
         // added functions
         case cix_command::PUT:
            cix_put(server, param[1]);
            break;
         case cix_command::GET:
            cix_get(server, param[1]);
            break;
         case cix_command::RM:
            cix_rm(server, param[1]);
            break;
         // ----------------------------------------------------- //
         default:
            log << line << ": invalid command" << endl;
            break;
         }
         param.clear();
      }
   }
   catch (socket_error &error)
   {
      log << error.what() << endl;
   }
   catch (cix_exit &error)
   {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}
