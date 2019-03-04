// $Id: ubigint.cpp,v 1.14 2016-06-23 17:21:26-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <algorithm>
#include <climits>
using namespace std;

#include "ubigint.h"
#include "debug.h"
#include "util.h"


ubigint::ubigint (unsigned long that) {
   DEBUGF ('~', this << " -> " << uvalue)

   if (that > ULONG_MAX) {
      throw ydc_exn("reached maximum");
   }

   if (that > 0) {
      ubig_value.push_back(that%10);
      that /= 10;
   }
}

ubigint::ubigint (const string& that) {

   for (const auto& c: that) {
      ubig_value.push_back(c - '0');
   }
   reverse(ubig_value.begin(), ubig_value.end());
}

ubigint ubigint::operator+ (const ubigint& that) const {

   ubigint real_sum;
   ubigvalue_t bigger_value, smaller_value;
   int sum = 0, left_size, right_size, smaller_size, bigger_size;
   int i;
   bool carry = false;
   left_size = ubig_value.size(); 
   right_size = that.ubig_value.size();
   
   if (left_size > right_size) {
      smaller_size = right_size;
      bigger_size = left_size;
      bigger_value = ubig_value;
      smaller_value = that.ubig_value;
   } else {
      smaller_size = left_size;
      bigger_size = right_size;
      bigger_value = that.ubig_value;
      smaller_value = ubig_value;
   }

   for (i = 0; i < smaller_size; ++i) {   
      sum = (smaller_value[i]) + (bigger_value[i]);
      if (carry) {
         sum = sum + 1;
      }
      if (sum >= 10) {
         sum = sum - 10;
         carry = true;
      } else {
         carry = false;
      }

      real_sum.ubig_value.push_back(sum);
   }

   while (i < bigger_size) {
      sum = bigger_value[i];
      if (carry){
         sum = sum + 1;
      }
      if (sum >= 10) {
         sum = sum - 10;
         carry = true;
      } else {
         carry = false;
      }
      ++i;
      real_sum.ubig_value.push_back(sum);
   }

   if (carry)  //the overflow
      real_sum.ubig_value.push_back(1);

   return real_sum;
}

ubigint ubigint::operator- (const ubigint& that) const {

   ubigint real_diff;
   ubigvalue_t bigger_value, smaller_value, result_value;
   int diff = 0, smaller_size, bigger_size;
   int i;
   bool borrow = false;

   if (this->operator<(that)) {
      bigger_value = that.ubig_value;
      bigger_size = that.ubig_value.size();
      smaller_value = ubig_value;
      smaller_size = ubig_value.size();
   } else {
      bigger_value = ubig_value;
      bigger_size = ubig_value.size();
      smaller_value = that.ubig_value;
      smaller_size = that.ubig_value.size();
   }

   for (i = 0; i < smaller_size; ++i) {
      diff = (bigger_value[i]) - (smaller_value[i]);
      if (borrow) {
         diff = diff - 1;
      }
      if (diff < 0) {
         diff = diff + 10;
         borrow = true;
      } else {
         borrow = false;
      }

      real_diff.ubig_value.push_back(diff);
   }

   while (i < bigger_size) {
      diff = bigger_value[i];
      if (borrow){
         diff = diff - 1;
      }
      if (diff < 0) {
         diff = diff + 10;
         borrow = true;
      } else {
         borrow = false;
      }
      ++i;
      real_diff.ubig_value.push_back(diff);
   }

   while (real_diff.ubig_value.size() > 1) { //pop back zeros
      if (real_diff.ubig_value.back() != 0) 
         break;
      else 
         real_diff.ubig_value.pop_back();
   }

   return real_diff;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint real_pro {"0"};
   int fac1, fac2, left_size, right_size;
   int i, j;
   int digit = 0;
   left_size = ubig_value.size();
   right_size = that.ubig_value.size();
   real_pro.ubig_value.assign(left_size+right_size+1, 0);

   for (i = 0; i < left_size; ++i) {
      for (j = 0; j < right_size; ++j) {
         fac1 = ubig_value[i];
         fac2 = that.ubig_value[j];
         real_pro.ubig_value[i+j] += fac1 * fac2;
         if (real_pro.ubig_value[i+j] >= 10) {
            digit = real_pro.ubig_value[i+j];
            real_pro.ubig_value[i+j+1] += digit/10;
            real_pro.ubig_value[i+j] = digit%10;
         }
      }
   }

   while (real_pro.ubig_value.size() > 1) { //pop back zeros
      if (real_pro.ubig_value.back() != 0) 
         break;
      else 
         real_pro.ubig_value.pop_back();
   }

   return real_pro;
}

void ubigint::multiply_by_2() {  
   *this = *this + *this;
}

void ubigint::divide_by_2() {
   ubigint real_quo {"0"};
   int index = ubig_value.size() - 1;
   int quo, rem = 0, curr;
   bool rem_exist = 0;

   while (index >= 0) {
      curr = ubig_value[index];

      if (rem_exist) {
         curr = curr + rem*10;
      }
      rem = 0;
      quo = curr / 2;
      rem = curr % 2;
      rem_exist = rem;

      real_quo.ubig_value.push_back(quo);
      --index;
   }

   reverse(real_quo.ubig_value.begin(), real_quo.ubig_value.end());

   while (real_quo.ubig_value.size() > 1) { //pop back zeros
      if (real_quo.ubig_value.back() != 0) 
         break;
      else 
         real_quo.ubig_value.pop_back();
   }

   ubig_value = real_quo.ubig_value;
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, ubigint divisor) {
   // Note: divisor is modified so pass by value (copy).
   ubigint zero {"0"};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {"1"};
   ubigint quotient {"0"};
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
   return !(*this<that or that<*this); 
}

bool ubigint::operator< (const ubigint& that) const {
   int index;

   if (ubig_value.size() < that.ubig_value.size())
      return true;
   else if (ubig_value.size() == that.ubig_value.size()) {
      index = that.ubig_value.size() - 1;
      while (index >= 0) {
         if(ubig_value[index] < that.ubig_value[index])
            return true;
         else if (ubig_value[index] == that.ubig_value[index])
            --index;
         else
            break;
      }
   }

   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) {
   int size = that.ubig_value.size();
   int bits = 69;
   string result; 
   for (const char& c: that.ubig_value) {
      result.push_back(c + '0');
   }
   reverse(result.begin(), result.end());

   while (bits < size) {
      result.insert(bits, "\\");
      result.insert(bits+1, "\n");
      bits += 71;
      size += 2;
   }

   return out << result;
}

