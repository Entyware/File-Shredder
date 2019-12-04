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

#ifndef SHREDDER_H
#define SHREDDER_H

//#include <iostream>
//using namespace std;
#include "global_features.h"
#pragma pack(1)            // sets alignment to a 1 byte boundary
#include "MersenneTwister.h"         //Random number generator

#define RANDOM              true
#define ARRAY               false


struct Shredding_thread_structure{
   //writing uniform arrays
   unsigned char *array;
   unsigned char value;
   unsigned long array_length;
   //writing random arrays
   bool random;
   char file_name[MAX_PATH];
   Shredding_thread_structure(){memset(this, 0, sizeof(*this));};
   ~Shredding_thread_structure(){};
};

//Forward declarations of random number functions in shredder.cpp
MTRand seed_random_from_file(char *);
void prime_random(MTRand &);

//Forward declarations of functions in main_window.cpp
void shredder(int, char *[]);

#endif
