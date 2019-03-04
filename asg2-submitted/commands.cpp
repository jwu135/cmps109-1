// $Id: commands.cpp,v 1.17 2018-01-25 14:02:55-08 - - $

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"#"     , fn_comment},
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"    , fn_rmr  },
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

void fn_comment (inode_state&, const wordvec&){
   //do nothing
}

void fn_cat (inode_state& state, const wordvec& words){
   if (words.size() == 1) 
      throw command_error("missing argument");
   else 
      state.goto_cat(state, words);

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_cd (inode_state& state, const wordvec& words){
   if (words.size() > 2) {
      throw command_error("incorrect commands");
   } else {
      if (words.size() < 2) 
         state.goto_cd(state, " ");
      else 
         state.goto_cd(state, words[1]);
   }

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_echo (inode_state& state, const wordvec& words){
   string result = "";

   cout << word_range(words.cbegin()+1, words.cend()) << endl;

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}


void fn_exit (inode_state& state, const wordvec& words){
   int status = 0;
   exit_status e;
   string s = "";
   if (words.size() > 1) {
      for (size_t i = 1; i < words.size(); ++i) 
         s += words[i];

      for (size_t i = 0; i < s.size(); ++i) {
         if (isalpha(s[i])) {
            status = 127;
            break;
         }
      }

      if (status != 127) status = stoi(s);
   }

   e.set(status);

   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   state.goto_ls(state, words);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_lsr (inode_state& state, const wordvec& words){
   state.goto_lsr(state, words);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   if (words.size() == 1) 
      throw command_error("missing argument");
   else 
      state.goto_make(state, words);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_mkdir (inode_state& state, const wordvec& words){  //working
   if (words.size() < 2) {
      throw command_error("missing argument");
   } else if (words.size() > 2) {
      throw command_error("incorrect command");
   } else {
      state.goto_mkdir(state, words[1]);
   }

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_prompt (inode_state& state, const wordvec& words){
   string new_prompt = "";
   for (size_t i = 1; i < words.size(); ++i) {
      new_prompt += words[i] + " ";
   }

   state.set_prompt(new_prompt);

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   if (words.size() == 1) 
      state.goto_pwd(state);
   else 
      throw command_error("incorrect command");

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words){
   if (words.size() == 1) 
      throw command_error("incorrect command");
   else 
      state.goto_rm(state, words);

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   if (words.size() == 1) 
      throw command_error("incorrect command");
   else 
      state.goto_rmr(state, words);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

