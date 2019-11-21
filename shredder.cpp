#include "verbose.h"
#include "shredder.h"
#include "text.h"

static bool shredding_thread_running = false;             //mutex
unsigned long WINAPI shredding_function(void *);          //forward declaration

void shredder(int arguments, char *argument_strings[])
{
 unsigned long long file_length;
 //determine number of prcessors
 SYSTEM_INFO system__info;
 memset(&system__info, 0, sizeof(SYSTEM_INFO));
 GetSystemInfo(&system__info); 
 for(unsigned long i = 1; i < arguments; i++)   //i = 1 skips the folder path
 {
    SetFileAttributes(argument_strings[i], FILE_ATTRIBUTE_NORMAL);
    if(PathIsDirectoryA(argument_strings[i]))
    {
       parse_directory(argument_strings[i], NO_THREAD_ID, NO_THREAD_TYPE);
       RemoveDirectory(argument_strings[i]);
    }
    else
    { //open file to get file length
       HANDLE hFile = CreateFile(argument_strings[i], GENERIC_READ, 0, NULL, 
                                                        OPEN_EXISTING, 0, NULL);
       if(hFile == INVALID_HANDLE_VALUE)        //if thre is a problem
          continue;                             //fail gracefully
       //get file length
       BY_HANDLE_FILE_INFORMATION file_data;
       GetFileInformationByHandle(hFile, &file_data);
       file_length = file_data.nFileSizeHigh;   //get top 32 bits
       file_length <<= 32;                      //assign these bits correctly
       file_length += file_data.nFileSizeLow;   //add bottom 32 bits
       //set a file buffer size: set a maximum default
       unsigned long long file_buffer_size = 1;
       file_buffer_size <<= MAXIMUM_BUFFER_BITS;
       //measure free uncached memory
       MEMORYSTATUSEX memory_data;                        //declae struct           
       memset(&memory_data, 0, sizeof(MEMORYSTATUSEX));   //clear struct
       memory_data.dwLength = sizeof(MEMORYSTATUSEX);     //give struct size
       GlobalMemoryStatusEx(&memory_data);                //fill struct 
       //need two buffers and space for other things so divide by three
       unsigned long long memory_space = memory_data.ullAvailPhys / 3;
       //adjust file buffer size to the largest power of two below memory limit  
       while(file_buffer_size > memory_space)
          file_buffer_size >>= 1;
       CloseHandle(hFile);                      //close file
       CloseHandle(hFile);                      //close file
       //shred file
       if(system__info.dwNumberOfProcessors == 1)
          linear_shred(argument_strings[i], file_length, file_buffer_size);
       else if(file_length < FILE_SIZE_BOUNDARY)
          linear_shred(argument_strings[i], file_length, file_buffer_size);
       else
          threaded_shred(argument_strings[i], file_length, file_buffer_size);
       //delete shredded file (does not put the file in the recycle bin
       DeleteFile(argument_strings[i]);
    }
 }
};

void parse_directory(char *argument_string, unsigned long thread_id, 
                                                      unsigned long thread_type)
{
 //modify argument string for FindFirstFile which requires a wild card 
 char new_string[MAX_PATH];
 strcpy(new_string, argument_string);
 strcat(new_string, "\\*\0"); 
 //prepare to parse directory
 WIN32_FIND_DATA file_parameters;
 HANDLE hFile = FindFirstFile(new_string, &file_parameters);
 //count files and folders in directory
 unsigned long file_counter = 0;
 while(FindNextFile(hFile, &file_parameters))
    file_counter++;
 FindClose(hFile);
 if(file_counter == 1)      //Folder is empty
    return;                 //Delete folder
 //prepare a list of files and folders in the directory
 char **argument_strings = new char*[file_counter];
 for(unsigned long i = 0; i < file_counter; i++)
    argument_strings[i] = new char[MAX_PATH];
 hFile = FindFirstFile(new_string, &file_parameters);
 unsigned long i = 0;
 while(FindNextFile(hFile, &file_parameters))
 {
    strcpy(argument_strings[i], argument_string);
    strcat(argument_strings[i], "\\");
    strcat(argument_strings[i++], file_parameters.cFileName);
 }
 FindClose(hFile);
 if(thread_type == SHREDDING_HIDDEN_WINDOW)
    hidden_verbose_shredder(file_counter, argument_strings, thread_id);
 if(thread_id)
    verbose_shredder(file_counter, argument_strings, thread_id);
 else
    shredder(file_counter, argument_strings);
 //Housekeeping
 for(unsigned long i = 0; i < file_counter; i++)
    delete [] argument_strings[i];
 delete [] argument_strings;
};

