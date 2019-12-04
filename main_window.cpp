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

#include "main_window.h"
#include "verbose.h"
#include "shredder.h"
#include "text.h"
#include "gnu_license.h"
#include "menubar_controls.h"

int main (int arguments, char *argument_strings[])
{/*Only allow a single instance of this program to run to prevent 
                                         instances competing for a hard drive */
 /* general release version */ 
 if(OpenMutex(MUTEX_ALL_ACCESS, 0, FILE_SHREDDER_COPYRIGHT))
    return 0;
 HANDLE h_mutex = CreateMutex(0, 0, FILE_SHREDDER_COPYRIGHT);

 /*Developemental version
 if(OpenMutex(MUTEX_ALL_ACCESS, 0, FILE_SHREDDER_VERSION))
    return 0;
 HANDLE h_mutex = CreateMutex(0, 0, FILE_SHREDDER_VERSION);
*/

 //run the program in the background
 SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
 //check the program is registered and if not register it
 if(!check_registration())
    return 0;
 //select appropriate mode of shredding or open main GUI window
 if(arguments > 1)                    //Program required to shred files
 {  
    if(!strcmp(argument_strings[1], COMPLETE_SHREDDING))
    {  /* continues a hidden instance of shredding that was terminated before 
                                               all the files had been deleted */
       char *folder_name = get_temporary_folder_name();
       parse_directory(folder_name, NO_THREAD_ID, NO_THREAD_TYPE);  //shred file
       //remove the registry key launching the program on startup
       HKEY hKey_startup;
       RegOpenKeyEx(HKEY_CURRENT_USER, REG_USER_STARTUP, 0, 
                                           KEY_READ | KEY_WRITE, &hKey_startup);
       RegDeleteValue(hKey_startup, REG_PROGRAM_NAME);
       RegCloseKey(hKey_startup);
       //delete directory path of temporary folder in registry
       HKEY hKey;           //Find the folder to delete
       RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 
                                                   KEY_READ | KEY_WRITE, &hKey);
       RegDeleteValue(hKey, REG_TEMP_FOLDER_NAME);
       remove_temporary_folders(hKey, folder_name);       //clear folders
       RegCloseKey(hKey);
       delete [] folder_name;
       return 0;
    }
    //checks the settings in the registry are valid and sets the display means
    unsigned long display_mode, display_method;
    check_registry_keys(display_mode, display_method); 
    if(display_mode == SILENT)        //In silent mode shred files immediately
    {
       if(display_method == COVERT)   //Hide the files before shredding started
          shred_in_background(arguments, argument_strings, display_mode);
       else                           //otherwise shred the files in situ
          shredder(arguments, argument_strings); 
       return 0;
    }                                 //else check that files are to be shredded
    else if(MessageBox(NULL, 
           SHREDDING_MESSAGE, SHREDDING_TITLE,
            MB_ICONWARNING | MB_SYSTEMMODAL | MB_DEFBUTTON2 | MB_YESNO) == IDNO)
       return 0;                      //confirmation rejected, do nothing
    if(display_method == COVERT)      //Hide the files before shredding started
       shred_in_background(arguments, argument_strings, display_mode);
    else if(display_mode == CONFIRM)  //Shred the files silently
       shredder(arguments, argument_strings);          //shred files
    //this implies that VERBOSE is the default mode
    else                              //supply feedback to user
       run_verbose(arguments, argument_strings);       //shred files
 }
 else                                 //Open main GUI to adjust settings
 {
    main_window = new GUI_Window();   //intialise main window
    HWND hwnd = main_window->create_GUI_window();    //create main window GUI
    run_window();
    //housekeeping
    delete main_window;
    main_window = NULL;
 }
 //single instance housekeeping
 ReleaseMutex(h_mutex);
 CloseHandle(h_mutex);
 return 0;
};

/******************************************************************************/
/*********** functions declared in main_window.h (registry actions) ***********/ 
/******************************************************************************/
bool check_registration()
{/* implement registration the first time the program is used */
 unsigned long value_size, key_status;
 HKEY hKey;
 if(!RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 
                                                   KEY_READ | KEY_WRITE, &hKey))
 {  //test for registration
    key_status = RegQueryValueEx(hKey, REG_USER_AGREEMENT, 0,  NULL, NULL, 
                                                                   &value_size);
    if(key_status == ERROR_SUCCESS)   //program is registered
    {
       RegCloseKey(hKey);
       return true;                   //so continue
    }
 }
 //Otherwise register the users instance of this program
 main_window = new GUI_Window();                //intialise main window
 HWND hwnd = main_window->create_GUI_window();  //create main window GUI
 main_window->run_register_popup_window();      //create registraion sub window
 bool accepted = run_window();                  // wait for user to accept or 
 if(accepted)                             /* decline the terms and conditions */
 {  //if terms and conditions accepted register the instance
    if(RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 
                                                   KEY_READ | KEY_WRITE, &hKey))
    { //if registry key does not exist create a new key
       RegCreateKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 0, 
                           REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0, &hKey, 0);
       RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 
                                                   KEY_READ | KEY_WRITE, &hKey);
    }
    //set date and time of registration and record this in the registry key
    char reg_string[50];
    unsigned long string_length = registration_string(reg_string);
    RegSetValueEx(hKey, REG_USER_AGREEMENT, 0, REG_SZ, 
                                             (LPBYTE)reg_string, string_length);
    RegCloseKey(hKey);
 }
 //housekeeping
 main_window->close_register_popup_window();
 return accepted;
};

