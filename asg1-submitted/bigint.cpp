// $Id: bigint.cpp,v 1.76 2016-06-14 16:34:24-07 - - $

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
  if (is_negative and that.is_negative) { //both negative
    return {uvalue + that.uvalue, true};
  }
  if (is_negative or that.is_negative) {  //one of them negative
    if (is_negative) {
      if (uvalue <= that.uvalue)  
        return that.uvalue - uvalue;
      else 
        return {uvalue - that.uvalue, true};
    } else {
      if (uvalue >= that.uvalue) 
        return uvalue - that.uvalue;
      else 
        return {that.uvalue - uvalue, true};
    }
  }
  return uvalue + that.uvalue;  //both non-negative
}

bigint bigint::operator- (const bigint& that) const {
   if (!is_negative and !that.is_negative) {  //both non-negative
    if (uvalue >= that.uvalue) 
      return uvalue - that.uvalue;
    else 
      return {uvalue - that.uvalue, true};
   } else if (is_negative and that.is_negative) { //both negative
    if (uvalue < that.uvalue) 
      return {uvalue - that.uvalue, true};
    else 
      return uvalue - that.uvalue;
   } else { //one of them negative
    if (is_negative) 
      return {uvalue + that.uvalue, true};
    else
      return uvalue + that.uvalue;
   }
}

bigint bigint::operator* (const bigint& that) const {
  if (is_negative and that.is_negative)  //both non-negative
    return uvalue * that.uvalue;
  if (is_negative or that.is_negative) 
    return {uvalue * that.uvalue, true};
  return uvalue * that.uvalue;
}

bigint bigint::operator/ (const bigint& that) const {
  if (is_negative and that.is_negative)  //both non-negative
    return uvalue / that.uvalue;
  if (is_negative or that.is_negative) 
    return {uvalue / that.uvalue, true};
  return uvalue / that.uvalue;
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
   return out << (that.is_negative ? "-" : "") << that.uvalue;
}

