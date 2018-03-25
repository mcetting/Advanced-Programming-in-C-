// $Id: commands.cpp,v 1.17 2018-01-25 14:02:55-08 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 2/05/2018
*/
#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   // make the default cwd and root to be the same thing
   // make a root_file of type directory and set it to 
   // blank for default
   inode root_file(file_type::DIRECTORY_TYPE);

   // make a shared pointer to root_file becaause
   // it will keep access after dealocation?
   root = make_shared<inode>(root_file);

   // defaults cwd to root directory
   cwd = root;

   root->con_as_dir()->mkfirst(root);

   // debug
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
          
}

inode_state::~inode_state(){
    while(root->con_as_dir()->get_dirents().size() > 2){
        auto p = root->con_as_dir()->get_dirents().begin();
        p ++;
        p ++;

        root->con_as_dir()->recursive_remove_dir(p->second, p->second, true,
        root,root,p->first);
    }

    root->con_as_dir()->get_dirents().find(".")->second = nullptr;
    root->con_as_dir()->get_dirents().find("..")->second = nullptr;
}

void inode_state::set_cwd(inode_ptr n){
    cwd = n;
}

const string& inode_state::prompt() const { return prompt_; }

void inode_state::set_prompt(string new_prompt){ prompt_ = new_prompt;}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   this->type = type;
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

// returns the file type
file_type inode::get_file_type(){
    return type;
}

// returns root
inode_ptr inode_state::get_root(){
    return root;
}
// returns cwd
inode_ptr inode_state::get_cwd(){
    return cwd;
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}
// contents to file/directory
shared_ptr<directory> inode::con_as_dir(){
    return dynamic_pointer_cast<directory>(contents);
}

shared_ptr<plain_file> inode::con_as_file(){
    return dynamic_pointer_cast<plain_file>(contents);
}

file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data.clear();
   // dont put the first 2 in
   for(unsigned int i = 2; i < words.size(); ++ i){
       data.push_back(words[i]);
   }

}

void plain_file::remove (inode_ptr, inode_ptr, const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&, inode_ptr) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&, inode_ptr) {
   throw file_error ("is a plain file");
}