/* ensure that all the required registry keys are present, if present record 
relevant values, otherwise create a key with using the default value, this 
                     strategy facilitates upgrading and altering the program */
 void check_registry_keys(unsigned long &display_mode, 
                                                  unsigned long &display_method)
{
 unsigned long value_size, key_status;
 HKEY hKey;
 if(RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 
                                                   KEY_READ | KEY_WRITE, &hKey))
 { //if registry key does not exist create a new key
    RegCreateKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 0, 
                           REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0, &hKey, 0);
    RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 
                                                   KEY_READ | KEY_WRITE, &hKey);
 }
 //Check mode in registry
 key_status = RegQueryValueEx(hKey, REG_DISPLAY_MODE, 0,  NULL, 
                                            (LPBYTE)&display_mode, &value_size);
 if(key_status == ERROR_FILE_NOT_FOUND)
 {
    DEFAULT_MODE; 
    RegSetValueEx(hKey, REG_DISPLAY_MODE, 0, REG_DWORD, 
                                          (LPBYTE)&display_mode, sizeof(DWORD));
 }
 else if(display_mode > VERBOSE || display_mode < SILENT)
 {
    DEFAULT_MODE; 
    RegSetValueEx(hKey, REG_DISPLAY_MODE, 0, REG_DWORD, 
                                          (LPBYTE)&display_mode, sizeof(DWORD));
 }
 //Check method in registry
 key_status = RegQueryValueEx(hKey, REG_DISPLAY_METHOD, 0,  NULL, 
                                          (LPBYTE)&display_method, &value_size);
 if(key_status == ERROR_FILE_NOT_FOUND)
 {
    DEFAULT_METHOD; 
    RegSetValueEx(hKey, REG_DISPLAY_METHOD, 0, REG_DWORD, 
                                        (LPBYTE)&display_method, sizeof(DWORD));
 }
 else if(display_method > COVERT || display_method < OVERT)
 {
    DEFAULT_METHOD; 
    RegSetValueEx(hKey, REG_DISPLAY_METHOD, 0, REG_DWORD, 
                                        (LPBYTE)&display_method, sizeof(DWORD));
 }
 //Check top locus of main window: deals with changes in screen characteristics
 long top_gui;
 key_status = RegQueryValueEx(hKey, REG_TOP_GUI_CORNER, 0,  NULL, 
                                                 (LPBYTE)&top_gui, &value_size);
 if(key_status == ERROR_FILE_NOT_FOUND)
 {
    top_gui = MAIN_WINDOW_TOP;
    RegSetValueEx(hKey, REG_TOP_GUI_CORNER, 0, REG_DWORD, 
                                               (LPBYTE)&top_gui, sizeof(DWORD));
 }
 else if(top_gui > GetSystemMetrics(SM_CYSCREEN) - 50)
 {
    top_gui = GetSystemMetrics(SM_CYSCREEN) - 50; 
    RegSetValueEx(hKey, REG_TOP_GUI_CORNER, 0, REG_DWORD, 
                                               (LPBYTE)&top_gui, sizeof(DWORD));
 }
 //Check left locus of main window: deals with changes in screen characteristics
 long left_gui;
 key_status = RegQueryValueEx(hKey, REG_LEFT_GUI_CORNER, 0,  NULL, 
                                                (LPBYTE)&left_gui, &value_size);
 if(key_status == ERROR_FILE_NOT_FOUND)
 {
    left_gui = MAIN_WINDOW_LEFT;
    RegSetValueEx(hKey, REG_LEFT_GUI_CORNER, 0, REG_DWORD, 
                                          (LPBYTE)&left_gui, sizeof(DWORD));
 }
 if(left_gui > GetSystemMetrics(SM_CXSCREEN) - 50)
 {
    left_gui = GetSystemMetrics(SM_CXSCREEN) - 50; 
    RegSetValueEx(hKey, REG_LEFT_GUI_CORNER, 0, REG_DWORD, 
                                          (LPBYTE)&left_gui, sizeof(DWORD));
 }
 //Check top locus of verbose window: deals with changes in screen
  long top_verbose_gui;
  key_status = RegQueryValueEx(hKey, REG_TOP_VERBOSE_CORNER, 0,  NULL, 
                                         (LPBYTE)&top_verbose_gui, &value_size);
 if(key_status == ERROR_FILE_NOT_FOUND)
 {
    top_verbose_gui = VERBOSE_TOP;
    RegSetValueEx(hKey, REG_TOP_VERBOSE_CORNER, 0, REG_DWORD, 
                                       (LPBYTE)&top_verbose_gui, sizeof(DWORD));
 }
 if(top_verbose_gui > GetSystemMetrics(SM_CYSCREEN) - 50)
 {
    top_verbose_gui = GetSystemMetrics(SM_CYSCREEN) - 50; 
    RegSetValueEx(hKey, REG_TOP_VERBOSE_CORNER, 0, REG_DWORD, 
                                       (LPBYTE)&top_verbose_gui, sizeof(DWORD));
 }
 //Check left locus of verbose window: deals with changes in screen
 long left_verbose_gui;
 RegQueryValueEx(hKey, REG_LEFT_VERBOSE_CORNER, 0,  NULL, 
                                        (LPBYTE)&left_verbose_gui, &value_size);
 if(key_status == ERROR_FILE_NOT_FOUND)
 {
    left_verbose_gui = VERBOSE_LEFT;
    RegSetValueEx(hKey, REG_LEFT_VERBOSE_CORNER, 0, REG_DWORD, 
                                      (LPBYTE)&left_verbose_gui, sizeof(DWORD));
 }
 if(left_verbose_gui > GetSystemMetrics(SM_CXSCREEN) - 50)
 {
    left_verbose_gui = GetSystemMetrics(SM_CXSCREEN) - 50; 
    RegSetValueEx(hKey, REG_LEFT_VERBOSE_CORNER, 0, REG_DWORD, 
                                      (LPBYTE)&left_verbose_gui, sizeof(DWORD));
 }
 RegCloseKey(hKey);         //housekeeping
};

/* retrieve the name and path of a temporary folder holding files to be deleted 
                                                            from the registry */
char *get_temporary_folder_name()
{
 unsigned long value_size;
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_READ, &hKey);
 //Before reading strings from the registry this next (dummy) step is essential
 RegQueryValueEx(hKey, REG_TEMP_FOLDER_NAME, 0, NULL, NULL, &value_size);
 char *folder_name = new char[value_size];
 RegQueryValueEx(hKey, REG_TEMP_FOLDER_NAME, 0, NULL, 
                                              (LPBYTE)folder_name, &value_size);
 RegCloseKey(hKey);
 return folder_name;
}

