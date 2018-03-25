// $Id: ubigint.cpp,v 1.14 2016-06-23 17:21:26-07 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 1/22/2018
*/
#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that): ubig_value (that) {
  // DEBUGF ('~', this << " -> " << ubig_value)
}

ubigint::ubigint (const string& that): ubig_value(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (int i = that.length() - 1; i >= 0; --i) {
      if (not isdigit (that[i])) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      //ubig_value = ubig_value * 10 + digit - '0';
      ubig_value.push_back(that[i] - 48);
   }
}
using digit_c = unsigned char;
ubigint ubigint::operator+ (const ubigint& that) const {
   // get bigger ubig_value
   const ubigvalue_t* ubig_tempvalue_small_ref;
   const ubigvalue_t* ubig_tempvalue_big_ref;

   if(ubig_value.size() > that.ubig_value.size()){
       ubig_tempvalue_big_ref = &ubig_value;
       ubig_tempvalue_small_ref = &that.ubig_value;
   }else if(ubig_value.size() < that.ubig_value.size()){
       ubig_tempvalue_big_ref = &that.ubig_value;
       ubig_tempvalue_small_ref = &ubig_value;
   }else{
       ubig_tempvalue_big_ref = &ubig_value;
       ubig_tempvalue_small_ref = &that.ubig_value;
   }
   

   ubigint ubig_result("0");
   ubig_result.ubig_value.pop_back();

   digit_c digit = 0;
   for(unsigned int i = 0; i < ubig_tempvalue_big_ref->size(); ++i){
       digit_c dig_res;
       if(ubig_tempvalue_small_ref->size() <= i){
           dig_res = (*ubig_tempvalue_big_ref)[i] + digit;
       }else{
           dig_res = (*ubig_tempvalue_big_ref)[i] + (*ubig_tempvalue_small_ref)[i] + digit;
       }
       digit = 0;
       if(dig_res >= 10){
           // carry
           digit = 1;
           dig_res = dig_res - 10;
       }
       
       ubig_result.ubig_value.push_back(dig_res);
   }
   if(digit != 0){
        ubig_result.ubig_value.push_back(digit);

   }
   return ubig_result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");

   // get bigger ubig_value
   const ubigvalue_t* ubig_tempvalue_small_ref;
   const ubigvalue_t* ubig_tempvalue_big_ref;

   // just to see which is the small and big
   ubig_tempvalue_big_ref = &this->ubig_value;
   ubig_tempvalue_small_ref = &that.ubig_value;

   ubigint result("0");
   result.ubig_value.pop_back();
   //unsigned int i = 0;
   int carry = 0;
   int current = 0;
   for(unsigned int i = 0; i < this->ubig_value.size(); ++ i){
       if(i < ubig_tempvalue_small_ref->size()){
           // handle small stuff first
           current = ubig_value[i] - carry;
           carry = 0;

           if (current < that.ubig_value[i]){
              carry = 1;
              current = current + 10;
           }
           result.ubig_value.push_back(current - that.ubig_value[i]);
       }else if(i < ubig_tempvalue_big_ref->size()){
           // theres more to do
           if (carry > 0) {
              if(this->ubig_value[i] == 0){
                  result.ubig_value.push_back(0);
              }else{
                  for(int x = result.ubig_value.size() - 1; x >= 0; -- x){
                    if(result.ubig_value[x] == 0){
                        result.ubig_value[x] = 9;
                    }else{
                        break;
                    }
                  }

                  carry = 0;
                  result.ubig_value.push_back((this->ubig_value[i] - 1));
              }
           } else {
              result.ubig_value.push_back(this->ubig_value[i]);
           }
       }
   }
   bool run = true;
   while(run){
        if(result.ubig_value[result.ubig_value.size() - 1] == 0){
            if(result.ubig_value.size() != 1){
                result.ubig_value.erase(result.ubig_value.end() - 1);
            }else{
                run = false;
            }
        }else{
                run = false;
        }
   }
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint ubig_result("0");

   ubig_result.ubig_value.pop_back();

   const ubigvalue_t* ubig_tempvalue_small_ref;
   const ubigvalue_t* ubig_tempvalue_big_ref;

   // bigintin it, ya feel me holmes?
   if(ubig_value.size() > that.ubig_value.size()){
       ubig_tempvalue_big_ref = &ubig_value;
       ubig_tempvalue_small_ref = &that.ubig_value;
   }else if(ubig_value.size() < that.ubig_value.size()){
       ubig_tempvalue_big_ref = &that.ubig_value;
       ubig_tempvalue_small_ref = &ubig_value;
   }else{
       if(ubig_value[ubig_value.size() - 1] > that.ubig_value[that.ubig_value.size() - 1]){
            ubig_tempvalue_big_ref = &ubig_value;
            ubig_tempvalue_small_ref = &that.ubig_value;
       }else{
            ubig_tempvalue_big_ref = &that.ubig_value;
            ubig_tempvalue_small_ref = &ubig_value;
       }
   }
   digit_c carry = 0;
   for(unsigned int i = 0; i < ubig_tempvalue_small_ref->size(); ++i){
       // clear
       ubigint partial_product;
       for(unsigned int j = 0; j < i; ++ j){
            partial_product.ubig_value.push_back(0);
       }

       for(unsigned int j = 0; j < ubig_tempvalue_big_ref->size(); ++j){
           digit_c mult;
           mult = (*ubig_tempvalue_small_ref)[i] * (*ubig_tempvalue_big_ref)[j];
           if(carry != 0 ){
               mult += carry;
               carry = 0;
           }
           // get the 10
           carry = mult / 10;
           partial_product.ubig_value.push_back(mult % 10);
       }
       if(carry != 0){
           partial_product.ubig_value.push_back(carry);

       }
       carry = 0;
       ubig_result = ubig_result + partial_product;
   }

   bool run = true;
   while(run){
        if(ubig_result.ubig_value[ubig_result.ubig_value.size() - 1] == 0){
            if(ubig_result.ubig_value.size() != 1){
                ubig_result.ubig_value.erase(ubig_result.ubig_value.end() - 1);
            }else{
                run = false;
            }
        }else{
                run = false;
        }
   }
   return ubig_result;
}

