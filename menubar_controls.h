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

#ifndef MENUBAR_CONTROLS_H
#define MENUBAR_CONTROLS_H

#include "global_features.h"
#include "text.h"

void set_user_mode(HWND, unsigned long);
void set_user_method(HWND, unsigned long);
void set_about_menu(HWND, bool);


#endif