unsigned long read_display_mode_from_registry()
{
 unsigned long DEFAULT_MODE, value_size = sizeof(DWORD);
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_READ, &hKey);
 RegQueryValueEx(hKey, REG_DISPLAY_MODE, 0,  NULL, (LPBYTE)&display_mode, 
                                                                   &value_size);
 RegCloseKey(hKey);
 return display_mode;
};

unsigned long read_display_method_from_registry()
{
 unsigned long DEFAULT_METHOD, value_size = sizeof(DWORD);
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_READ, &hKey);
 RegQueryValueEx(hKey, REG_DISPLAY_METHOD, 0,  NULL, (LPBYTE)&display_method, 
                                                                   &value_size);
 RegCloseKey(hKey);
 return display_method;
};

char *create_temporary_folders(HKEY hKey, char *filename)
{
 unsigned long error_code = 0;
 bool temp_folder = false;
 bool base_folder = false;
 char *pathname = new char[MAX_PATH];
 //copy drive letter to pathname
 char *out_pointer = pathname;
 char *in_pointer = filename;
 while(*in_pointer != '\\')
    *out_pointer++ = *in_pointer++;   //copy drive 
 *out_pointer = '\0';                 //null termiate output string
 strcat(pathname, PATH_TEMP);         //append a temporary locus
 if(!CreateDirectory(pathname, NULL)) //create temp folder if required
 {
    error_code = GetLastError();
    if(error_code == ERROR_ALREADY_EXISTS)
       temp_folder = true; 
    error_code = 0; 
 }
 strcat(pathname, PATH_PROGRAM);
 if(!CreateDirectory(pathname, NULL))
 {
    error_code = GetLastError();
    if(error_code == ERROR_ALREADY_EXISTS)
       base_folder = true; 
    error_code = 0; 
 }
 while(*out_pointer)                 //move output pointer
   *out_pointer++;                   //to current null character
 *out_pointer++ = '\\';              //add folder character
 //create unusual folder name
 unsigned long long number, value;
 LARGE_INTEGER tick; 
 do{
    QueryPerformanceCounter(&tick);   //Get elapsed time
    value = tick.LowPart;   //return least significant part of time elapsed
 }while(value < 256); 
 value *= (unsigned long long)time(NULL);
 for(unsigned short i = 0; i < 8; i++)
 {
    number = value % 128;
    number += 128;
    *out_pointer++ = number;
    value >>= 7;
 }
 *out_pointer = '\0';
 CreateDirectory(pathname, NULL);
 //update registry for temp folder and base folder
 unsigned long temp_status = 0, program_status = 0;
 if(!temp_folder) 
    temp_status = 1;
 if(!base_folder) 
    program_status = 1;
 unsigned long value_size, key_status;
 RegSetValueEx(hKey, REG_DELETE_TEMP_FOLDER, 0, REG_DWORD, 
                                           (LPBYTE)&temp_status, sizeof(DWORD));
 RegSetValueEx(hKey, REG_DEL_PROGRAM_FOLDER, 0, REG_DWORD, 
                                        (LPBYTE)&program_status, sizeof(DWORD));
 return pathname;
};

void remove_temporary_folders(HKEY hKey, char *folder_name)
{//Dedicated directory
 RemoveDirectory(folder_name);        //remove dedicated directory
 //Program directory
 char *folder_pointer = folder_name;
 while(*folder_pointer)               //locate end of string
    folder_pointer++;
 while(*folder_pointer != '\\')       //backtrack to last slash in the string
    folder_pointer--;
 *folder_pointer = '\0';              //replace slash with null vlaue  
 unsigned long value_size, folder_value;
 //check if program folder was empty and needs to be removed
 RegQueryValueEx(hKey, REG_DEL_PROGRAM_FOLDER, 0,  NULL, 
                                            (LPBYTE)&folder_value, &value_size);
  if(folder_value)
 {
    RemoveDirectory(folder_name);
    folder_value = 0;                 //reset folder value and amend registry
    RegSetValueEx(hKey, REG_DEL_PROGRAM_FOLDER, 0, REG_DWORD, 
                                  (LPBYTE)&folder_value, sizeof(DWORD));
 }
 //Temp directory
 while(*folder_pointer != '\\')       //backtrack to last slash in the string
    folder_pointer--;
 *folder_pointer = '\0';              //replace slash with null vlaue  
 //check if program folder was empty and needs to be removed
 RegQueryValueEx(hKey, REG_DELETE_TEMP_FOLDER, 0,  NULL, 
                                            (LPBYTE)&folder_value, &value_size);
 if(folder_value)
 {
    RemoveDirectory(folder_name);
    folder_value = 0;                 //reset folder value and amend registry
    RegSetValueEx(hKey, REG_DELETE_TEMP_FOLDER, 0, REG_DWORD, 
                                  (LPBYTE)&folder_value, sizeof(DWORD));
 }
};

/******************************************************************************/
/************ functions declared in main_window.h (hybrid function) ***********/ 
/******************************************************************************/

