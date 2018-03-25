// $Id: listmap.tcc,v 1.11 2018-01-25 14:19:14-08 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 2/19/2018
*/
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
listmap<Key, Value, Less>::node::node(node *next, node *prev,
                                      const value_type &value) : link(next, prev), value(value)
{
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
listmap<Key, Value, Less>::~listmap()
{
   DEBUGF('l', reinterpret_cast<const void *>(this));
   while (anchor()->prev != anchor())
   {
      // erasing the list elements
      erase(anchor()->prev);
      erase(anchor());
   }
}

//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::insert(const value_type &pair)
{
   DEBUGF('l', &pair << "->" << pair);

   // check if the node already exists
   if (find(pair.first) != anchor())
   {
      // already exists value is replaced
      find(pair.first).where->value.second = pair.second;
      return find(pair.first);
   }
   iterator itor(anchor()->next);
   node *newNode = new node(anchor(), anchor()->prev, pair);
   bool emp = true;
   for (;;)
   {
      if (itor.where == anchor())
         break;
      // juicy stuff
      if (itor.where->value.first.compare(pair.first) > 0)
      {
         newNode->prev = itor.where->prev;
         newNode->next = itor.where;

         itor.where->prev->next = newNode;
         itor.where->prev = newNode;

         emp = false;
         break;
      }
      ++itor;
   }
   if (emp)
   {
      anchor()->prev->next = newNode;
      anchor()->prev = newNode;
   }

   return iterator(newNode);
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::find(const key_type &that)
{
   DEBUGF('l', that);
   // fnd
   // go through the entire list and see if its there
   for (node *n = anchor()->next; n != anchor(); n = n->next)
   {
      // if its found return an iterator pointing to that node
      if (n->value.first == that)
      {
         return iterator(n);
      }
   }

   // return "off the end"
   return iterator(anchor());
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::erase(iterator position)
{
   DEBUGF('l', &*position);
   node *n = position.where->next;
   if (position.where != anchor())
   {

      // if theres nothing there
      position.where->prev->next = position.where->next;
      position.where->next->prev = position.where->prev;

      // delete the value
      delete (position.where);
   }

   return iterator(n);
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
typename listmap<Key, Value, Less>::value_type &
    listmap<Key, Value, Less>::iterator::operator*()
{
   DEBUGF('l', where);
   return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::value_type *
    listmap<Key, Value, Less>::iterator::operator->()
{
   DEBUGF('l', where);
   return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator &
    listmap<Key, Value, Less>::iterator::operator++()
{
   DEBUGF('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator &
    listmap<Key, Value, Less>::iterator::operator--()
{
   DEBUGF('l', where);
   where = where->prev;
   return *this;
}

//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key, Value, Less>::iterator::operator==(const iterator &that) const
{
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key, Value, Less>::iterator::operator!=(const iterator &that) const
{
   return this->where != that.where;
}
