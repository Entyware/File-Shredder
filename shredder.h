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
