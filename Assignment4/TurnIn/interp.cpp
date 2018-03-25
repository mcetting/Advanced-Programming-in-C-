// $Id: interp.cpp,v 1.2 2015-07-16 16:57:30-07 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 3/5/2018
*/
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

unordered_map<string, interpreter::interpreterfn>
    interpreter::interp_map{
        {"define", &interpreter::do_define},
        {"draw", &interpreter::do_draw},
        {"border", &interpreter::do_border},
        {"moveby", &interpreter::do_move},
    };

unordered_map<string, interpreter::factoryfn>
    interpreter::factory_map{
        {"text", &interpreter::make_text},
        {"ellipse", &interpreter::make_ellipse},
        {"circle", &interpreter::make_circle},
        {"polygon", &interpreter::make_polygon},
        {"rectangle", &interpreter::make_rectangle},
        {"square", &interpreter::make_square},
        {"triangle", &interpreter::make_triangle},
        {"equilateral", &interpreter::make_equilateral},
        {"diamond", &interpreter::make_diamond},
    };

interpreter::shape_map interpreter::objmap;

interpreter::~interpreter()
{
   for (const auto &itor : objmap)
   {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret(const parameters &params)
{ // array of strings
   DEBUGF('i', params);
   param begin = params.cbegin();        // gets first part of array in memory
   string command = *begin;              // gets value at that location
   auto itor = interp_map.find(command); // searches the map for that string
   if (itor == interp_map.end())
      throw runtime_error("syntax error"); // reach the end
   interpreterfn func = itor->second;
   func(++begin, params.cend());
}

void interpreter::do_define(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string name = *begin;
   objmap.emplace(name, make_shape(++begin, end));
}

void interpreter::do_draw(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   if (end - begin != 4)
      throw runtime_error("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find(name);
   if (itor == objmap.end())
   {
      throw runtime_error(name + ": no such shape");
   }
   rgbcolor color{begin[0]};
   vertex where{from_string<GLfloat>(begin[2]),
                from_string<GLfloat>(begin[3])};
   //itor->second->draw (where, color);
   itor->second->my_ID = window::return_objects_size();
   object obj(itor->second, where, color);

   window::push_back(obj);
}

void interpreter::do_border(param begin, param end)
{
   (void)end;
   // color then thickness
   borderColor[0] = rgbcolor(*begin).ubvec[0];
   borderColor[1] = rgbcolor(*begin).ubvec[1];
   borderColor[2] = rgbcolor(*begin).ubvec[2];
   ++begin;
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   line_width = p1;
}
void interpreter::do_move(param begin, param end)
{
   (void)end;
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   move_speed = int(p1);
}

shape_ptr interpreter::make_shape(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end())
   {
      throw runtime_error(type + ": no such shape"); //  works
   }
   factoryfn func = itor->second;
   return func(begin, end);
}

shape_ptr interpreter::make_text(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string font_name = *begin;
   ++begin;
   string data = "";
   while (begin != end)
   {
      data += *begin + " ";
      ++begin;
   }

   if (fontcode.find(font_name) == fontcode.end())
   {
      throw runtime_error("No font specified.");
   }
   // go through and
   return make_shared<text>(fontcode.find(font_name)->second, data);
}

shape_ptr interpreter::make_ellipse(param begin, param end)
{
   DEBUGF('f', range(begin, end));

   // convert the dimensions from string to float
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   ++begin;
   string parameter2(*begin);
   string::size_type sz2;
   float p2 = std::stof(parameter2, &sz2);

   return make_shared<ellipse>(p1, p2);
}

shape_ptr interpreter::make_circle(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   return make_shared<circle>(p1);
}
float getAverageX(vertex_list v)
{
   // add up all the first verticies then divide
   float x = 0;
   for (unsigned int i = 0; i < v.size(); i++)
   {
      x += v[i].xpos;
   }
   x /= v.size();
   return x;
}
float getAverageY(vertex_list v)
{
   // add up all the second verticies then divide
   float y = 0;
   for (unsigned int i = 0; i < v.size(); i++)
   {
      y += v[i].ypos;
   }
   y /= v.size();
   return y;
}
shape_ptr interpreter::make_polygon(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   vertex_list vList;
   while (begin != end)
   {
      string parameter1(*begin);
      string::size_type sz;
      float p1 = std::stof(parameter1, &sz);
      ++begin;
      if (begin == end)
      {
         throw runtime_error("Inproper Verticies.");
      }
      string parameter2(*begin);
      string::size_type sz2;
      float p2 = std::stof(parameter2, &sz2);

      vertex v;
      v.xpos = p1;
      v.ypos = p2;
      vList.push_back(v);
      ++begin;
   }
   // normalize it
   float averageX = getAverageX(vList);
   float averageY = getAverageY(vList);

   for (unsigned int i = 0; i < vList.size(); ++i)
   {
      vList[i].xpos -= averageX;
      vList[i].ypos -= averageY;
   }

   return make_shared<polygon>(vList);
}

shape_ptr interpreter::make_rectangle(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   ++begin;
   string parameter2(*begin);
   string::size_type sz2;
   float p2 = std::stof(parameter2, &sz2);

   return make_shared<rectangle>(p1, p2);
}

shape_ptr interpreter::make_diamond(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   ++begin;
   string parameter2(*begin);
   string::size_type sz2;
   float p2 = std::stof(parameter2, &sz2);

   return make_shared<diamond>(p1, p2);
}
shape_ptr interpreter::make_square(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   return make_shared<square>(p1);
}

shape_ptr interpreter::make_triangle(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   vertex_list vList;
   int i = 0;
   while (begin != end)
   {
      if (i == 3)
      {
         throw runtime_error("Inproper Syntax.");
      }
      i++;
      string parameter1(*begin);
      string::size_type sz;
      float p1 = std::stof(parameter1, &sz);
      ++begin;
      if (begin == end)
      {
         throw runtime_error("Inproper Verticies.");
      }
      string parameter2(*begin);
      string::size_type sz2;
      float p2 = std::stof(parameter2, &sz2);

      vertex v;
      v.xpos = p1;
      v.ypos = p2;
      vList.push_back(v);
      ++begin;
   }
   if (i < 3)
   {
      throw runtime_error("Inproper Syntax.");
   }
   // normalize it
   float averageX = getAverageX(vList);
   float averageY = getAverageY(vList);

   for (unsigned int i = 0; i < vList.size(); ++i)
   {
      vList[i].xpos -= averageX;
      vList[i].ypos -= averageY;
   }

   return make_shared<triangle>(vList);
}

shape_ptr interpreter::make_equilateral(param begin, param end)
{
   DEBUGF('f', range(begin, end));
   string parameter1(*begin);
   string::size_type sz;
   float p1 = std::stof(parameter1, &sz);
   return make_shared<equilateral>(p1);
}