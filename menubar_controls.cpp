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

#include "menubar_controls.h"
#include "main_window.h"

void set_user_mode(HWND hwnd, unsigned long mode_requested)
{
 HMENU menu_bar = GetMenu(hwnd);
 //activate all menus
 EnableMenuItem(menu_bar, SILENT, MF_BYCOMMAND | MF_ENABLED);
 EnableMenuItem(menu_bar, CONFIRM, MF_BYCOMMAND | MF_ENABLED);
 EnableMenuItem(menu_bar, VERBOSE, MF_BYCOMMAND | MF_ENABLED);
 CheckMenuItem(menu_bar, SILENT, MF_BYCOMMAND | MF_UNCHECKED);
 CheckMenuItem(menu_bar, CONFIRM, MF_BYCOMMAND | MF_UNCHECKED);
 CheckMenuItem(menu_bar, VERBOSE, MF_BYCOMMAND | MF_UNCHECKED);
 //grey out selected menu and mark as checked (ticked)
 EnableMenuItem(menu_bar, mode_requested, MF_BYCOMMAND | MF_GRAYED);
 CheckMenuItem(menu_bar, mode_requested, MF_BYCOMMAND | MF_CHECKED);
 //update registry
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_WRITE, &hKey);
 RegSetValueEx(hKey, REG_DISPLAY_MODE, 0, REG_DWORD, 
                                        (LPBYTE)&mode_requested, sizeof(DWORD));
 RegCloseKey(hKey);
 DrawMenuBar(hwnd);                 //make updates visible
 //update main window to reflect changes
 InvalidateRect(hwnd, NULL, TRUE);
 UpdateWindow(hwnd);
/*a single alternative to the two functions above is
 RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE); */
};

void set_user_method(HWND hwnd, unsigned long method_requested)
{
 HMENU menu_bar = GetMenu(hwnd);
 //activate all menus
 EnableMenuItem(menu_bar, OVERT, MF_BYCOMMAND | MF_ENABLED);
 EnableMenuItem(menu_bar, COVERT, MF_BYCOMMAND | MF_ENABLED);
 CheckMenuItem(menu_bar, OVERT, MF_BYCOMMAND | MF_UNCHECKED);
 CheckMenuItem(menu_bar, COVERT, MF_BYCOMMAND | MF_UNCHECKED);
 //grey out selected menu and mark as checked (ticked)
 EnableMenuItem(menu_bar, method_requested, MF_BYCOMMAND | MF_GRAYED);
 CheckMenuItem(menu_bar, method_requested, MF_BYCOMMAND | MF_CHECKED);
 //update registry
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_WRITE, &hKey);
 RegSetValueEx(hKey, REG_DISPLAY_METHOD, 0, REG_DWORD, 
                                      (LPBYTE)&method_requested, sizeof(DWORD));
 RegCloseKey(hKey);
 DrawMenuBar(hwnd);                 //make updates visible
};

void set_about_menu(HWND hwnd, bool greymenu)
{
 HMENU menu_bar = GetMenu(hwnd);
 if(greymenu)
    EnableMenuItem(menu_bar, ABOUT, MF_BYCOMMAND | MF_GRAYED);
 else
    EnableMenuItem(menu_bar, ABOUT, MF_BYCOMMAND | MF_ENABLED);
 DrawMenuBar(hwnd);                 //make updates visible
};