void shred_in_background(int arguments, char *argument_strings[], 
                                                     unsigned long display_mode)
{
 HKEY hKey;       /******* use a single registry key for creating and destroying 
 temporary folders to avoid the registry being misread during file shredding **/
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, 
                                                   KEY_READ | KEY_WRITE, &hKey);
 char *folder_name = create_temporary_folders(hKey, argument_strings[1]);
 /******************** record temporary folder name in registry so shredding can
 continue if it is interrupted ************************************************/
 //record directory path of temporary folder in registry
 RegSetValueEx(hKey, REG_TEMP_FOLDER_NAME, 0, REG_SZ, 
                                  (LPBYTE)folder_name, strlen(folder_name) + 1);
 //Create startup key
 char registry_string[MAX_PATH + 256];
 strcpy(registry_string, "\"");
 strcat(registry_string, argument_strings[0]);
 strcat(registry_string, "\" ");
 strcat(registry_string, COMPLETE_SHREDDING);
 HKEY hKey_startup;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_USER_STARTUP, 0, 
                                           KEY_READ | KEY_WRITE, &hKey_startup);
 RegSetValueEx(hKey_startup, REG_PROGRAM_NAME, 0, REG_SZ, 
                          (LPBYTE)registry_string, strlen(registry_string) + 1);
 unsigned long counter = 0;
 if(display_mode == SILENT || display_mode == CONFIRM)
 {//move files to a temporary folder
    move_files(arguments, argument_strings, folder_name, counter, 0);   
    parse_directory(folder_name, NO_THREAD_ID, NO_THREAD_TYPE);//shred files
 }
 else                                               //infers Verbose is default
 {
    move_files_verbose(arguments, argument_strings, folder_name, counter);   
    run_verbose(folder_name);                       //shred files
 }
 //delete start up instruction from registry
 unsigned long value_size, folder_value;
 RegDeleteValue(hKey_startup, REG_PROGRAM_NAME);
 RegCloseKey(hKey_startup);
 //delete directory path of temporary folder in registry
 if(ERROR_SUCCESS == RegQueryValueEx(hKey, REG_TEMP_FOLDER_NAME, 0,  NULL, 
                                            (LPBYTE)&folder_value, &value_size))
    RegDeleteValue(hKey, REG_TEMP_FOLDER_NAME);
 remove_temporary_folders(hKey, folder_name);
 RegCloseKey(hKey);
 //housekeeping
 delete [] folder_name;
};

/******************************************************************************/
/********* functions declared in main_window.h (GUI related functions) ********/ 
/******************************************************************************/

LRESULT CALLBACK run_main(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
 switch(message)
 {
    case WM_COMMAND:{
       switch(LOWORD(wparam)){
          case OVERT:
          case COVERT:{
             set_user_method(hwnd, LOWORD(wparam));
          }break;
          case SILENT:
          case CONFIRM:
          case VERBOSE:{
             set_user_mode(hwnd, LOWORD(wparam));
          }break;
          case CLOSE_GUI:{
             SendMessage(hwnd, WM_CLOSE, 0, 0);
          }break;
          case ABOUT:{
             main_window->run_about_popup_window();
          }break;
          default:
          break;
       }
    }break;
    case WM_SETFOCUS:{      // This message keeps a child window topmost so a 
       main_window->set_focus_to_sub_menu();    //user must respond to it 
    }break;                                     //before proceeding
    case WM_MOVE:
       main_window->update_GUI_loci();
    break;
    case WM_PAINT:
       display_shred_text(hwnd);
    break;
    case WM_DESTROY:                //Destroy all messages
       PostQuitMessage(false);
    break;
    default:
       return DefWindowProc(hwnd, message, wparam, lparam);
 }
 return 0;
};

void display_shred_text(HWND hwnd)
{//display first multi line test string: set up
 unsigned long small_offset = SCREEN_FONT_HEIGHT / 2;
 RECT rect;                                 //Define a window for the message
 rect.top = SCREEN_FONT_HEIGHT, rect.left = SCREEN_FONT_HEIGHT, 
                                            rect.bottom = 200, rect.right = 395;
 PAINTSTRUCT ps;
 HDC hdc = BeginPaint(hwnd, &ps); 
 //display logo
 BitBlt(hdc, rect.left, rect.top, 48, 48, main_window->logo_hdc, 0, 0, SRCCOPY);
 //add text components
 LOGFONT *screen_font = create_screen_font();        //Create font 
 rect.left += (48 + SCREEN_FONT_HEIGHT);
 //set weighting to bold
 screen_font->lfWeight = FW_BOLD;
 HFONT font = CreateFontIndirect(screen_font);       //Select font
 HGDIOBJ default_font = SelectObject(hdc, font);     //Choose font
 SetTextColor(hdc, TEXT_COLOUR);
 SetBkMode(hdc, TRANSPARENT);
 DrawText(hdc, SHRED_HEADER, -1, &rect, DT_LEFT);  //write string 
 //reset weighting to normal
 screen_font->lfWeight = FW_THIN;
 font = CreateFontIndirect(screen_font);             //Select font
 SelectObject(hdc, font);                            //Choose font
 rect.top += (SCREEN_FONT_HEIGHT + small_offset);
 DrawText(hdc, SHRED_HELP_00, -1, &rect, DT_LEFT);   //write string 
 //display second multi line test string
 rect.left = (2 * SCREEN_FONT_HEIGHT);
 rect.top += ((SCREEN_FONT_HEIGHT * 4) + small_offset);
 char text[512];
 //texts are different for silent and other modes
 switch((unsigned short)read_display_mode_from_registry())
 {
    case SILENT:{
       //build text sring
       strcpy(text, SHRED_HELP_01);
       strcat(text, SHRED_HELP_02b);
       strcat(text, SHRED_HELP_04);
       strcat(text, SHRED_HELP_05b);
       DrawText(hdc, text, -1, &rect, DT_LEFT); 
       rect.top += ((SCREEN_FONT_HEIGHT * 3) + small_offset);
    }break;
    case CONFIRM:
    case VERBOSE:{
       //build text sring
       strcpy(text, SHRED_HELP_01);
       strcat(text, SHRED_HELP_02a);
       strcat(text, SHRED_HELP_04);
       strcat(text, SHRED_HELP_05a);
       DrawText(hdc, text, -1, &rect, DT_LEFT); 
       rect.top += ((SCREEN_FONT_HEIGHT * 4) + small_offset);
    }break;
 }
 DrawText(hdc, SHRED_HELP_06, -1, &rect, DT_LEFT); 
 //housekeeping
 SelectObject(hdc, default_font);                    //revert to original font
 delete screen_font;
 EndPaint(hwnd, &ps); 
};

