/*   File Shredder deletes files on a hard disk drive in a manner that is
     intended to make any recovery of the data impossible    
     Copyright (C) 2019  Alan Entwistle

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see https://www.gnu.org/licenses.      */

#ifndef VERBOSE_H
#define VERBOSE_H

#pragma pack(1)            // sets alignment to a 1 byte boundary

#include "global_features.h"
#include <shlwapi.h>

//Define gui menu class names: these do not appear anywhere externally
#define MAIN_GUI                      "Main_GUI"
#define ABOUT_GUI                     "About_GUI"
#define ABOUT_SCROLL_GUI              "About_Scroll_GUI"
#define REGISTER_GUI                  "Register_GUI"
#define PROGRESS_GUI                  "Progress_GUI"

// not needed with spinner
//#define MOVE_UPDATE_FREQUENCY         128
//#define REFRESH_WINDOW                2048      //does not overlap with WM_ 

//Define window reporting a queue
#define SHREDDING_WINDOW              1
#define MOVING_WINDOW                 2
#define SHREDDING_HIDDEN_WINDOW       3

#define NO_THREAD_ID                  0
#define NO_THREAD_TYPE                0

class Verbose_Window{
   private:
      HWND hwnd;
      HDC hdc;
      HFONT bold_font;
      HFONT screen_font;
      long top;
      long left;
      long width;
      long height;
      unsigned long window_type;
      unsigned long text_locus;
   public:
      Verbose_Window(){memset(this, 0, sizeof(*this));};
      Verbose_Window(unsigned long);
      ~Verbose_Window();
      HWND create_verbose_window();
      void display_verbose_text();
      void update_verbose_loci();
}static *verbose_window = NULL;       //initialising this as NULL is essential

void move_files_verbose(int, char *[], char *, unsigned long &);
void move_files(int, char *[], char *, unsigned long &, unsigned long);
void run_verbose(char *);
void run_verbose(int, char *[]);
void verbose_shredder(int, char *[], unsigned long);
void hidden_verbose_shredder(int, char *[], unsigned long);
LRESULT CALLBACK run_verbose(HWND, UINT, WPARAM, LPARAM);

//Forward declarations of functions in main_window.cpp
bool run_window();
WNDCLASSEX register_verbose_gui();
LOGFONT *create_screen_font();

void linear_shred(char [], unsigned long long, unsigned long long);
void threaded_shred(char [], unsigned long long, unsigned long long);
void parse_directory(char *, unsigned long, unsigned long);
void parse_directory(char *, char *, unsigned long &, unsigned long);

//verbose thread components
unsigned long WINAPI verbose_thread_function(void *);

//Spinner components
struct Spinner_structure{
   HWND root_hwnd;
   long x_mid; 
   long y_mid;
   long diameter; 
   COLORREF colour;
   HANDLE h_thread;
   Spinner_structure(){memset(this, 0, sizeof(*this));};
   ~Spinner_structure(){};
};
 
void start_spinner(HWND, long, long, long, COLORREF);
void stop_spinner();
unsigned long WINAPI run_spinner_thread(void *);
 
#endif
