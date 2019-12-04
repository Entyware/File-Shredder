#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "global_features.h"

//Define menu bar features
#define OPTIONS                       0         //(1000)

#define MODE                          0         //(1001)
#define SILENT                        1010
#define CONFIRM                       1011
#define VERBOSE                       1012

#define METHOD                        1        //(1002)
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

#define OVERT                         1020
#define COVERT                        1021

#define CLOSE_GUI                     1030

#define ABOUT                         1100

#define DECLINES                      2000
#define ACCCEPTS                      2001

//Define utilities
#define MAIN_ICON                     10001
#define LOGO_BITMAP                   10002

//Declarations of classes and structures
class GUI_Window{
   private:
      HWND hwnd;
      HWND hwnd_about;
      HWND hwnd_register;
      HMENU menu_bar;
      long top;
      long left;
      long width;
      long height;
   public:
      HDC logo_hdc;
      GUI_Window();
      ~GUI_Window();
      HWND create_GUI_window();
      void update_GUI_loci();
      void run_about_popup_window();
      void close_about_popup_window();
      void set_focus_to_sub_menu();
      void run_register_popup_window();
      void close_register_popup_window();
}static *main_window = NULL;

//Registry manipulation
bool check_registration();
void check_registry_keys(unsigned long &, unsigned long &);
char *get_temporary_folder_name();
unsigned long read_display_mode_from_registry();
unsigned long read_display_method_from_registry();
char *create_temporary_folders(HKEY, char *);
void remove_temporary_folders(HKEY, char *);

//Main functions: hybrid
void shred_in_background(int, char *[], unsigned long);
//Main functions: GUI related functions
LRESULT CALLBACK run_main(HWND, UINT, WPARAM, LPARAM);
void display_shred_text(HWND);
LRESULT CALLBACK run_about(HWND, UINT, WPARAM, LPARAM);
void display_about_text(HWND);
LRESULT CALLBACK run_register(HWND, UINT, WPARAM, LPARAM);
void display_register_text(HWND);
LRESULT CALLBACK run_about_scroll(HWND, UINT, WPARAM, LPARAM);
void display_about_legal_text(HWND hwnd);
void adjust_v_scroll_bars(HWND, unsigned long);
unsigned long registration_string(char *);

#endif
