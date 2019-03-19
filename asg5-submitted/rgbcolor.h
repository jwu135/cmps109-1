// $Id: rgbcolor.h,v 1.2 2019-02-28 15:24:20-08 - - $

#ifndef __RGBCOLOR_H__
#define __RGBCOLOR_H__

#include <string>
#include <unordered_map>
using namespace std;

#include <GL/freeglut.h>

struct rgbcolor {
   union {
      GLubyte ubvec[3];
      struct {
         GLubyte red {};
         GLubyte green {};
         GLubyte blue {};
      } rgb {};
   };
   explicit rgbcolor() = default;
   explicit rgbcolor (GLubyte red, GLubyte green, GLubyte blue):
               rgb ({red, green,blue}) {};
   explicit rgbcolor (const string&);
   const GLubyte* ubvec3() { return ubvec; }
   operator string() const;
};

ostream& operator<< (ostream&, const rgbcolor&);

extern const std::unordered_map<std::string,rgbcolor> color_names;

#endif