size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::mkfirst(inode_ptr root){
    dirents.insert({".", root});
    dirents.insert({"..", root});;
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (inode_ptr ptr, inode_ptr cwd,
 const string& filename) {
   // get parent
   inode_ptr parent_dir;

   // make a file in this directory
   auto temp = filename;
   auto temp_the_second = filename;
   auto x = temp.find_last_of('/');
   if(x != string::npos){
       temp = temp.substr(0, x);
       temp_the_second = temp_the_second.substr(x + 1, 
       (temp_the_second.size() - x) - 1);
       parent_dir = string_to_inode(temp, cwd);
   }else{
       parent_dir = cwd;
   }

//-------------------------------------------------------------------//

   if(ptr->get_file_type() == file_type::DIRECTORY_TYPE){
       // check size of directory
       // 2 for . and ..
       if(ptr->con_as_dir()->dirents.size() == 2){
           // do it
           ptr->con_as_dir()->dirents.find(".")->second = nullptr;
           ptr->con_as_dir()->dirents.find("..")->second = nullptr;

           parent_dir->con_as_dir()->dirents.find(temp_the_second)->second
            = nullptr;
           parent_dir->con_as_dir()->dirents.erase(
               parent_dir->con_as_dir()->dirents.find(temp_the_second));
       }
   }else{
       parent_dir->con_as_dir()->dirents.find(temp_the_second)->second
        = nullptr;
       parent_dir->con_as_dir()->dirents.erase(
           parent_dir->con_as_dir()->dirents.find(temp_the_second));
   }
}


// SEARCH FUNCTION
inode_ptr directory::string_to_inode(const string& str, inode_ptr cwd){
   wordvec paths = split(str, "/");
   inode_ptr cur = cwd;
   for(unsigned int i = 0; i < paths.size(); ++ i){
       if(cur->con_as_dir()->dirents.find(paths[i]) != 
          cur->con_as_dir()->dirents.end()){
           cur = cur->con_as_dir()->dirents.at(paths[i]);
       }else{
           return nullptr;
       }
   }
   return cur;
}

string directory::get_cwd_name(inode_ptr cwd){
    inode_ptr temp = cwd;
    // get parent of cwd
    inode_ptr parent = cwd->con_as_dir()->dirents.find("..")->second;
    // go through parent dirents and try to match for cwd
    // then return its name from the pair in dirents

    // for each p in dirents
    for(auto p : parent->con_as_dir()->dirents){
        if(p.second == cwd){
            return p.first;
        }
    }
    return "";
}
// finds the path by searching through the tree for the pointer
string directory::inode_to_string(inode_ptr dir, inode_ptr root){
    // make a temp so we can play with it
    inode_ptr cwd_temp = dir;

    if(cwd_temp == root){
        // puts the root slash on the left then ends
        return "/";
    }

    // step 1 print the name of this diretory
    string name = "";

    string cwd_name = get_cwd_name(cwd_temp);
    name = cwd_name;

    // set to parent
    cwd_temp = cwd_temp->con_as_dir()->dirents.find("..")->second;

    while(true){
        // base case
        if(cwd_temp == root){
            // puts the root slash on the left then ends
            return "/" + name;
        }

        string cwd_name = get_cwd_name(cwd_temp);
        name = cwd_name + "/" + name;
        // set to parent
        cwd_temp = cwd_temp->con_as_dir()->dirents.find("..")->second;
    }
    return name;
}

inode_ptr directory::mkdir (const string& dirname, inode_ptr cwd) {
   DEBUGF ('i', dirname);

   inode new_dir(file_type::DIRECTORY_TYPE);

   inode_ptr new_dir_ptr = make_shared<inode>(new_dir);

   //dirents.insert(p);

   auto temp = dirname;
   auto temp_the_second = dirname;
   auto x = temp.find_last_of('/');
   if(x != string::npos){
       temp = temp.substr(0, x);
       temp_the_second = temp_the_second.substr(x + 1, 
       (temp_the_second.size() - x) - 1);
   }

   pair<string, inode_ptr> p{temp_the_second,new_dir_ptr};

   inode_ptr q = new_dir.con_as_dir()->string_to_inode(temp, cwd);
   if(x == string::npos){
       q = cwd;
   }
   if(q == nullptr){
       return nullptr;
   }
   q->con_as_dir()->dirents.insert(p);

   new_dir.con_as_dir()->dirents.insert({".", new_dir_ptr});
   new_dir.con_as_dir()->dirents.insert({"..", q});

   return new_dir_ptr;
}

inode_ptr directory::mkfile (const string& filename, inode_ptr cwd) {
   DEBUGF ('i', filename);

   inode new_file(file_type::PLAIN_TYPE);

   inode_ptr new_file_ptr = make_shared<inode>(new_file);
   inode_ptr parent_dir;
   // make a file in this directory
   auto temp = filename;
   auto temp_the_second = filename;
   auto x = temp.find_last_of('/');
   if(x != string::npos){
       temp = temp.substr(0, x);
       temp_the_second = temp_the_second.substr(x + 1, 
       (temp_the_second.size() - x) - 1);
       parent_dir = string_to_inode(temp, cwd);
   }else{
       parent_dir = cwd;
   }


   if(parent_dir == nullptr){
       return nullptr;
   }

   pair<string, inode_ptr> p{temp_the_second, new_file_ptr};

   parent_dir->con_as_dir()->dirents.insert(p);
   return new_file_ptr;
}

int return_size(wordvec words){
    int size = 0;
    for(unsigned int i= 0; i < words.size(); ++ i){
        size += words[i].length();
    }
    return size;
}

// recursive_print
void directory::recursive_print_dir(inode_ptr starter, inode_ptr current,
 bool first, inode_ptr root){
    if(starter == current && !first){
        return;
    }
    first = false;

    // recurse!
    string str = current->con_as_dir()->inode_to_string(current, root);
    cout << str << ":" << endl;
    current->con_as_dir()->print_dir();

    // loop through dirents
    for(auto p : current->con_as_dir()->dirents){
        if(p.second->get_file_type() == file_type::DIRECTORY_TYPE &&
         p.first != "."&& p.first != ".."){
            recursive_print_dir(starter, p.second, first, root);
        }
    }
    return;
}


void directory::recursive_remove_dir(inode_ptr starter, inode_ptr current,
 bool first, inode_ptr root, inode_ptr cwd, string filename){
    if(starter->get_file_type() == file_type::PLAIN_TYPE){
        if(first == true){
            remove(starter,cwd,filename);
            return;
        }
    }
            
    if(starter == current && !first){
        return;
    }
    first = false;
    while(current->con_as_dir()->get_dirents().size() > 2){
        // delete the third one
        auto p = current->con_as_dir()->get_dirents().begin();
        p++;
        p++;
        if(p->second->get_file_type() == file_type::DIRECTORY_TYPE &&
           p->first != "."&& p->first != ".."){
            recursive_remove_dir(starter, p->second, first,
            root,cwd,filename + "/" + p->first);
        }else if(p->second->get_file_type() == file_type::PLAIN_TYPE){
            remove(p->second,cwd,filename + "/" + p->first);
        }
    }
    remove(current,cwd,filename);
    return;
}

// prints the directories contents to the screen
void directory::print_dir(){
    for(auto p : dirents){
        cout << p.second->get_inode_nr() << "\t";
        // if its a directory then print the p.first else do wordvec
        if(p.second->get_file_type() == file_type::DIRECTORY_TYPE){
            // its a directory!
            cout << p.second->con_as_dir()->dirents.size();
            cout << "\t" << p.first;
            if(p.first != ".." && p.first != "."){
                cout << "/" << endl;
            }else{
                cout << endl;
            }
        }else{
            cout << return_size(p.second->con_as_file()->get_data());
            // the number of characters in the string
            cout << "\t" << p.first << endl;
        }
    }
}


