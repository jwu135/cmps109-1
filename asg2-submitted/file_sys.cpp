// $Id: file_sys.cpp,v 1.6 2018-06-27 14:44:57-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <iomanip>

using namespace std;

#include "debug.h"
#include "file_sys.h"
#include "commands.h"

int inode::next_inode_nr {1};

void inode_state::set_prompt(string& p) {
  prompt_ = p;
}

void inode_state::goto_cd(inode_state& state, string name) { 

  wordvec target = split(name, "/");
  inode_ptr curr = state.get_cwd();
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  bool found = false;

  if (name == " " or name.compare("/")==0) {
    cwd = root;
  } else if (target.size() == 1) {
    for (auto i: curr_dirents) {
      if (i.first.compare(name) == 0) {
        curr = i.second;
        cwd = curr;
        found = true;
      }
    }
    curr_dirents = curr->contents->get_dirents();
    if (not found) {throw command_error("cd1: no such directory");}
  } else if (target.size() > 1) {  //case: dir1/dir2 has problem
    for (size_t i = 0; i < target.size(); ++i) {
      found = false;
      for (auto j: curr_dirents) {
        if (j.first.compare(target[i]) == 0) {
          curr = j.second;
          cwd = curr;
          found = true;
          break;
        }
      }
      curr_dirents = curr->contents->get_dirents();
      if (not found) {throw command_error("cd2: no such directory");}
    }
  }
}

void inode_state::goto_mkdir(inode_state& state, string name) {
  wordvec target = split(name, "/");
  inode_ptr curr = state.get_cwd();
  bool found = false;
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();

  if (target.size() > 1) { //case: dir1/dir2
    for (size_t i = 0; i < target.size()-1; ++i) {
      found = false;
      for (auto j: curr_dirents) {
        if (j.first.compare(target[i]) == 0) {
          curr = j.second;
          name = target[i+1];
          found = true;
          break;
        }
      }
      curr_dirents = curr->contents->get_dirents();
      if (not found) {throw command_error("mkdir: no such directory");}
    }
  } 

  string name_ = target[target.size()-1];
  for (auto i: curr_dirents) {
    if (i.first.compare(name_) == 0) {
      throw command_error("mkdir: name exists");
    }
  }

  inode_ptr new_dir = curr->contents->mkdir(name);
  new_dir->contents->get_state();
  //connect with parent
  map<string, inode_ptr>& new_dirent = new_dir->contents->get_dirents();
  new_dirent["."] = new_dir;
  new_dirent[".."] = curr;
  curr_dirents.insert(pair<string, inode_ptr>(name, new_dir));
  curr->contents->update_dirents(curr_dirents);
}

void inode_state::goto_pwd(inode_state& state) {  
  vector<string> path;
  inode_ptr curr = state.get_cwd();
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  string s = "";

  while (curr != root) {
    s = state.get_name(curr, curr_dirents);
    path.insert(path.begin(), s);
    curr = curr_dirents[".."];
    curr_dirents = curr->contents->get_dirents();
  }

  for (size_t i = 0; i < path.size(); ++i) {
    cout << "/" << path[i];
  }

  if (path.size() < 1) {cout << "/";}
  cout << endl;
}

string inode_state::get_name(inode_ptr curr, map<string, inode_ptr> m) {
  string s;
  inode_ptr parent = m[".."];
  map<string, inode_ptr> parent_dir = parent->contents->get_dirents();
  auto i = parent_dir.begin();
  for (; i != m.end(); ++i) {
    if (i->second == curr) {
      s = i->first;
      break;
    }
  }
  return s;
}

void inode_state::goto_ls(inode_state& state, const wordvec& words) { 
  wordvec target;
  if (words.size() > 1)
     target = split(words[1], "/");
  bool found = false;
  inode_ptr curr = state.get_cwd();
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  string format = "";

  if (words.size() == 1) {
    if (curr == root){
      format = "/";
    } else {
      format = "/" + curr->getname(curr);
    }
  } else {  //has args
    if (words[1] == "/") {
      format = "/";
    } else if (words[1] == ".") {
      format = ".";
    } else if (words[1] == "..") {
      format = "..";
    } else {
      format = "/" + words[1];
    }
  }

  if (target.size() > 1) {  //case: ls dir1/dir2
    for (size_t i = 0; i < target.size(); ++i) {
      found = false;
      for (auto j: curr_dirents) {
        if (j.first.compare(target[i]) == 0) {
          curr = j.second;
          found = true;
        }
      }
      curr_dirents = curr->contents->get_dirents();
      if (not found) {throw command_error("ls: directory not found");}
    }
  } else if (target.size() == 1) {  //case: ls dir
    for (auto i: curr_dirents) {
      if (i.first.compare(target[0]) == 0) {
        curr = i.second;
        found = true;
      }
    }
    curr_dirents = curr->contents->get_dirents();
    if (not found) {throw command_error("ls: directory not found");}  
  }

  cout << format << ":" << endl;
  for (auto i : curr_dirents) {
    bool isDir = i.second->contents->isDirectory();
    if (i.first == "." or i.first == "..") {
      cout << setw(6) << i.second->get_inode_nr() << setw(6) 
          << i.second->contents->size() << " " << i.first << endl;
    } else if (isDir) {
      cout << setw(6) << i.second->get_inode_nr() << setw(6) 
          << i.second->contents->size() << " " << i.first 
          << "/" << endl;
    } else {
      cout << setw(6) << i.second->get_inode_nr() << setw(6) 
          << i.second->contents->size() << " " << i.first << endl;
    }
  }
}