void linear_shred(char filename[], unsigned long long file_length, 
                                             unsigned long long max_buffer_size)
{
 unsigned long array_length, short_array_length;
 unsigned long bytes_read, bytes_written;
 //initialise and prime random numbers, using the filename as part of the seed
 MTRand mtrand;
 char *name_pointer = filename;
 while(*name_pointer)                 //find end of filename
    name_pointer++;
 while(*name_pointer != '\\')         //find end of file path
    name_pointer--;
 name_pointer++;                      //point to file name
 mtrand = seed_random_from_file(name_pointer);
 prime_random(mtrand);
 //Break long files into iterations of the maximum buffer size
 unsigned long iterations = 0;
 if(file_length > max_buffer_size)
 {
    iterations = file_length / max_buffer_size; 
    file_length %= max_buffer_size; 
 }
 //create buffers that accept an unsigned long pointer
 if(iterations)
    array_length = max_buffer_size; 
 else
 {
    array_length = file_length; 
    unsigned long over_run = file_length % sizeof(unsigned long);
    if(over_run)
       array_length += (sizeof(unsigned long) - over_run);
 }
 short_array_length = array_length / sizeof(unsigned long);
 unsigned char *primary_array = new unsigned char [array_length];
 unsigned char *secondary_array = new unsigned char [array_length];
 //assign long pointers to arrays 
 unsigned long *long_primary = (unsigned long *)primary_array;
 unsigned long *long_secondary = (unsigned long *)secondary_array;
 //Open file
 HANDLE hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                                  OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
 //read file and overwrite sequences
 unsigned long long total_bytes_written = 0;
 unsigned long low_distance = 0;
 long high_distance = 0;
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    ReadFile(hFile, primary_array, max_buffer_size, &bytes_read, NULL); 
    //copy primary array to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
    //invert data in secondary array
    for(unsigned long i = 0; i < short_array_length; i++)
       long_primary[i] = ~long_primary[i]; 
    //copy secondary array (inverted data) to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, max_buffer_size, &bytes_written, NULL);//?                                                FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
    //advance file pointer indicees
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    ReadFile(hFile, primary_array, file_length, &bytes_read, NULL); 
    //copy primary array to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, file_length, &bytes_written, NULL);//?                                                FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
    //invert data in secondary array
    for(unsigned long i = 0; i < short_array_length; i++)
       long_primary[i] = ~long_primary[i]; 
    //copy secondary array (inverted data) to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
 }
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 //zero data in primary array
 memset(primary_array, 64, array_length);
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 //all one's in secondary array
 memset(secondary_array, 255, array_length);
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, secondary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, secondary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 //write first set of random numbers to file:
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {  //create random numbers
    for(unsigned long i = 0; i < short_array_length; i++)
       long_primary[i] = mtrand.randInt(); 
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    for(unsigned long i = 0; i < short_array_length; i++)
       long_primary[i] = mtrand.randInt(); 
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 //write second set of random numbers to file:
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {  //create random numbers
    for(unsigned long i = 0; i < short_array_length; i++)
       long_secondary[i] = mtrand.randInt(); 
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, secondary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    for(unsigned long i = 0; i < short_array_length; i++)
       long_secondary[i] = mtrand.randInt(); 
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, secondary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 //housekeeping
 CloseHandle(hFile);                                        //close file
 delete [] primary_array;
 delete [] secondary_array;
};

void threaded_shred(char filename[], unsigned long long file_length, 
                                             unsigned long long max_buffer_size)
{
 unsigned long array_length, short_array_length;
 unsigned long bytes_read, bytes_written;
 //prepare a thread structure
 Shredding_thread_structure *shredding_thread = new Shredding_thread_structure; 
 //initialise and prime random numbers, using the filename as part of the seed
 MTRand mtrand;
 char *name_pointer = filename;
 while(*name_pointer)                 //find end of filename
    name_pointer++;
 while(*name_pointer != '\\')         //find end of file path
    name_pointer--;
 name_pointer++;                      //point to file name
 mtrand = seed_random_from_file(name_pointer);
 prime_random(mtrand);
 //Break long files into iterations of the maximum buffer size
 unsigned long iterations = 0;
 if(file_length > max_buffer_size)
 {
    iterations = file_length / max_buffer_size; 
    file_length %= max_buffer_size; 
 }
 //create buffers that accept an unsigned long pointer
 if(iterations)
    array_length = max_buffer_size; 
 else
 {
    array_length = file_length; 
    unsigned long over_run = file_length % sizeof(unsigned long);
    if(over_run)
       array_length += (sizeof(unsigned long) - over_run);
 }
 short_array_length = array_length / sizeof(unsigned long);
 unsigned char *primary_array = new unsigned char [array_length];
 unsigned char *secondary_array = new unsigned char [array_length];
 //assign long pointers to arrays 
 unsigned long *long_primary = (unsigned long *)primary_array;
 unsigned long *long_secondary = (unsigned long *)secondary_array;
 //Open file
 HANDLE hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                                  OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
 //read file and overwrite sequences
 unsigned long long total_bytes_written = 0;
 unsigned long low_distance = 0;
 long high_distance = 0;
 //threading create second buffer whilst writing the first
 shredding_thread->random = ARRAY;
 shredding_thread->array = secondary_array; 
 shredding_thread->value = 0;
 shredding_thread->array_length = array_length;
 HANDLE h_thread = CreateThread(NULL, 0, shredding_function, 
                                             (void *)shredding_thread, 0, NULL);
 //read file and overwrite sequences
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    ReadFile(hFile, primary_array, max_buffer_size, &bytes_read, NULL); 
    //copy primary array to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
    //invert data in secondary array
    for(unsigned long i = 0; i < short_array_length; i++)
       long_primary[i] = ~long_primary[i]; 
    //copy secondary array (inverted data) to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, max_buffer_size, &bytes_written, NULL);//?                                                FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
    //advance file pointer indicees
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    ReadFile(hFile, primary_array, file_length, &bytes_read, NULL); 
    //copy primary array to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, file_length, &bytes_written, NULL);//?                                                FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
    //invert data in secondary array
    for(unsigned long i = 0; i < short_array_length; i++)
       long_primary[i] = ~long_primary[i]; 
    //copy secondary array (inverted data) to file
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    FlushFileBuffers(hFile);                                   //flush buffer
 }
 while(shredding_thread_running)
    Sleep(15);
 CloseHandle(h_thread);
 //fill primary array with all ones
 shredding_thread->random = ARRAY;
 shredding_thread->array = primary_array; 
 shredding_thread->value = 255;
 shredding_thread->array_length = array_length;
 h_thread = CreateThread(NULL, 0, shredding_function, 
                                             (void *)shredding_thread, 0, NULL);
 //Write all zeros in secondary array to file
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, secondary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, secondary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 while(shredding_thread_running)
    Sleep(15);
 CloseHandle(h_thread);
 //fill secondary arry with random numbers
 shredding_thread->random = RANDOM;
 shredding_thread->array = secondary_array; 
 shredding_thread->value = 0;
 shredding_thread->array_length = short_array_length;
 h_thread = CreateThread(NULL, 0, shredding_function, 
                                             (void *)shredding_thread, 0, NULL);
 //Write all ones in primary array to file
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
 }
 if(file_length)
 {
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, primary_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 while(shredding_thread_running)
    Sleep(15);
 CloseHandle(h_thread);
 //set up to alternate buffer usage
 unsigned char *current_array = secondary_array;
 unsigned char *buffer_array = NULL;  //belt and braces code
 unsigned char *next_array = primary_array;
 //write first set of random numbers to file
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 //write first set of random numbers to file:
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {  //create random numbers
    shredding_thread->array = next_array; 
    h_thread = CreateThread(NULL, 0, shredding_function, 
                                             (void *)shredding_thread, 0, NULL);
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, current_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
    while(shredding_thread_running)
       Sleep(15);
    CloseHandle(h_thread);
    buffer_array = current_array;
    current_array = next_array;
    next_array = buffer_array; 
    buffer_array = NULL;              //belt and braces code
 }
 if(file_length)
 {
    shredding_thread->array = next_array; 
    h_thread = CreateThread(NULL, 0, shredding_function, 
                                             (void *)shredding_thread, 0, NULL);
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, current_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    while(shredding_thread_running)
       Sleep(15);
    CloseHandle(h_thread);
    buffer_array = current_array;
    current_array = next_array;
    next_array = buffer_array; 
    buffer_array = NULL;              //belt and braces code
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 //write second set of random numbers to file:
 total_bytes_written = 0;
 low_distance = 0;                    //reset the file pointer
 high_distance = 0;                   //reset the file pointer
 //write first set of random numbers to file:
 for(unsigned long i = 0; i < iterations; i++)            //write file
 {  //create random numbers
    shredding_thread->array = next_array; 
    h_thread = CreateThread(NULL, 0, shredding_function, 
                                             (void *)shredding_thread, 0, NULL);
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, current_array, max_buffer_size, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    total_bytes_written += max_buffer_size;
    low_distance = total_bytes_written;
    high_distance = total_bytes_written >> 32;
    while(shredding_thread_running)
       Sleep(15);
    CloseHandle(h_thread);
    buffer_array = current_array;
    current_array = next_array;
    next_array = buffer_array; 
    buffer_array = NULL;              //belt and braces code
 }
 if(file_length)
 {
    shredding_thread->array = next_array; 
    h_thread = CreateThread(NULL, 0, shredding_function, 
                                             (void *)shredding_thread, 0, NULL);
    SetFilePointer(hFile, (long)low_distance, &high_distance, FILE_BEGIN); 
    WriteFile(hFile, current_array, file_length, &bytes_written, NULL);//?                                                 FILE_FLAG_WRITE_THROUGH, NULL);
    while(shredding_thread_running)
       Sleep(15);
    CloseHandle(h_thread);
    buffer_array = current_array;
    current_array = next_array;
    next_array = buffer_array; 
    buffer_array = NULL;              //belt and braces code
 }
 FlushFileBuffers(hFile);                                   //flush buffer
 //housekeeping
 CloseHandle(hFile);                                        //close file
 delete [] primary_array;
 delete [] secondary_array;
 delete shredding_thread;
};

unsigned long WINAPI shredding_function(void *structure)
{
 shredding_thread_running = true;
 Shredding_thread_structure *shredding_thread = 
                                        (Shredding_thread_structure *)structure;
 if(shredding_thread->random)
 {
    MTRand mtrand = seed_random_from_file(shredding_thread->file_name);
    prime_random(mtrand);
    unsigned long *long_array = (unsigned long *)shredding_thread->array;
    for(unsigned long i = 0; i < shredding_thread->array_length; i++)
       long_array[i] = mtrand.randInt(); 
 }
 else
    memset(shredding_thread->array, shredding_thread->value, 
                                                shredding_thread->array_length);
 shredding_thread_running = false;
 return 0;             //Return to operating system
};

MTRand seed_random_from_file(char *file_name)
{
 //Create four unsigned long values to seed the generator with
 unsigned long param_1, param_2, param_3, param_4;
 //Seed first parameter using fast internal timer
 LARGE_INTEGER elapsed;
 do{
    QueryPerformanceCounter(&elapsed);          //Get elapsed time
    param_1 = elapsed.LowPart;
 }while(param_1 < 256);
 //seed next two parameters from file name
 unsigned long length = strlen(file_name);
 if(length > 12)
    file_name += 2;
 if(length > 7)
 {
    unsigned long i = 0;
    while(i++ < 4)
    {
       param_2 <<= 8;      
       param_2 += *file_name++;
    }
    i = 0;
    while(i++ < 4)
    {
       param_3 <<= 8;      
       param_3 += *file_name++;
    }
 }
 else if(length > 4)
 {
    unsigned long i = 0;
    while(i++ < 4)
    {
       param_2 <<= 8;      
       param_2 += *file_name++;
    }
    i = 0;
    file_name -= 3;
    i = 0;
    while(i++ < 4)
    {
       param_3 <<= 8;      
       param_3 += *file_name++;
    }
 }
 else
 {
    unsigned long i = 0;
    param_2 = 0;
    while(i++ < length)
    {
       param_2 <<= 8;      
       param_2 += *file_name++;
    }
    param_3 = ~param_2;
 }
 //Seed fourth parameter using seconds since January 1, 1970
 param_4 = (unsigned long)time(NULL);
 /*The next section of code Pertains to MersenneTwister.h; (Richard J. Wagner 
 15 May 2003). The use of the Mersenne Twister is not recommended for 
 encryptation (http://en.wikipedia.org/wiki/Mersenne_twister).*/
 MTRand::uint32 bigSeed[4] = {param_1, param_2, param_3, param_4};
 MTRand mtrand(bigSeed, 4);                //seed the random number generator
 return mtrand;
};


void prime_random(MTRand &mtrand)
{
 unsigned long seed; 
 LARGE_INTEGER time;
 do{
    QueryPerformanceCounter(&time);
    seed = (unsigned short)time.LowPart;
 }while(seed < 256);
 for(unsigned long i = 0; i < seed; i++)
    mtrand.randInt();
};

void parse_directory(char *argument_string, char *new_folder, 
                                unsigned long &counter, unsigned long thread_id)
{
 //modify argument string for FindFirstFile which requires a wild card 
 char new_string[MAX_PATH];
 strcpy(new_string, argument_string);
 strcat(new_string, "\\*\0"); 
 //prepare to parse directory
 WIN32_FIND_DATA file_parameters;
 HANDLE hFile = FindFirstFile(new_string, &file_parameters);
 //count files and folders in directory
 unsigned long file_counter = 0;
 while(FindNextFile(hFile, &file_parameters))
    file_counter++;
 FindClose(hFile);
 //prepare a list of files and folders in the directory
 char **argument_strings = new char*[file_counter];
 for(unsigned long i = 0; i < file_counter; i++)
    argument_strings[i] = new char[MAX_PATH];
 hFile = FindFirstFile(new_string, &file_parameters);
 unsigned long i = 0;
 while(FindNextFile(hFile, &file_parameters))
 {
    strcpy(argument_strings[i], argument_string);
    strcat(argument_strings[i], "\\");
    strcat(argument_strings[i++], file_parameters.cFileName);
 }
 FindClose(hFile);
 move_files(file_counter, argument_strings, new_folder, counter, thread_id);
 //Housekeeping
 for(unsigned long i = 0; i < file_counter; i++)
    delete [] argument_strings[i];
 delete [] argument_strings;
};

