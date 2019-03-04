// $Id: listmap.tcc,v 1.11 2018-01-25 14:19:14-08 - - $

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* next, node* prev,
                                     const value_type& value):
            link (next, prev), value (value) {
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
   for (iterator i = begin(); i != end();) {
      i = erase(i);
   }
   DEBUGF ('l', reinterpret_cast<const void*> (this));
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {

   iterator found = find(pair.first);
   //exist
   if (found != end()) {   //replace with new pair
      found->second = pair.second;
      return found;
   }

   //not exist
   iterator i;
   for (i = begin(); i != end(); ++i) {
      bool not_less = !less(i->first, pair.first);
      bool greater = less(pair.first, i->first);
      bool fit = not_less and greater;
      if (fit) {
         break;
      }
   }

   node* n = new node(i.where, i.where->prev, pair);
   i.where->prev->next = n;
   i.where->prev = n;
   return i;

   DEBUGF ('l', &pair << "->" << pair);
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) {

   iterator i;
   for (i = begin(); i != end(); ++i) {
      bool not_less = !less(i->first, that);
      bool not_greater = !less(that, i->first);
      bool equal = not_less and not_greater;
      if (equal) 
         return i;
   }

   //key not found?
   return end();
   DEBUGF ('l', that);
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {

   auto i = find(position.where->value.first);
   if (i != end()) {
      position.where->prev->next = position.where->next;
      position.where->next->prev = position.where->prev;
      iterator curr(position.where->next);

      delete position.where;
      return curr;
   } else {
      //cout << "key not found" << endl;
      return iterator();
   }

   DEBUGF ('l', &*position);
   
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
   DEBUGF ('l', where);
   return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
   DEBUGF ('l', where);
   return &(where->value);
}
   


//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
   DEBUGF ('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
   DEBUGF ('l', where);
   where = where->prev;
   return *this;
}


//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
            (const iterator& that) const {
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
            (const iterator& that) const {
   return this->where != that.where;
}

