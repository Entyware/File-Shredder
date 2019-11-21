#ifndef GLOBAL_FEATURES_H
#define GLOBAL_FEATURES_H

#define WINVER 0x0500       //defines windows version as Windows 2000 or later
#include <windows.h>        //a library required for windows includes cstdlib

//Global colours
#define BACKGROUND_COLOUR             RGB(199, 223, 255)
#define TEXT_COLOUR                   RGB(0, 0, 143)
#define BACKGROUND_WHITE              RGB(255, 255, 255)
#define TEXT_BLACK                    RGB(0, 0, 0)

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