LRESULT CALLBACK run_about(HWND hwnd, UINT message, 
                                                   WPARAM wparam, LPARAM lparam)
{
 switch(message)
 {
    case WM_PAINT:
       display_about_text(hwnd);
    break;
    case WM_CLOSE:
       main_window->close_about_popup_window();
    break;
    default:
       return DefWindowProc(hwnd, message, wparam, lparam);
 }
 return 0;
};

void display_about_text(HWND hwnd)
{
 RECT rect;                                 //Define a window for the message
 unsigned long small_offset = SCREEN_FONT_HEIGHT / 2;
 rect.top = SCREEN_FONT_HEIGHT, rect.left = SCREEN_FONT_HEIGHT, 
                                             rect.bottom = 86, rect.right = 391;
 PAINTSTRUCT ps;
 HDC hdc = BeginPaint(hwnd, &ps); 
 BitBlt(hdc, rect.left, rect.top, 48, 48, main_window->logo_hdc, 0, 0, SRCCOPY);
 rect.left += 48;
 rect.left += SCREEN_FONT_HEIGHT;
 LOGFONT *screen_font = create_screen_font();        //Create font 
 //set weighting to bold
 screen_font->lfWeight = FW_BOLD;
 HFONT font = CreateFontIndirect(screen_font);       //Select font
 HGDIOBJ default_font = SelectObject(hdc, font);     //Choose font
 SetTextColor(hdc, TEXT_COLOUR);
 SetBkMode(hdc, TRANSPARENT);
 DrawText(hdc, FILE_SHREDDER_COPYRIGHT, -1, &rect, DT_LEFT);   //write string 
 //reset weighting to normal
 screen_font->lfWeight = FW_THIN;
 font = CreateFontIndirect(screen_font);             //Select font
 SelectObject(hdc, font);                            //Choose font
 rect.top += (SCREEN_FONT_HEIGHT + small_offset);
 DrawText(hdc, CURRENT_VERSION, -1, &rect, DT_LEFT);  //write string 
 screen_font->lfHeight = (-10);
 font = CreateFontIndirect(screen_font);             //Select font
 SelectObject(hdc, font);                            //Choose font
 rect.left = (SCREEN_FONT_HEIGHT + small_offset);
// rect.top += (SCREEN_FONT_HEIGHT + 48);
 rect.top += (48 - small_offset);
 DrawText(hdc, ABOUT_USAGE, -1, &rect, DT_LEFT);  //write string 
 screen_font->lfHeight = (-1 * SCREEN_FONT_HEIGHT);
 font = CreateFontIndirect(screen_font);             //Select font
 SelectObject(hdc, font);                            //Choose font
 rect.top = 300, rect.left = SCREEN_FONT_HEIGHT, rect.bottom = 400; 
 DrawText(hdc, ABOUT_CREDITS, -1, &rect, DT_LEFT);  //write string 
 //housekeeping
 SelectObject(hdc, default_font);                    //revert to original font
 delete screen_font;
 EndPaint(hwnd, &ps); 
};

LRESULT CALLBACK run_register(HWND hwnd, UINT message, 
                                                   WPARAM wparam, LPARAM lparam)
{
 switch(message)
 {
    case WM_COMMAND:{
       switch(wparam)
       {
          case DECLINES:{
             PostMessage(NULL, WM_COMMAND, false, 0);
          }break;
          case ACCCEPTS:{
             PostMessage(NULL, WM_COMMAND, true, 0);
          }break;
          default:
          break;
       }
    }break;
    case WM_PAINT:
       display_register_text(hwnd);
    break;
    case WM_CLOSE:
       PostMessage(NULL, WM_COMMAND, false, 0);
    break;
    default:
       return DefWindowProc(hwnd, message, wparam, lparam);
 }
 return 0;
};

void display_register_text(HWND hwnd)
{
 RECT rect;                                 //Define a window for the message
 unsigned long small_offset = SCREEN_FONT_HEIGHT / 2;
 rect.top = SCREEN_FONT_HEIGHT, rect.left = SCREEN_FONT_HEIGHT, 
                                             rect.bottom = 86, rect.right = 391;
 PAINTSTRUCT ps;
 HDC hdc = BeginPaint(hwnd, &ps); 
 BitBlt(hdc, rect.left, rect.top, 48, 48, main_window->logo_hdc, 0, 0, SRCCOPY);
 rect.left += 48;
 rect.left += SCREEN_FONT_HEIGHT;
 LOGFONT *screen_font = create_screen_font();        //Create font 
 //set weighting to bold
 screen_font->lfWeight = FW_BOLD;
 HFONT font = CreateFontIndirect(screen_font);       //Select font
 HGDIOBJ default_font = SelectObject(hdc, font);     //Choose font
 SetTextColor(hdc, TEXT_COLOUR);
 SetBkMode(hdc, TRANSPARENT);
 DrawText(hdc,FILE_SHREDDER_COPYRIGHT, -1, &rect, DT_LEFT);    //write string 
 //reset weighting to normal
 screen_font->lfWeight = FW_THIN;
 font = CreateFontIndirect(screen_font);             //Select font
 SelectObject(hdc, font);                            //Choose font
 rect.top += (SCREEN_FONT_HEIGHT + small_offset);
 DrawText(hdc, SHRED_HELP_00, -1, &rect, DT_LEFT);   //write string 

 rect.top = 290, rect.left = SCREEN_FONT_HEIGHT, rect.bottom = 400; 
 DrawText(hdc, REGISTER_HELP_01, -1, &rect, DT_LEFT);  //write string 
 //housekeeping
 SelectObject(hdc, default_font);                    //revert to original font
 delete screen_font;
 EndPaint(hwnd, &ps); 
};

LRESULT CALLBACK run_about_scroll(HWND hwnd, UINT message, 
                                                   WPARAM wparam, LPARAM lparam)
{
 switch(message)
 {
    case WM_VSCROLL:{                 //Slider activated
       adjust_v_scroll_bars(hwnd, wparam);
    }break;               
    case WM_PAINT:
       display_about_legal_text(hwnd);
    break;
    default:
       return DefWindowProc(hwnd, message, wparam, lparam);
 }
 return 0;
};

