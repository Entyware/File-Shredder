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

#ifndef GLOBAL_FEATURES_H
#define GLOBAL_FEATURES_H

#define WINVER 0x0500       //defines windows version as Windows 2000 or later
#include <windows.h>        //a library required for windows includes cstdlib

//Global colours
#define BACKGROUND_COLOUR             RGB(199, 223, 255)
#define TEXT_COLOUR                   RGB(0, 0, 143)
#define BACKGROUND_WHITE              RGB(255, 255, 255)
#define TEXT_BLACK                    RGB(0, 0, 0)

#define TERMS_LENGTH                  9336

#define SCREEN_FONT_HEIGHT            12

//main window position default values
#define MAIN_WINDOW_TOP               100
#define MAIN_WINDOW_LEFT              200
#define MAIN_WINDOW_HEIGHT            250
#define MAIN_WINDOW_WIDTH             400
#define SPINNER_WIDTH                 48

//verbose GUI window position default values
#define VERBOSE_TOP                   110
#define VERBOSE_LEFT                  220
#define VERBOSE_HEIGHT                115
#define VERBOSE_WIDTH                 500

#define DEFAULT_MODE                  display_mode = VERBOSE
#define DEFAULT_METHOD                display_method = OVERT

#define FILE_SIZE_BOUNDARY            300
#define MAXIMUM_BUFFER_BITS           29        //limit for 32 bit architecture

char *fixed_length_numeral(unsigned long, unsigned short);
char *null_string(char *);
unsigned long null_string(char *, char*);
unsigned long null_string_length(char *);
char *end_string(char *);
unsigned long end_string(char *, char*);
unsigned long end_string_length(char *);
void unsigned_2_numeral(char *&, unsigned long long);
void copy_string(char *&string_out, char *string_in);

#endif
