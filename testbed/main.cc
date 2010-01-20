/* 
 * Poly2Tri Copyright (c) 2009-2010, Mason Green
 * http://code.google.com/p/poly2tri/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Poly2Tri nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <cstdlib>
#include <GL/glfw.h>
#include <time.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iostream>
using namespace std;

#include "../poly2tri/poly2tri.h"


void Init();
void ShutDown(int return_code);
void MainLoop(const double zoom);
void Draw(const double zoom);
void DrawMap(const double zoom);
void ConstrainedColor(bool constrain);

float rotate_y = 0,
      rotate_z = 0;
const float rotations_per_tick = .2;

/// Constrained triangles
vector<Triangle*> triangles;
/// Triangle map
list<Triangle*> map;

double StringToDouble(const std::string& s) {
  std::istringstream i(s);
  double x;
  if (!(i >> x))
   return 0;
  return x;
}

bool draw_map = true;

int main(int argc, char* argv[]) {
  
  if (argc != 3) {
    cout << "Usage: p2t filename zoom" << endl;
    return 1;
  }
  
	/*
  // initialize random seed: 
  srand ( time(NULL) );
  
  int a = 0;
  int b = 2000;
  
  for(int i = 0; i < num_points; i++) {
    double x = rand() % (b - a - 1) + a + 1;
    double y = rand() % (b - a - 1) + a + 1;
    polyline[i] = Point(x, y);
  }
 
  */
  
  string line;
  ifstream myfile (argv[1]);
  vector<Point*> points;
  if (myfile.is_open()) {
    while (!myfile.eof()) {
      getline (myfile,line);
      if(line.size() == 0) {
        break;
      }
      istringstream iss(line);
      vector<string> tokens;
      copy(istream_iterator<string>(iss), istream_iterator<string>(), 
           back_inserter<vector<string> >(tokens));
      double x = StringToDouble(tokens[0]);
      double y = StringToDouble(tokens[1]);
      points.push_back(new Point(x, y));
    }
    myfile.close();
  } else {
    cout << "File not opened" << endl;
  }
  
  int num_points = points.size();
  cout << "Number of points = " << num_points << endl;
  
	Point** polyline = new Point *[num_points];
  for(int i = 0; i < num_points; i++) {
    polyline[i] = points[i];
  }
  
  Init();
  
  // Perform triangulation
  double init_time = glfwGetTime();
  CDT * cdt = new CDT(polyline, num_points);
  cdt->Triangulate();
  double dt = glfwGetTime() - init_time;
  cout << "Elapsed time (secs) = " << dt << endl;
  
  triangles = cdt->GetTriangles();
  map = cdt->GetMap();
  
  MainLoop(atof(argv[2]));
  
  delete [] polyline;
  ShutDown(0);
  return 0;
}
 
void Init()
{
  const int window_width = 800,
            window_height = 600;
  
  if (glfwInit() != GL_TRUE)
    ShutDown(1);
  // 800 x 600, 16 bit color, no depth, alpha or stencil buffers, windowed
  if (glfwOpenWindow(window_width, window_height, 5, 6, 5, 0, 0, 0, GLFW_WINDOW) != GL_TRUE)
	ShutDown(1);
	
  glfwSetWindowTitle("Poly2Tri - C++");
 
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    
}
 
void ShutDown(int return_code)
{
  glfwTerminate();
  exit(return_code);
}
 
void MainLoop(const double zoom)
{
  // the time of the previous frame
  double old_time = glfwGetTime();
  // this just loops as long as the program runs
  bool running = true;
  
  while(running)
  {
    // calculate time elapsed, and the amount by which stuff rotates
    double current_time = glfwGetTime(),
    delta_rotate = (current_time - old_time) * rotations_per_tick * 360;
    old_time = current_time;
    
    // escape to quit, arrow keys to rotate view
    // Check if ESC key was pressed or window was closed
    running = !glfwGetKey( GLFW_KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED );
      
    if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS)
      rotate_y += delta_rotate;
    if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
      rotate_y -= delta_rotate;
    // z axis always rotates
    rotate_z += delta_rotate;
 
    // Draw the scene
    if(draw_map) {
      DrawMap(zoom);
    } else {
      Draw(zoom);
    }
    
    // swap back and front buffers
    glfwSwapBuffers();
  }
}

void ResetZoom(double zoom, double cx, double cy, double width, double height) {

    double left = -width / zoom;
    double right = width / zoom;
    double bottom = -height / zoom;
    double top = height / zoom;
    
    // Reset viewport
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Reset ortho view
    glOrtho(left, right, bottom, top, 1, -1);
    glTranslatef(-cx, -cy, 0);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DEPTH_TEST);
    glLoadIdentity();
    
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);
    
}
    
void Draw(const double zoom) {

  // reset zoom
  Point center = Point(0, 0);
  
  ResetZoom(zoom, center.x, center.y, 800, 600);
  
  for (int i = 0; i < triangles.size(); i++) {
  
    Triangle& t = *triangles[i];
    Point& a = *t.GetPoint(0);
    Point& b = *t.GetPoint(1);
    Point& c = *t.GetPoint(2);
    
    // Red
    glColor3f(1, 0, 0);
    
    glBegin(GL_LINE_LOOP);                      
      glVertex2f(a.x, a.y);                         
      glVertex2f(b.x, b.y);                          
      glVertex2f(c.x, c.y);                           
    glEnd();

  }
  
}

void DrawMap(const double zoom) {

  // reset zoom
  Point center = Point(0, 0);
  
  ResetZoom(zoom, center.x, center.y, 800, 600);
  
  list<Triangle*>::iterator it; 
  for (it = map.begin(); it != map.end(); it++) {
  
    Triangle& t = **it;
    Point& a = *t.GetPoint(0);
    Point& b = *t.GetPoint(1);
    Point& c = *t.GetPoint(2);
    
    ConstrainedColor(t.constrained_edge[2]); 
    glBegin(GL_LINES);  
      glVertex2f(a.x, a.y); 
      glVertex2f(b.x, b.y); 
    glEnd( );

    ConstrainedColor(t.constrained_edge[0]); 
    glBegin(GL_LINES);
      glVertex2f(b.x, b.y); 
      glVertex2f(c.x, c.y);
    glEnd( );

    ConstrainedColor(t.constrained_edge[1]); 
    glBegin(GL_LINES);
      glVertex2f(c.x, c.y); 
      glVertex2f(a.x, a.y); 
    glEnd( );
  
  }
  
}

void ConstrainedColor(bool constrain) {
  if(constrain) {
    // Green
    glColor3f(0, 1, 0);
  } else {
    // Red
    glColor3f(1, 0, 0);
  }
}