void display_about_legal_text(HWND hwnd)
{//set up to draw window
 RECT rect;                                 //Define a window for the message
 unsigned long small_offset = SCREEN_FONT_HEIGHT / 2;
 PAINTSTRUCT ps;
 HDC hdc = BeginPaint(hwnd, &ps); 

 SCROLLINFO scroll_propertires;
 memset(&scroll_propertires, 0, sizeof(SCROLLINFO));
 scroll_propertires.cbSize = sizeof(SCROLLINFO);
 scroll_propertires.fMask = SIF_ALL;
 GetScrollInfo(hwnd, SB_VERT, &scroll_propertires);

 //slider need to adjust this between 0 and -46 window 194 high
 rect.top = 0 - scroll_propertires.nPos; 
 rect.left = 0, rect.bottom = TERMS_LENGTH + rect.top, rect.right = 340;

 //set up fonts, including colours and backgrounds
 LOGFONT *screen_font = create_screen_font();        //Create font 
 HFONT font = CreateFontIndirect(screen_font);       //Select font
 HGDIOBJ default_font = SelectObject(hdc, font);     //Choose font
 SetTextColor(hdc, TEXT_BLACK);
 SetBkMode(hdc, TRANSPARENT);
 //Create the contnets
 DrawText(hdc, GNU_LICENSE_3, -1, &rect, DT_LEFT);  //write string 
 //housekeeping
 SelectObject(hdc, default_font);                    //revert to original font
 delete screen_font;
 EndPaint(hwnd, &ps); 
};

void adjust_v_scroll_bars(HWND hwnd, unsigned long wparam)
{
 SCROLLINFO scroll_propertires;
 memset(&scroll_propertires, 0, sizeof(SCROLLINFO));
 scroll_propertires.cbSize = sizeof(SCROLLINFO);
 scroll_propertires.fMask = SIF_ALL;
 GetScrollInfo(hwnd, SB_VERT, &scroll_propertires);
 unsigned long scroll_range = scroll_propertires.nMax - scroll_propertires.nMin;
 long scroll_locus = scroll_propertires.nPos;
 //determine incrument (or decrement)
 //determine incrument (or decrement): //set incrument size
 long long step_size = 1;
 if(GetKeyState(VK_SHIFT) < 0)
    step_size *= 3;
 if(GetKeyState(VK_CONTROL) < 0)
    step_size *= SCREEN_FONT_HEIGHT;
 if(GetKeyState(VK_MENU) < 0)
    step_size *= 100;
 switch (LOWORD(wparam))
 {
    case SB_LINELEFT:       //Slider moved by clicking left/down arrow
       scroll_locus -= step_size;
    break;               
    case SB_LINERIGHT:      //Slider moved by clicking right/up arrow
       scroll_locus += step_size;
    break;               
    case SB_THUMBTRACK:{    //Slider moved with mouse
       scroll_locus = HIWORD(wparam);
    }break;               
    default:
       return;              //ignore all other actions
 } 
 if(scroll_locus < scroll_propertires.nMin)
    scroll_locus = scroll_propertires.nMin;
 if(scroll_locus > scroll_range - scroll_propertires.nPage)
    scroll_locus = scroll_range - scroll_propertires.nPage;
// if(scroll_locus == scroll_propertires.nPos)    //if there is no movement
//    return;                                     //do nothing
 scroll_propertires.nPos = scroll_locus;
 SetScrollInfo(hwnd, SB_VERT, &scroll_propertires, true);
 RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
};

/******************************************************************************/
/*********************** functions declared in verbose.h **********************/ 
/******************************************************************************/
bool run_window()
{
 MSG message;
 while(GetMessage(&message, NULL, 0, 0))
 {  //Trap messages from accept & decline buttons in registration window
    if(!message.hwnd && message.message == WM_COMMAND && message.wParam)
       return true;
    else if(!message.hwnd && message.message == WM_COMMAND)
       return false;
    TranslateMessage(&message);       //Displays text as it is typed in
    DispatchMessage(&message);
 }
 return true;
};

WNDCLASSEX register_verbose_gui()
{
 WNDCLASSEX window;
 memset(&window, 0, sizeof(WNDCLASSEX));        //zero the structure
 window.cbSize        = sizeof(WNDCLASSEX);
 window.lpfnWndProc   = run_verbose;
 window.hInstance     = GetModuleHandle(NULL);
 window.hCursor       = LoadCursor(NULL, IDC_ARROW);
 window.hbrBackground = CreateSolidBrush(BACKGROUND_COLOUR);
 window.lpszClassName = PROGRESS_GUI;
 window.hIcon         = LoadIcon(window.hInstance, MAKEINTRESOURCE(MAIN_ICON));
 return window;
};

LOGFONT *create_screen_font()
{
 LOGFONT *screen_font = new LOGFONT; 
 memset(screen_font, 0, sizeof(LOGFONT));
 screen_font->lfHeight = (-1 * SCREEN_FONT_HEIGHT);
 screen_font->lfWeight = FW_THIN;
 screen_font->lfPitchAndFamily = FF_SWISS;      //sans serif
 strcpy(screen_font->lfFaceName, "Arial");
 return screen_font;
};