void ubigint::multiply_by_2() {
    ubigint temp_int("2");
    ubigint res = *this * temp_int;
    (*this).ubig_value = res.ubig_value;
}

void ubigint::divide_by_2() {
   digit_c rem = 0;
   for(int i = ubig_value.size() - 1; i >= 0; --i){
       // divide
       digit_c temp = ((ubig_value[i] + rem) / 2);

       rem = 0;
       if(ubig_value[i] % 2 != 0){
           // its divisible by 2
           // not carrying
           rem = 10;
       }
       ubig_value[i] = temp;
   }
   bool run = true;
   while(run){
      if(ubig_value[ubig_value.size() - 1] == 0){
          if(ubig_value.size() != 1){
              ubig_value.erase(ubig_value.end() - 1);
          }else{
              run = false;
          }
      }else{
           run = false;
      }
   }
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, ubigint divisor) {
   ubigint zero("0");
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2("1");
   ubigint quotient("0");
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   // base case: check if the sizes are the same
   if (this->ubig_value.size() != that.ubig_value.size()){
       // the sizes are not the same so they cannot be equal
       return false;
   }else if(this->ubig_value.size() == that.ubig_value.size()){
      // the sizes are equal
      for (unsigned int i = 0; i < this->ubig_value.size(); ++i){
         if (this->ubig_value[i] != that.ubig_value[i]){
            return false;
         }
      }
      // if it gets out here they are equal
      return true;
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   int check = 0;
   // base case they are equal
   if(this->ubig_value == that.ubig_value){
       return false;
   }else{
      // case: that has more digits
      if(this->ubig_value.size() < that.ubig_value.size()){
          return true;
      }else if(this->ubig_value.size() > that.ubig_value.size()){
          return false;
      }else{
         for (int i = ubig_value.size() - 1; i >= 0; --i) {
            if(this->ubig_value[i] > that.ubig_value[i]){
               return false;
            }else if(this->ubig_value[i] < that.ubig_value[i]){
               return true;
            }
         }
         check = 1;
      }
      return check == 1 ? true : false;
   }
}

ostream& operator<< (ostream& out, const ubigint& that) { 
    //return out << "ubigint(" << that.ubig_value << ")";
    int temp = 0;
    int index = 0;
    for(int i = that.ubig_value.size() - 1; i >= 0; --i){
        temp = that.ubig_value[i];
        out << temp;
        index ++;
        if(index == 69){
            out << "\\" <<endl;
            index = 0;
        }

    }
    return out;
}
