#include <algorithm>
#include <chrono>
#include <iostream>
#include <ostream>
#include <ratio>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <thread>

#define PALETTE                                                                \
  ".'`^,:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$"
#define ESCAPE_RADIUS 4
using LD = long double;

typedef struct {
  LD x;
  LD y;
} complex;

typedef struct {
  LD xA;
  LD xB;
  LD yA;
  LD yB;
} dims;

typedef struct {
  int w;
  int h;
} screen;

screen get_size() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return {w.ws_col, w.ws_row};
}

void sqr_c(complex &c) {
  LD tempx = c.x;
  c = {c.x * c.x - c.y * c.y, 2 * c.y * tempx};
}

LD map(LD iA, LD iB, LD oA, LD oB, LD val) {
  return oA + ((val - iA) / (iB - iA)) * (oB - oA);
}

int calc_mandelbrot(complex c, int maxIter) {
  complex c0 = c;
  int iter = 0;
  while (iter < maxIter && (c.x * c.x + c.y * c.y) < ESCAPE_RADIUS) {
    sqr_c(c);
    c.x += c0.x;
    c.y += c0.y;
    iter++;
  }
  return iter;
}

complex get_center(dims coords) {
  // gets coords height and width
  LD width = coords.xB - coords.xA;
  LD height = coords.yB - coords.yA;

  // returns position of coords center
  return {width / 2 + coords.xA, height / 2 + coords.yA};
}

/*
Pans coords towards a center position dest_position.
step_size ranges from 0-1 and determines how much of the distance shall be
travelled in the direction of dest_position.
*/
void pan(dims &coords_orig, complex dest_position, float step_size = 0.5) {
  complex coords_center = get_center(coords_orig);
  LD width = coords_orig.xB - coords_orig.xA;
  LD height = coords_orig.yB - coords_orig.yA;
  LD aw = std::abs(width);
  LD ah = std::abs(height);

  LD x_step = std::clamp((dest_position.x - coords_center.x) * step_size,
                         -aw / 8, aw / 8);
  LD y_step = std::clamp((dest_position.y - coords_center.y) * step_size,
                         -aw / 8, aw / 8);

  coords_orig.xA += x_step;
  coords_orig.xB += x_step;
  coords_orig.yA += y_step;
  coords_orig.yB += y_step;
}

void print_image(screen &s, dims coords = {-3, 0.47, -1.12, 1.12},
                 int maxIter = 100) {
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  std::string palette = PALETTE;
  std::stringstream out_string;
  for (int j = 0; j < s.h; j++) {
    for (int i = 0; i < s.w; i++) {
      complex c = {map(0, s.w, coords.xA, coords.xB, i),
                   map(0, s.h, coords.yB, coords.yA, j)};
      int output =
          map(0, maxIter, 0, palette.length() - 1, calc_mandelbrot(c, maxIter));
      out_string << palette[(int)output];
    }
  }
  std::cout << out_string.str() << std::flush;
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  std::cout << ((std::chrono::duration<double>)(end - start)).count() << " s" << std::endl;
}

void sub_thread(char* line, int y_pos,  screen *s, dims *lims, int maxIter) {
  std::string palette = PALETTE;
  int pal_len = palette.length();
  LD y_pos_float = map(0,s->h,lims->yA,lims->yB,y_pos);
  for (int i = 0; i < s->w; i++){
    complex c = {map(0,s->w,lims->xA, lims->xB, i),
                 y_pos_float};
    line[i] = palette[(int)map(0,maxIter,0,pal_len-1,calc_mandelbrot(c, maxIter))];
  } 
}

void print_image_thread(screen &s, dims coords = {-3, 0.47, -1.12, 1.12}, int maxIter = 100) {
  
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  char frame[s.h][s.w];
  std::thread* thread_list[s.h];
  for (int i = 0 ; i<s.h ; i++) {
    //starts thread and adds it to list (so we can wait on all to finish)
    thread_list[i] = new std::thread(sub_thread, (char*)(frame[i]), i, &s, &coords, maxIter);
  }
  for (std::thread *t : thread_list) {
    t->join();
  }

  for(char* pix = (char*)frame; *(pix+2); pix++){
    std::cout << *pix ;
  }
  std::cout << std::endl;

  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  std::cout << ((std::chrono::duration<double>)(end - start)).count() << " s\nThreaded" << std::endl;
}


int main(int argc, char *argv[]) {
  screen s = get_size();
  int maxIter = 6000;
  dims init_coords = {-1.5, 0.23, -0.56, 0.56};
  print_image_thread(s, init_coords, maxIter);
  print_image(s, init_coords, maxIter);
  return 0;
}