/******************************************************************************/
/*********************** functions for GUI_Window class ***********************/ 
/******************************************************************************/
GUI_Window::GUI_Window()
{
 memset(this, 0, sizeof(*this));
 //allocate default window coordinates
 top = MAIN_WINDOW_TOP;
 left = MAIN_WINDOW_LEFT;
 height = MAIN_WINDOW_HEIGHT;
 width = MAIN_WINDOW_WIDTH;
 //update values from the registry
 unsigned long value_size = sizeof(DWORD);
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_READ, &hKey);
 RegQueryValueEx(hKey, REG_TOP_GUI_CORNER, 0, NULL, (LPBYTE)&top, &value_size);
 RegQueryValueEx(hKey, REG_LEFT_GUI_CORNER, 0, NULL, (LPBYTE)&left, 
                                                                   &value_size);
 RegCloseKey(hKey);
 //Set up (or define) the form of the window
 WNDCLASSEX window;
 memset(&window, 0, sizeof(WNDCLASSEX));        //zero the structure
 window.cbSize        = sizeof(WNDCLASSEX);
 window.lpfnWndProc   = run_main;
 window.hInstance     = GetModuleHandle(NULL);
 window.hCursor       = LoadCursor(NULL, IDC_ARROW);
 window.hbrBackground = CreateSolidBrush(BACKGROUND_COLOUR);
 window.lpszMenuName  = "MAIN_MENU";            //Name of menu template resorce
 window.lpszClassName = MAIN_GUI;
 window.hIcon         = LoadIcon(window.hInstance, MAKEINTRESOURCE(MAIN_ICON));
 RegisterClassEx(&window);
 //load logo bitmap
 logo_hdc = CreateCompatibleDC(NULL);
 HBITMAP h_logo_bitmap = LoadBitmap(GetModuleHandle(NULL), 
                                                  MAKEINTRESOURCE(LOGO_BITMAP));
 SelectObject(logo_hdc, h_logo_bitmap);    //copy screen_bitmap to bitmap_hdc 
 DeleteObject(h_logo_bitmap);              //deletes original screen_bitmap         
};

GUI_Window::~GUI_Window()
{//record last top left window loci in the registry
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_WRITE, &hKey);
 //record loci of upper left corner in registry
 RegSetValueEx(hKey, REG_TOP_GUI_CORNER, 0, REG_DWORD,
                                                   (LPBYTE)&top, sizeof(DWORD));
 RegSetValueEx(hKey, REG_LEFT_GUI_CORNER, 0, REG_DWORD, 
                                                  (LPBYTE)&left, sizeof(DWORD));
 RegCloseKey(hKey);
 if(logo_hdc)
    DeleteDC(logo_hdc);
 DestroyWindow(hwnd);
};

HWND GUI_Window::create_GUI_window()
{
 //Define and create the actual fixed sized window and record its handle
 hwnd = CreateWindowEx(0, MAIN_GUI, PROGRAM_NAME,
           WS_SYSMENU | WS_CAPTION | WS_VISIBLE, left, top, width, height, 
                               HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);
 //Set menubar to reflect the diplay mode stored in the registry settings
 set_user_mode(hwnd, read_display_mode_from_registry());
 set_user_method(hwnd, read_display_method_from_registry());
 return hwnd;
};

void GUI_Window::update_GUI_loci()
{
 if(!hwnd)                  //Move is called during CreateWindowEx(... ), this
    return;                 //avoids extracting a value from unused memory 
 RECT rect;
 GetWindowRect(hwnd, &rect);
 left = rect.left;
 top = rect.top;
};

void GUI_Window::run_about_popup_window()
{
 WNDCLASSEX window;
 memset(&window, 0, sizeof(WNDCLASSEX));        //zero the structure
 window.cbSize        = sizeof(WNDCLASSEX);
 window.lpfnWndProc   = run_about;
 window.hInstance     = GetModuleHandle(NULL);
 window.hCursor       = LoadCursor(NULL, IDC_ARROW);
 window.hbrBackground = CreateSolidBrush(BACKGROUND_COLOUR);
 window.lpszClassName = ABOUT_GUI;
 window.hIcon         = LoadIcon(window.hInstance, MAKEINTRESOURCE(MAIN_ICON));
 RegisterClassEx(&window);
 memset(&window, 0, sizeof(WNDCLASSEX));        //zero the structure
 window.cbSize        = sizeof(WNDCLASSEX);
 window.lpfnWndProc   = run_about_scroll;
 window.hInstance     = GetModuleHandle(NULL);
 window.hCursor       = LoadCursor(NULL, IDC_ARROW);
 window.hbrBackground = CreateSolidBrush(BACKGROUND_WHITE);
 window.lpszClassName = ABOUT_SCROLL_GUI;
 RegisterClassEx(&window);
 //Window is a popup menu from main GUI
 hwnd_about = CreateWindowEx(0, ABOUT_GUI, ABOUT_PROGRAM_NAME,
                WS_SYSMENU | WS_CAPTION | WS_POPUP | WS_VISIBLE, 
                                   left + 80, top + 50, width, 400, hwnd, 
                                             NULL, GetModuleHandle(NULL), NULL);
 //update the menubar
 set_about_menu(hwnd, true);          //only allows a single instance to run
 HWND hwnd_terms = CreateWindowEx(0, ABOUT_SCROLL_GUI, NULL, 
             WS_CHILD | WS_DLGFRAME | WS_VSCROLL | WS_VISIBLE, 
                                             10, 88, width - 24, 200,
                                 hwnd_about, NULL, GetModuleHandle(NULL), NULL); 
 //set up scroll bar
 SCROLLINFO scroll_propertires;
 memset(&scroll_propertires, 0, sizeof(SCROLLINFO));
 scroll_propertires.cbSize = sizeof(SCROLLINFO);
 scroll_propertires.fMask = SIF_ALL;
 GetScrollInfo(hwnd_terms, SB_VERT, &scroll_propertires);
 scroll_propertires.nMax = TERMS_LENGTH;   //length of text or script
 scroll_propertires.nPage = 256 - 48;      //height of window available (less 2)
 SetScrollInfo(hwnd_terms, SB_VERT, &scroll_propertires, true);
};

void GUI_Window::close_about_popup_window()
{
 set_about_menu(hwnd, false);         //make the about menu available
 DestroyWindow(hwnd_about);
 hwnd_about = NULL;
};

//This function keeps a child window topmost so a user must respond to it
void GUI_Window::set_focus_to_sub_menu()
{
 if(hwnd_register)          //if the "Register File Shredder" window is open
    SetFocus(hwnd_register);//it steals the focus
 if(hwnd_about)             //if the "About File Shredder" window is open
    SetFocus(hwnd_about);   //it steals the focus
};                          //otherwise leaves the window accessible

