// $Id: shape.cpp,v 1.1 2015-07-16 16:47:51-07 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 3/5/2018
*/
#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "graphics.h"
#include "util.h"

GLubyte borderColor[3];
GLfloat line_width = 1;
text* zero_to_nine[10];

static unordered_map<void *, string> fontname{
    {GLUT_BITMAP_8_BY_13, "Fixed-8x13"},
    {GLUT_BITMAP_9_BY_15, "Fixed-9x15"},
    {GLUT_BITMAP_HELVETICA_10, "Helvetica-10"},
    {GLUT_BITMAP_HELVETICA_12, "Helvetica-12"},
    {GLUT_BITMAP_HELVETICA_18, "Helvetica-18"},
    {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
    {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

unordered_map<string, void *> fontcode{
    {"Fixed-8x13", GLUT_BITMAP_8_BY_13},
    {"Fixed-9x15", GLUT_BITMAP_9_BY_15},
    {"Helvetica-10", GLUT_BITMAP_HELVETICA_10},
    {"Helvetica-12", GLUT_BITMAP_HELVETICA_12},
    {"Helvetica-18", GLUT_BITMAP_HELVETICA_18},
    {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
    {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream &operator<<(ostream &out, const vertex &where)
{
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape()
{
   DEBUGF('c', this);
}

text::text(void *glut_bitmap_font, const string &textdata) : glut_bitmap_font(glut_bitmap_font), textdata(textdata)
{
   DEBUGF('c', this);
}

ellipse::ellipse(GLfloat width, GLfloat height) : dimension({width, height})
{
   DEBUGF('c', this);
}

circle::circle(GLfloat diameter) : ellipse(diameter, diameter)
{
   DEBUGF('c', this);
}

polygon::polygon(const vertex_list &vertices) : vertices(vertices)
{
   DEBUGF('c', this);
}

rectangle::rectangle(GLfloat width, GLfloat height) : polygon({{-width / 2, height / 2}, {width / 2, height / 2}, {width / 2, -height / 2}, {-width / 2, -height / 2}})
{
   DEBUGF('c', this << "(" << width << "," << height << ")");
}

diamond::diamond(GLfloat width, GLfloat height) : polygon({{-width / 2, 0}, {0, height / 2}, {width / 2, 0}, {0, -height / 2}})
{
   DEBUGF('c', this << "(" << width << "," << height << ")");
}

triangle::triangle(const vertex_list &v) : polygon(v)
{
}

equilateral::equilateral(GLfloat width) : triangle({{-width / 2, -width / 2}, {0, width / 2}, {width / 2, -width / 2}})
{
   DEBUGF('c', this);
}

square::square(GLfloat width) : rectangle(width, width)
{
   DEBUGF('c', this);
}

void text::draw(const vertex &center, const rgbcolor &color) const
{
   DEBUGF('d', this << "(" << center << "," << color << ")");

   glColor3ubv(color.ubvec);
   glRasterPos2f(center.xpos, center.ypos);
   auto u_str = reinterpret_cast<const GLubyte *>(this->textdata.c_str());
   glutBitmapString(this->glut_bitmap_font, u_str);


   if (this->my_ID == window::selected)
   {
       // draw a rectangle
      string s = this->textdata;
      auto x = reinterpret_cast<const GLubyte*> (s.c_str());
      size_t width = glutBitmapLength (this->glut_bitmap_font, x);
      int height = int(glutBitmapHeight (this->glut_bitmap_font));
      rectangle r(width, height);


      vertex_list vertices;
      vertex v;
      v.xpos = 0;
      v.ypos = -height / 2;
      vertices.push_back(v);
      v.xpos = width;
      v.ypos = -height / 2;
      vertices.push_back(v);
      v.xpos = width;
      v.ypos = height;
      vertices.push_back(v);
      v.xpos = 0;
      v.ypos = height;
      vertices.push_back(v);

      // border
      glLineWidth(line_width);
      glBegin(GL_LINE_LOOP);
      glEnable(GL_LINE_SMOOTH);
      glColor3ubv(borderColor);
      for (unsigned int i = 0; i < 4; ++i)
      {
         glVertex2f(vertices[i].xpos + center.xpos, vertices[i].ypos + center.ypos);
      }

      glEnd();
   
   }
}

void ellipse::draw(const vertex &center, const rgbcolor &color) const
{
   DEBUGF('d', this << "(" << center << "," << color << ")");
   // draw the ellipse to the screen?
   // the dimensions are saved as part of the object

   glBegin(GL_POLYGON);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);

   float scale = 1.0;
   const float delta = 2 * M_PI / 32;
   float width = this->dimension.xpos / 3 * scale;
   float height = this->dimension.ypos / 3 * scale;
   for (float theta = 0; theta < 2 * M_PI; theta += delta)
   {
      float xpos = width * cos(theta) + center.xpos;
      float ypos = height * sin(theta) + center.ypos;
      glVertex2f(xpos, ypos);
   }
   glEnd();

   if (this->my_ID == window::selected)
   {
      glLineWidth(line_width);
      glBegin(GL_LINE_LOOP);
      glEnable(GL_LINE_SMOOTH);
      glColor3ubv(borderColor);
      float scale = 1.0;
      const float delta = 2 * M_PI / 32;
      float width = this->dimension.xpos / 3 * scale;
      float height = this->dimension.ypos / 3 * scale;
      for (float theta = 0; theta < 2 * M_PI; theta += delta)
      {
         float xpos = width * cos(theta) + center.xpos;
         float ypos = height * sin(theta) + center.ypos;
         glVertex2f(xpos, ypos);
      }
      glEnd();
   }
   if (this->my_ID < 10)
   {
      rgbcolor gbr;
      gbr.ubvec[0] = 255;
      gbr.ubvec[1] = 255;
      gbr.ubvec[2] = 255;

      string s = "0";
      auto x = reinterpret_cast<const GLubyte*> (s.c_str());
      size_t width = glutBitmapLength (fontcode.find("Times-Roman-24")->second, x);
      size_t height = glutBitmapHeight (fontcode.find("Times-Roman-24")->second);

      vertex temp;
      temp.xpos = center.xpos - width / 2;
      temp.ypos = center.ypos - height / 2;

      zero_to_nine[this->my_ID]->draw(temp, gbr);
   }
}

void polygon::draw(const vertex &center, const rgbcolor &color) const
{
   DEBUGF('d', this << "(" << center << "," << color << ")");

   glBegin(GL_POLYGON);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);

   for (unsigned int i = 0; i < this->vertices.size(); ++i)
   {
      glVertex2f(this->vertices[i].xpos + center.xpos, this->vertices[i].ypos + center.ypos);
   }

   glEnd();
   if (this->my_ID == window::selected)
   {
      // border
      glLineWidth(line_width);
      glBegin(GL_LINE_LOOP);
      glEnable(GL_LINE_SMOOTH);
      glColor3ubv(borderColor);
      for (unsigned int i = 0; i < this->vertices.size(); ++i)
      {
         glVertex2f(this->vertices[i].xpos + center.xpos, this->vertices[i].ypos + center.ypos);
      }

      glEnd();
   }
   if (this->my_ID < 10)
   {
      rgbcolor gbr;
      gbr.ubvec[0] = 255;
      gbr.ubvec[1] = 255;
      gbr.ubvec[2] = 255;

      string s = "0";
      auto x = reinterpret_cast<const GLubyte*> (s.c_str());
      size_t width = glutBitmapLength (fontcode.find("Times-Roman-24")->second, x);
      size_t height = glutBitmapHeight (fontcode.find("Times-Roman-24")->second);

      vertex temp;
      temp.xpos = center.xpos - width / 2;
      temp.ypos = center.ypos - height / 2;

      zero_to_nine[this->my_ID]->draw(temp, gbr);
   }
}

void shape::show(ostream &out) const
{
   out << this << "->" << demangle(*this) << ": ";
}

void text::show(ostream &out) const
{
   shape::show(out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show(ostream &out) const
{
   shape::show(out);
   out << "{" << dimension << "}";
}

void polygon::show(ostream &out) const
{
   shape::show(out);
   out << "{" << vertices << "}";
}

ostream &operator<<(ostream &out, const shape &obj)
{
   obj.show(out);
   return out;
}
