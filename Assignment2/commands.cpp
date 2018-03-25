// $Id: commands.cpp,v 1.17 2018-01-25 14:02:55-08 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 2/05/2018
*/
#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   }, // done
   {"cd"    , fn_cd    }, // done
   {"echo"  , fn_echo  }, // done
   {"exit"  , fn_exit  }, // done
   {"ls"    , fn_ls    }, // done
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  }, // done
   {"mkdir" , fn_mkdir }, // done
   {"prompt", fn_prompt}, // done
   {"pwd"   , fn_pwd   }, // done
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // simply print the contents file
   // search for the file
   // if its not found throw an error
   // if its there print its contents to the screen
   for(unsigned int i = 1; i < words.size(); ++ i){
        inode_ptr temp = state.get_cwd()->con_as_dir()->string_to_inode(words[i]
         , state.get_cwd());

        if(temp == nullptr){
            throw command_error("no such file or directory.");
        }

        if(temp->get_file_type() == file_type::PLAIN_TYPE){
            for(unsigned int j = 0; j <temp->con_as_file()->get_data().size(); ++ j){
                    cout << temp->con_as_file()->get_data()[j];
                    cout << " ";
            }
            cout << endl;
        }else{
            throw command_error("cant cat a directory.");
        }
   }

}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   // checking for just cd with no arguments
   if(words.size() == 1){
       state.set_cwd(state.get_root());
       return;
   }else if(words[1] == "/"){
       state.set_cwd(state.get_root());
       return;
   }
   
   inode_ptr temp = state.get_cwd()->con_as_dir()->string_to_inode(
       words[1], state.get_cwd());

    if(temp == nullptr){
        throw command_error("No such directory.");
    }

   if(temp->get_file_type() == file_type::DIRECTORY_TYPE){
       state.set_cwd(temp);
   }else{
       throw command_error("cant cd a non directory.");
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
       exit_status::set(0);
       throw ysh_exit();
   }
   char* temp;
   int x = std::strtol(words[1].c_str(), &temp, 10);
   if(temp == words[1].c_str() || *temp != '\0'){
       x = 127;
   }
   exit_status::set(x);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if(words.size() == 1){
       state.get_cwd()->con_as_dir()->print_dir();
       return;
   }

   for(unsigned int i = 1; i < words.size(); ++ i){
        // check if the file exists if it doesnt throw error
        inode_ptr temp;
        if(words[i].at(0) == '/'){
                temp = state.get_cwd()->con_as_dir()->string_to_inode(words[i],
                 state.get_root());
        }else{
                temp = state.get_cwd()->con_as_dir()->string_to_inode(words[i],
                 state.get_cwd());
        }

        if(words.size() > 2){
                string str = state.get_cwd()->con_as_dir()->inode_to_string(temp,
                 state.get_root());
                cout << str << ":" << endl;
        }

        if(temp == nullptr){
            // its not there
            throw command_error("no file/directory found.");
        }
        if(temp->get_file_type() == file_type::DIRECTORY_TYPE){
            temp->con_as_dir()->print_dir();
        }else{
            fn_echo(state, words);
        }

   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
       state.get_cwd()->con_as_dir()->recursive_print_dir(state.get_cwd(),
       state.get_cwd(), true, state.get_root());
       return;
   }
   for(unsigned int i = 1; i < words.size(); ++ i){
        inode_ptr temp;
        if(words[i].at(0) == '/'){
                temp = state.get_cwd()->con_as_dir()->string_to_inode(words[i],
                 state.get_root());
        }else{
                temp = state.get_cwd()->con_as_dir()->string_to_inode(words[i],
                 state.get_cwd());
        }

        if(temp == nullptr){
            // its not there
            throw command_error("no file/directory found.");
        }
        if(temp->get_file_type() == file_type::DIRECTORY_TYPE){
            temp->con_as_dir()->recursive_print_dir(temp, temp, true,
             state.get_root());
        }else{
            fn_echo(state, words);
        }
   }
   // check if the file exists if it doesnt throw error
}

// words[1] = pathname, rest is in the vector
void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   // see if its there if not throw error
   inode_ptr temp = state.get_cwd()->con_as_dir()->string_to_inode(words[1],
    state.get_cwd());

   if(temp != nullptr){
       // check the file type
       if(temp->get_file_type() == file_type::DIRECTORY_TYPE){
          throw command_error("Cannot make, because its a directory.");
       }else{
           // overright the content rather that make a new file
           // write the contents to data
           temp->con_as_file()->writefile(words);
       }
       return;
   }
   // make a file with the arg[1] as the name
   temp = state.get_cwd()->con_as_dir()->mkfile(words[1], state.get_cwd());
   if(temp == nullptr){
       throw command_error("make: cant make");
   }
   // write the contents to data
   temp->con_as_file()->writefile(words);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
       throw command_error("mkdir: missing parameter");
   }
   if(state.get_cwd()->con_as_dir()->mkdir(words[1], state.get_cwd()) == nullptr){
       throw command_error("mkdir: no directory");
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string new_prompt = "";
   for(unsigned int i = 1; i < words.size(); ++ i){
       new_prompt += words[i] + " ";
   }
   state.set_prompt(new_prompt);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string path = state.get_cwd()->con_as_dir()->inode_to_string(state.get_cwd(),
   state.get_root());
   cout << path << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1 || words[1] == "." || words[1] == ".." || words[1] == "/"){
       throw command_error("No rm target.");
   }

   // see if its there if not throw error
   inode_ptr temp = state.get_cwd()->con_as_dir()->string_to_inode(words[1],
    state.get_cwd());

   if(temp == nullptr){
       throw command_error("No file found. Cannot be removed.");
   }

   state.get_cwd()->con_as_dir()->remove(temp, state.get_cwd(), words[1]);

}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1 || words[1] == "." || words[1] == ".."){
       throw command_error("No rm target.");
   }

   // see if its there if not throw error
   inode_ptr temp = state.get_cwd()->con_as_dir()->string_to_inode(words[1],
    state.get_cwd());
   if(temp == nullptr){
       throw command_error("No file found. Cannot be removed.");
   }
   state.get_cwd()->con_as_dir()->recursive_remove_dir(temp,temp, true,
    state.get_root(),state.get_cwd(),words[1]);
}

