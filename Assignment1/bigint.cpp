// $Id: bigint.cpp,v 1.76 2016-06-14 16:34:24-07 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 1/22/2018
*/
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
                uvalue(uvalue), is_negative(is_negative) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   // if both values are negative return the sum of both values and mark as negative
   // this is a pointer to the current class and that the other object

   // handle if both are posotive or negative
   if(this->is_negative == that.is_negative){
       return {this->uvalue + that.uvalue, this->is_negative};
   }

   // it takes the sign of whatever is larger
   if(this->uvalue > that.uvalue){
       return {this->uvalue - that.uvalue, this->is_negative};
   }else if(this->uvalue < that.uvalue){
       return {that.uvalue - this->uvalue, that.is_negative};
   }else{
       // both values are equal and you can return 0 and its pos
       return {ubigint("0"), false};
   }
}

bigint bigint::operator- (const bigint& that) const {
   // used +operator as a starting point
   // check if the same is_negative
   if (this->is_negative == that.is_negative) {
      // both is_negatives are the same
      // base case the values are equal return 0
      if (this->uvalue == that.uvalue){
          // return 0 and posotive
          return {ubigint("0"), false};
      }else if (that.uvalue < this->uvalue){
          // uvalue is less that that.uvalue
          return {this->uvalue - that.uvalue, this->is_negative};
      }else{
          // uvalue is more that that.uvalue
          return {that.uvalue - this->uvalue, !this->is_negative};
      } 
   }
   return {this->uvalue + that.uvalue, this->is_negative};
}

bigint bigint::operator* (const bigint& that) const {
   // base case both signs are equal
   // when both signs are the same they return the same
   if(this->is_negative == that.is_negative){
       // return posotive
       return {this->uvalue * that.uvalue, false};
   }
   // neg * pos is neg
   // if they arnt the same then one must be neg makeing it neg
   return {this->uvalue * that.uvalue, true};
}

bigint bigint::operator/ (const bigint& that) const {
   // based on *operator
   // base case both signs are equal
   // when both signs are the same they return the same
   if(this->is_negative == that.is_negative){
       // as long as they are the same sign they will return posotive
       if(this->uvalue == that.uvalue){
           // divide by itself = 1
           return {ubigint("1"), false};
       }
       return {this->uvalue / that.uvalue, false};
   }else{
        if(this->uvalue == that.uvalue){
           // divide by itself = -1
           return {ubigint("1"), true};
       }
       // if they are dif it will be negative
       return { this->uvalue / that.uvalue, true};
   }

}

bigint bigint::operator% (const bigint& that) const {
   bigint result = uvalue % that.uvalue;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << (that.is_negative ? "-" : "")
              <<that.uvalue;
}

