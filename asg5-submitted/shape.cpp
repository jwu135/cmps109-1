// $Id: shape.cpp,v 1.2 2019-02-28 15:24:20-08 - - $

#include <typeinfo>
#include <unordered_map>
#include <math.h>
using namespace std;

#include "shape.h"
#include "util.h"
#include "graphics.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font_, const string& textdata_):
      glut_bitmap_font(glut_bitmap_font_), textdata(textdata_) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
            polygon({
               {-width/2,  height/2}, 
               { width/2,  height/2},
               { width/2, -height/2}, 
               {-width/2, -height/2}
            }) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

diamond::diamond (const GLfloat width, const GLfloat height):
            polygon({
               {0.0, height/2},
               {-width/2, 0.0},
               {0.0, -height/2},
               { width/2, 0.0}
            }) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

triangle::triangle (const vertex_list& vl): polygon(vl) {
   DEBUGF ('c', this);
}

equilateral::equilateral (const GLfloat width): 
            triangle({
               {0.0, static_cast<GLfloat>(sqrt(3)*width/3)},
               {-width/2, static_cast<GLfloat>(-sqrt(3)*width/6)},
               {width/2, static_cast<GLfloat>(-sqrt(3)*width/6)}
            }) {
   DEBUGF ('c', this << "(" << width << ")");
}

void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glColor3ubv(color.ubvec);
   glRasterPos2f(center.xpos, center.ypos);
   glutBitmapString(glut_bitmap_font, 
           reinterpret_cast<const GLubyte*>(textdata.c_str()));

   if (window::selected) {
      glLineWidth(window::border_width);
      glBegin (GL_LINE_LOOP);
      glEnable (GL_LINE_SMOOTH);
      GLfloat width = glutBitmapLength(glut_bitmap_font, 
         reinterpret_cast<const GLubyte*>(textdata.c_str()));
      GLfloat height = glutBitmapHeight(glut_bitmap_font);
      glColor3ubv(rgbcolor(window::border_color).ubvec);
      vertex_list vl = {{-width/2,  height/2},
                        { width/2,  height/2},
                        { width/2, -height/2},
                        {-width/2, -height/2}};
      for (size_t i = 0; i < vl.size(); ++i) {
         glVertex2i(vl[i].xpos + center.xpos + width/2, 
            vl[i].ypos + center.ypos + height/3);
      }
      glEnd(); glFlush();
   } 
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glBegin(GL_POLYGON);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);
   float ii = 2 * 3.1415926 / 32;
   for (float i = 0; i < 2 * 3.1415926; i += ii) {
      float x = (dimension.xpos * cos (i))/2 + center.xpos;
      float y = (dimension.ypos * sin (i))/2 + center.ypos;
      glVertex2f (x, y);
   }
   glEnd(); glFlush();

   if (window::selected) {
      glLineWidth(window::border_width);
      glBegin (GL_LINE_LOOP);
      glEnable (GL_LINE_SMOOTH);
      glColor3ubv(rgbcolor(window::border_color).ubvec);
      for (float i = 0; i < 2 * 3.1415926; i += ii) {
         float x = (dimension.xpos * cos (i))/2 + center.xpos;
         float y = (dimension.ypos * sin (i))/2 + center.ypos;
         glVertex2i (x, y);
      }
      glEnd(); glFlush();
   }
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glBegin(GL_POLYGON);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv (color.ubvec);
   for (size_t i = 0; i < vertices.size(); ++i) {
      glVertex2f(vertices[i].xpos + center.xpos, 
         vertices[i].ypos + center.ypos);
   }
   glEnd(); glFlush();

   if (window::selected) {
      glLineWidth(window::border_width);
      glBegin (GL_LINE_LOOP);
      glEnable (GL_LINE_SMOOTH);
      glColor3ubv(rgbcolor(window::border_color).ubvec);
      for (size_t i = 0; i < vertices.size(); ++i) {
         glVertex2i(vertices[i].xpos + center.xpos, 
            vertices[i].ypos + center.ypos);
      }
      glEnd(); glFlush();
   }
}


/*
   useless functions
*/
void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