void inode_state::goto_lsr(inode_state& state, const wordvec& words) {
  inode_ptr curr = state.get_cwd();
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  bool found;
  string format; 

  if (words.size() == 1) {  //no argument following
    format = curr->getname(curr);
    lsr(curr, format);
  } else if (words.size() > 1) {  //goto target dir
    wordvec target = split(words[1], "/");
    if (words[1] == "/") {
      curr = root;
      curr_dirents = curr->contents->get_dirents();
    }
    for (size_t t = 0; t < target.size(); ++t) {
      found = false;
      for (auto i : curr_dirents) {
        if (i.first == target[t]) {
          curr = i.second;
          found = true;
        }
      }
      curr_dirents = curr->contents->get_dirents();
      if (not found) {throw command_error("lsr: directory not found");}
    }
    format = curr->getname(curr);
    lsr(curr, format);
  }
}

void lsr(inode_ptr& curr, string format) {
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  string format_ = format;
  cout << "/" << format << ":" << endl;

  for (auto i : curr_dirents) {
    bool isDir = i.second->contents->isDirectory();
    if (i.first == "." or i.first == "..") {
      cout << setw(6) << i.second->get_inode_nr() << setw(6) 
          << i.second->contents->size() << " " << i.first << endl;
    } else if (isDir) {
      cout << setw(6) << i.second->get_inode_nr() << setw(6) 
          << i.second->contents->size() << " " << i.first 
          << "/" << endl;
    } else {
      cout << setw(6) << i.second->get_inode_nr() << setw(6) 
          << i.second->contents->size() << " " << i.first << endl;
    }
  }
  
  for (auto i : curr_dirents) {
    bool isDir = i.second->contents->isDirectory();
    if (isDir) {
      if (i.first == "." or i.first == "..") ;
      else {
        if (format == "") {
          format = format + i.second->getname(i.second);
        } else {
          format = format_ + "/" + i.second->getname(i.second);
        }
        lsr(i.second, format);
        format = format_;
      }
    }
  }
}

void inode_state::goto_make(inode_state& state, const wordvec& words) {
  wordvec target = split(words[1], "/");
  inode_ptr curr = state.get_cwd();
  bool found = false;
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();

  if (target.size() > 1) { 
    for (size_t i = 0; i < target.size()-1; ++i) {
      found = false;
      for (auto j: curr_dirents) {
        if (j.first.compare(target[i]) == 0) {
          curr = j.second;
          found = true;
          break;
        }
      }
      curr_dirents = curr->contents->get_dirents();
      if (not found) {throw command_error("make: no such directory");}
    }
  } 

  string file_name = target[target.size()-1];
  auto i = curr_dirents.begin();
  for (; i != curr_dirents.end(); ++i) {
    if (i->first == file_name) {
      //if it is a directory
      bool isDir = i->second->contents->isDirectory();
      if (isDir) 
        throw command_error("make: directory exists with same name");
      else {  //rewrite file with same name
        i->second->contents->writefile(words);
        curr_dirents.insert(pair<string, inode_ptr>(file_name, i->second));
        ++i;
      }
    }
  }
  curr->contents->update_dirents(curr_dirents);

  //if file not exists
  inode_ptr new_file = curr->contents->mkfile(file_name);
  new_file->contents->writefile(words);
  curr_dirents.insert(pair<string, inode_ptr>(file_name, new_file));
  curr->contents->update_dirents(curr_dirents);
}

void inode_state::goto_cat(inode_state& state, const wordvec& words) {
  inode_ptr curr = state.get_cwd();
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  bool found = false;

  for (size_t i = 1; i < words.size(); ++i) {
    for (auto j: curr_dirents) {
      if (j.first.compare(words[i]) == 0) {
        bool isDir = j.second->contents->isDirectory();
        if (isDir) 
          throw command_error("cat: can not read directory");
        else {
          for (auto& k: j.second->contents->readfile()) 
            cout << k << " ";
          cout << endl;
          found = true;
        }
      }
    }
    if (not found) {throw command_error("cat: file not found");}
  }

}

