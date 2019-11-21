#include "global_features.h"

char *fixed_length_numeral(unsigned long number, unsigned short length)
{
 unsigned long shift = length + 1;         //enumerate string length
 char *numeral = new char[shift];          //declare string
 char *pointer = numeral;                  //create pointer to string
 unsigned_2_numeral(pointer, number);      //convert number to numeral string
 //shift string to end of array
 shift -= null_string_length(numeral);      //calculate shift
 for(unsigned long i = null_string_length(numeral); i;)
 {//move string values by shift (including the null termination) 
    i--;
    numeral[i + shift] = numeral[i];
 }
 memset(numeral, '0', shift);              //fill start of string with zeros
 return numeral;
};

//point to null charater at the end of a string
char *null_string(char *string_pointer)
{
 while(*string_pointer)
    string_pointer++;
 return string_pointer;
};

unsigned long null_string(char *string, char *&string_pointer)
{
 string_pointer = string; 
 while(*string_pointer++);
 return (string_pointer-- - string);
};

unsigned long null_string_length(char *string)
{
 char *string_pointer = string; 
 while(*string_pointer++);
 return (string_pointer-- - string);  //decrements string_pointer after sum
};

//point to the end of a string
char *end_string(char *string_pointer)
{
 while(*string_pointer)
    string_pointer++;
 return --string_pointer;
};

unsigned long end_string(char *string, char *&string_pointer)
{
 string_pointer = string; 
 while(*string_pointer++);
 string_pointer--;
 return (string_pointer-- - string);
};

unsigned long end_string_length(char *string)
{
 char *string_pointer = string; 
 while(*string_pointer)
    string_pointer++;
 return (string_pointer-- - string);  //decrements string_pointer after sum
};

void unsigned_2_numeral(char *&numeral, unsigned long long value)
{
 if(value / 10)
    unsigned_2_numeral(numeral, value / 10);
 *numeral++ = '0' + (value % 10);
 *numeral = '\0';
};

void copy_string(char *&string_out, char *string_in)
{
 while(*string_in)
    *string_out++ = *string_in++;
 *string_out = '\0';
};