void GUI_Window::run_register_popup_window()
{
 WNDCLASSEX window;
 memset(&window, 0, sizeof(WNDCLASSEX));        //zero the structure
 window.cbSize        = sizeof(WNDCLASSEX);
 window.lpfnWndProc   = run_register;
 window.hInstance     = GetModuleHandle(NULL);
 window.hCursor       = LoadCursor(NULL, IDC_ARROW);
 window.hbrBackground = CreateSolidBrush(BACKGROUND_COLOUR);
 window.lpszClassName = REGISTER_GUI;
 window.hIcon         = LoadIcon(window.hInstance, MAKEINTRESOURCE(MAIN_ICON));
 RegisterClassEx(&window);
 memset(&window, 0, sizeof(WNDCLASSEX));        //zero the structure
 window.cbSize        = sizeof(WNDCLASSEX);
 window.lpfnWndProc   = run_about_scroll;
 window.hInstance     = GetModuleHandle(NULL);
 window.hCursor       = LoadCursor(NULL, IDC_ARROW);
 window.hbrBackground = CreateSolidBrush(BACKGROUND_WHITE);
 window.lpszClassName = ABOUT_SCROLL_GUI;
 RegisterClassEx(&window);
 //Window is a popup menu from main GUI
 hwnd_register = CreateWindowEx(0, REGISTER_GUI, REGISTER_PROGRAM,
                WS_SYSMENU | WS_CAPTION | WS_POPUP | WS_VISIBLE, 
                                   left + 80, top + 50, width, 400, hwnd, 
                                             NULL, GetModuleHandle(NULL), NULL);
 //Create buttons to accept and decline registration
 CreateWindowEx(0, "button", BUTTON_DECLINE, 
          WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 292, 295, 95, 25, 
                   hwnd_register, (HMENU)DECLINES, GetModuleHandle(NULL), NULL);
 CreateWindowEx(0, "button", BUTTON_ACCEPT, WS_CHILD | WS_VISIBLE, 
                                                      292, 325, 95, 25, 
                   hwnd_register, (HMENU)ACCCEPTS, GetModuleHandle(NULL), NULL);

 HWND hwnd_terms = CreateWindowEx(0, ABOUT_SCROLL_GUI, NULL, 
             WS_CHILD | WS_DLGFRAME | WS_VSCROLL | WS_VISIBLE, 
                                             10, 88, width - 24, 200,
                              hwnd_register, NULL, GetModuleHandle(NULL), NULL); 
 //set up scroll bar
 SCROLLINFO scroll_propertires;
 memset(&scroll_propertires, 0, sizeof(SCROLLINFO));
 scroll_propertires.cbSize = sizeof(SCROLLINFO);
 scroll_propertires.fMask = SIF_ALL;
 GetScrollInfo(hwnd_terms, SB_VERT, &scroll_propertires);
 scroll_propertires.nMax = TERMS_LENGTH;   //length of text or script
 scroll_propertires.nPage = 256 - 48;      //height of window available (less 2)
 SetScrollInfo(hwnd_terms, SB_VERT, &scroll_propertires, true);
};


void GUI_Window::close_register_popup_window()
{
 set_about_menu(hwnd, false);         //make the about menu available
 DestroyWindow(hwnd_register);
 hwnd_register = NULL;
};

//abstracted away as long section of text writing code that does little
unsigned long registration_string(char *reg_string)
{
 char *string_pointer = reg_string;
 *string_pointer = '\0';
 copy_string(string_pointer, CALENDAR_REG);
 time_t now = time(NULL);
 tm *time_array = localtime(&now);
 unsigned_2_numeral(string_pointer, time_array->tm_hour);
 //inset zero with single digit values
 if(*(string_pointer - 2) == ' ')
 {
    string_pointer++;
    *string_pointer = *(string_pointer - 1); 
    *(string_pointer - 1) = *(string_pointer - 2); 
    *(string_pointer - 2) = '0';
 }
 copy_string(string_pointer, CALENDAR_COL);
 unsigned_2_numeral(string_pointer, time_array->tm_min);
 //inset zero with single digit values
 if(*(string_pointer - 2) == ':')
 {
    string_pointer++;
    *string_pointer = *(string_pointer - 1); 
    *(string_pointer - 1) = *(string_pointer - 2); 
    *(string_pointer - 2) = '0';
 }
 copy_string(string_pointer, CALENDAR_ONT);
 unsigned_2_numeral(string_pointer, time_array->tm_mday);
 switch(time_array->tm_mday){
    case 1:
    case 21:
    case 31:
    copy_string(string_pointer, CALENDAR_ST);
    break;
    case 2:
    case 22:
       copy_string(string_pointer, CALENDAR_ND);
    break;
    case 3:
    case 23:
       copy_string(string_pointer, CALENDAR_RD);
    break;
    default:   
       copy_string(string_pointer, CALENDAR_TH);
    break;
 }
 switch(time_array->tm_mon){
    case 0:
       copy_string(string_pointer, CALENDAR_JAN);
    break;
    case 1:
       copy_string(string_pointer, CALENDAR_FEB);
    break;
    case 2:
       copy_string(string_pointer, CALENDAR_MAR);
    break;
    case 3:
       copy_string(string_pointer, CALENDAR_APR);
    break;
    case 4:
       copy_string(string_pointer, CALENDAR_MAY);
    break;
    case 5:
       copy_string(string_pointer, CALENDAR_JUN);
    break;
    case 6:
       copy_string(string_pointer, CALENDAR_JLY);
    break;
    case 7:
       copy_string(string_pointer, CALENDAR_AUG);
    break;
    case 8:
       copy_string(string_pointer, CALENDAR_SEP);
    break;
    case 9:
       copy_string(string_pointer, CALENDAR_OCT);
    break;
    case 10:
       copy_string(string_pointer, CALENDAR_NOV);
    break;
    case 11:
       copy_string(string_pointer, CALENDAR_DEC);
    break;
 }
 unsigned_2_numeral(string_pointer, 1900 + time_array->tm_year);
 return string_pointer - reg_string + 1;
};