void inode_state::goto_rm(inode_state& state, const wordvec& words) {
  inode_ptr curr = state.get_cwd();
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  wordvec target = split(words[1], "/");
  bool found = false;

  for (size_t i = 1; i < words.size(); ++i) {
    auto j = curr_dirents.begin();
    for(; j != curr_dirents.end(); ++j) {
      if (j->first == words[i]) {
        bool isDir = j->second->contents->isDirectory();
        if (isDir) {
          if (j->second->contents->size() != 2) 
            {throw command_error("rm: directory not empty");}
          else {
            curr_dirents.erase(j);
            --j;
            found = true;
          }
        } else {
          curr_dirents.erase(j);
          --j;
          found = true;
        }
      }
    }
    curr->contents->update_dirents(curr_dirents);
    if (not found) 
      {throw command_error("rm: file or directory not found");}
  }

}

void inode_state::goto_rmr(inode_state& state, const wordvec& words) {
  inode_ptr curr = state.get_cwd();
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  wordvec target = split(words[1], "/");
  string name;
  bool found;

  if (target.size() == 1) {
    auto i = curr_dirents.begin();
    for (; i != curr_dirents.end(); ++i) {
      if (i->first == target[0]) {
        bool isDir = i->second->contents->isDirectory();
        if (isDir) {
          curr = i->second;
          found = true;
          rmr(curr);
        } else {
          curr_dirents.erase(i);
          found = true;
          --i;
        }
      }
    }
    curr->contents->update_dirents(curr_dirents);
    if (not found) {throw command_error("rmr: target not found");}
  }
  
}

void rmr(inode_ptr& curr) {
  map<string, inode_ptr> curr_dirents = curr->contents->get_dirents();
  bool isDir = curr->contents->isDirectory();
  if (isDir and curr_dirents.size() > 2) {  //not empty dir
    auto i = curr_dirents.begin();
    for (; i != curr_dirents.end(); ++i) {
      bool isDir1 = i->second->contents->isDirectory();
      if (i->first == "." or i->first == "..") {;}
      else if (isDir1) {
        rmr(i->second);
      } else {
        curr_dirents.erase(i);
        --i;
      }
    }
    curr->contents->update_dirents(curr_dirents);
  } 

  //else delete empty dir or file
  inode_ptr parent = curr_dirents[".."];
  map<string, inode_ptr> parent_dir = parent->contents->get_dirents();
  auto i = parent_dir.begin();
  for (; i != parent_dir.end(); ++i) {
    if (i->first == "." or i->first == "..") {;}
    else if (i->first == curr->getname(curr)) {
      parent_dir.erase(i);
      --i;
    }
  }
  parent->contents->update_dirents(parent_dir);
}

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

inode_state::inode_state() {  //ctor
  root = make_shared<inode>(file_type::DIRECTORY_TYPE);
  cwd = root; 
  map<string, inode_ptr>& r_dirents = root->contents->get_dirents();
  r_dirents["."] = root;
  r_dirents[".."] = root;

   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() const { return prompt_; }

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
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

string inode::getname(inode_ptr curr) {
  string s;
  map<string, inode_ptr> m = curr->contents->get_dirents();
  inode_ptr parent = m[".."];
  map<string, inode_ptr> parent_dir = parent->contents->get_dirents();
  for (auto i : parent_dir) {
    if (i.second == curr) {
      s = i.first;
      break;
    }
  }

  if (s.compare(".") == 0) {return "";}
  return s;
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   size = data.size() - 1;
   for (auto i : data) {
    size += i.size();
   }

   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
  if (words.size() > 2) {
    wordvec new_data;
    for (size_t i = 2; i < words.size(); ++i) {
      new_data.push_back(words[i]);
    }
    data = new_data; 
  }
  
   DEBUGF ('i', words);
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

map<string, inode_ptr>& plain_file::get_dirents () {
   throw file_error ("is not a directory");
}

void plain_file::update_dirents (map<string, inode_ptr>&) {
  throw file_error ("is not a directory");
}

void plain_file::get_state () {
  throw file_error ("is not a directory");
}

bool plain_file::isDirectory() {
  return false;
}



directory::directory() {  //ctor
  dirents.insert(pair<string, inode_ptr>(".", nullptr));
  dirents.insert(pair<string, inode_ptr>("..", nullptr));
}

size_t directory::size() const {
  size_t size {0};
  size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}

inode_ptr directory::mkdir (const string& dirname) {  
  inode_ptr new_dir = make_shared<inode>(file_type::DIRECTORY_TYPE);

   DEBUGF ('i', dirname);
   return new_dir;
}

inode_ptr directory::mkfile (const string& filename) {
  inode_ptr new_file = make_shared<inode>(file_type::PLAIN_TYPE);

   DEBUGF ('i', filename);
   return new_file;
}

map<string, inode_ptr>& directory::get_dirents() {
  return dirents;
}

void directory::update_dirents(map<string, inode_ptr>& m) {
  dirents = m;
}

void directory::get_state() {
  is_Directory = true;
}

bool directory::isDirectory() {
  return is_Directory;
}

