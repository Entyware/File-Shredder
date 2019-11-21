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
