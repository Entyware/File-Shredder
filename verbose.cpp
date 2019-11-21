#include "verbose.h"
#include "text.h"


//forward declarations of variables involved in the verbose GUI thread
static char **verbose_file_text = NULL;
static char **verbose_folder_text = NULL;

void move_files_verbose(int arguments, char *argument_strings[], 
                                      char *folder_name, unsigned long &counter)
{
 unsigned long thread_id;
 unsigned long thread_type = MOVING_WINDOW;
 HANDLE h_thread = CreateThread(NULL, 0, verbose_thread_function, 
                                            (void *)thread_type, 0, &thread_id);
 move_files(arguments, argument_strings, folder_name, counter, thread_id);   
 //housekeeping
 PostThreadMessage(thread_id, WM_QUIT, 0, 0);   //triggers mutex release whilst 
 //forcing the thread to cease looping and destroying the associated window 
 CloseHandle(h_thread);
};   

void move_files(int arguments, char *argument_strings[], 
              char *new_folder, unsigned long &counter, unsigned long thread_id)
{
 for(unsigned long i = 1; i < arguments; i++)   //i = 1 skips the folder path
 {
    SetFileAttributes(argument_strings[i], FILE_ATTRIBUTE_NORMAL);
    if(PathIsDirectoryA(argument_strings[i]))
    {
       parse_directory(argument_strings[i], new_folder, counter, thread_id);
       RemoveDirectory(argument_strings[i]);
    }
    else
    {  //Create new filepath
       char new_name[MAX_PATH];
       strcpy(new_name, new_folder);
       strcat(new_name, "\\");
       //append new file name
       char *file_name;
       file_name = fixed_length_numeral(counter, 15);
       strcat(new_name, file_name);
       delete [] file_name;
       strcat(new_name, ".del");
       MoveFile(argument_strings[i], new_name);
       counter++;
//       if(thread_id && (!(counter % MOVE_UPDATE_FREQUENCY)))
//          PostThreadMessage(thread_id, REFRESH_WINDOW, 0, 0);// not with spinner
    }
 }
};

void run_verbose(char *folder_name)
{//orignal pointers to different lines of text must be different
 char *null_text_0 = "";
 verbose_file_text = &null_text_0;
 char *null_text_1 = "";
 verbose_folder_text = &null_text_1;
 //Run verbose information window in a different thread
 unsigned long thread_id;
 unsigned long thread_type = SHREDDING_HIDDEN_WINDOW;
 HANDLE h_thread = CreateThread(NULL, 0, verbose_thread_function, 
                                            (void *)thread_type, 0, &thread_id);
 *verbose_folder_text = folder_name;
 parse_directory(folder_name, thread_id, thread_type);
 //housekeeping
 PostThreadMessage(thread_id, WM_QUIT, 0, 0);   /*triggers mutex release whilst 
 forcing the thread to cease looping and destroying the associated window*/ 
 CloseHandle(h_thread);
};

void run_verbose(int arguments, char *argument_strings[])
{//orignal pointers to different lines of text must be different
 char *null_text_0 = "";
 verbose_file_text = &null_text_0;
 char *null_text_1 = "";
 verbose_folder_text = &null_text_1;
 //Run verbose information window in a different thread
 unsigned long thread_id;
 unsigned long thread_type = SHREDDING_WINDOW;
 HANDLE h_thread = CreateThread(NULL, 0, verbose_thread_function, 
                                            (void *)thread_type, 0, &thread_id);
 //initiate shredding
 verbose_shredder(arguments, argument_strings, thread_id); 
 //housekeeping
 PostThreadMessage(thread_id, WM_QUIT, 0, 0);   /*triggers mutex release whilst 
 forcing the thread to cease looping and destroying the associated window */ 
 CloseHandle(h_thread);                         //housekeeping
};

void verbose_shredder(int arguments, char *argument_strings[], 
                                                        unsigned long thread_id)
{ //Set up GUI window
 char empty_text[MAX_PATH];
 memset(empty_text, ' ', MAX_PATH);   //create a string of spaces 
 empty_text[MAX_PATH - 1] = '\0';     //null trminate string
 char pathname[MAX_PATH];
 memcpy(pathname, empty_text, MAX_PATH); 
 unsigned long long file_length;
 //determine number of processors
 SYSTEM_INFO system_info;
 memset(&system_info, 0, sizeof(SYSTEM_INFO));
 GetSystemInfo(&system_info); 
 for(unsigned long i = 1; i < arguments; i++)   //i = 1 skips the folder path
 {  //make all folders and files deletable
    SetFileAttributes(argument_strings[i], FILE_ATTRIBUTE_NORMAL);
    if(PathIsDirectoryA(argument_strings[i]))
    {  //create pathname that clears the previous line of text
       memcpy(pathname, empty_text, MAX_PATH); 
       strcpy(pathname, argument_strings[i]);
       pathname[strlen(argument_strings[i])] = ' ';  //replace null value
       *verbose_folder_text = pathname;     //switch names instantly
       parse_directory(argument_strings[i], thread_id, NO_THREAD_TYPE);
       //parse directory() can alter the pathname so revert to original
       *verbose_folder_text = pathname;     //switch names instantly
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
       //copy filename into blank and extract the filename
       char filename[MAX_PATH];
       char *name_pointer = filename;        
       memcpy(filename, empty_text, MAX_PATH); 
       copy_string(name_pointer, argument_strings[i]);
       *name_pointer-- = ' ';              //replace null value
       while(*name_pointer != '\\')        //locate end of directory name
          name_pointer--;
       *name_pointer++ = '\0';             //replace backslash with null value

       *verbose_file_text = name_pointer;  //switch names instantly
       /*causes thread window to update to show the new name: not required when 
                                                        spinner is being used */
//       PostThreadMessage(thread_id, REFRESH_WINDOW, 0, 0);
       //shred file
       if(system_info.dwNumberOfProcessors == 1)
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

void hidden_verbose_shredder(int arguments, char *argument_strings[], 
                                                        unsigned long thread_id)
{ //Set up GUI window
 char empty_text[MAX_PATH];
 memset(empty_text, ' ', MAX_PATH);   //create a string of spaces 
 empty_text[MAX_PATH - 1] = '\0';     //null trminate string
 char pathname[MAX_PATH];
 memcpy(pathname, empty_text, MAX_PATH); 
 unsigned long long file_length;
 //determine number of processors
 SYSTEM_INFO system_info;
 memset(&system_info, 0, sizeof(SYSTEM_INFO));
 GetSystemInfo(&system_info); 
 for(unsigned long i = 1; i < arguments; i++)   //i = 1 skips the folder path
 {  //make all folders and files deletable
    SetFileAttributes(argument_strings[i], FILE_ATTRIBUTE_NORMAL);
    HANDLE hFile = CreateFile(argument_strings[i], GENERIC_READ, 0, NULL, 
                                                        OPEN_EXISTING, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE)           //if thre is a problem
       continue;                                //fail gracefully
    //get file length
    BY_HANDLE_FILE_INFORMATION file_data;
    GetFileInformationByHandle(hFile, &file_data);
    file_length = file_data.nFileSizeHigh;      //get top 32 bits
    file_length <<= 32;                         //assign these bits correctly
    file_length += file_data.nFileSizeLow;      //add bottom 32 bits
    //set a file buffer size: set a maximum default
    unsigned long long file_buffer_size = 1;
    file_buffer_size <<= MAXIMUM_BUFFER_BITS;
    //measure free uncached memory
    MEMORYSTATUSEX memory_data;                           //declae struct           
    memset(&memory_data, 0, sizeof(MEMORYSTATUSEX));      //clear struct
    memory_data.dwLength = sizeof(MEMORYSTATUSEX);        //give struct size
    GlobalMemoryStatusEx(&memory_data);                   //fill struct 
    //need two buffers and space for other things so divide by three
    unsigned long long memory_space = memory_data.ullAvailPhys / 3;
    //adjust file buffer size to the largest power of two below memory limit  
    while(file_buffer_size > memory_space)
       file_buffer_size >>= 1;
    CloseHandle(hFile);                         //close file
    //create file n of m files
    char filename[MAX_PATH];
    char *name_pointer = filename;        
    memcpy(filename, empty_text, MAX_PATH); 
    copy_string(name_pointer, MOVE_FILE);
    unsigned_2_numeral(name_pointer, i);
    copy_string(name_pointer, ONE_OF);
    unsigned_2_numeral(name_pointer, arguments - 1);
    copy_string(name_pointer, MOVE_FILES);
    *name_pointer = ' ';              //replace null value
    name_pointer = filename;
    *verbose_file_text = name_pointer;  //switch names instantly
    /*causes thread window to update to show the new name: not required when 
                                                        spinner is being used */
//      PostThreadMessage(thread_id, REFRESH_WINDOW, 0, 0);// not with spinner
    //shred file
    if(system_info.dwNumberOfProcessors == 1)
       linear_shred(argument_strings[i], file_length, file_buffer_size);
    else if(file_length < FILE_SIZE_BOUNDARY)
       linear_shred(argument_strings[i], file_length, file_buffer_size);
    else
       threaded_shred(argument_strings[i], file_length, file_buffer_size);
    //delete shredded file (does not put the file in the recycle bin
    DeleteFile(argument_strings[i]);
 }
};

unsigned long WINAPI verbose_thread_function(void *type)
{
 while(verbose_window)      //forces the function to wait for any other thread
    Sleep(15);              //to release the window. The thread controlling 
 //function must send PostThreadMessage(thread_id, WM_QUIT, 0, 0) before closing 
 unsigned long thread_type = (unsigned long)type;
 verbose_window = new Verbose_Window(thread_type);        //acts as the mutex
 HWND hwnd = verbose_window->create_verbose_window();
 //set up spinner
 RECT rect;
 GetClientRect(hwnd, &rect);
 start_spinner(hwnd, rect.right - 48, (6 * rect.bottom) / 10, SPINNER_WIDTH, 
                                                                   TEXT_COLOUR);
 //run window: a trap to update the window is not needed when spinner running
 MSG message;
 while(GetMessage(&message, NULL, 0, 0))
 {  //trap thread messages from move_files() function
//    if((!message.hwnd) && (message.message == REFRESH_WINDOW))
//       SendMessage(hwnd, WM_PAINT, 0, 0);
//    else
       DispatchMessage(&message);
 }
 //housekeeping activated by PostThreadMessage(thread_id, WM_QUIT, 0, 0)
 stop_spinner();
 if(verbose_window)
    delete verbose_window;  //this must be run from inside the thread function
 verbose_window = NULL;                                   //releases mutex
 return 0;
};

LRESULT CALLBACK run_verbose(HWND hwnd, UINT message, WPARAM wparam, 
                                                                  LPARAM lparam)
{
 switch(message)
 {
    case WM_MOVE:
       verbose_window->update_verbose_loci();
    break;
    case WM_PAINT:
       verbose_window->display_verbose_text();
    break;
    case WM_DESTROY:                //Destroy all messages
       PostQuitMessage(false);
    break;
    default:
       return DefWindowProc(hwnd, message, wparam, lparam);
 }
 return 0;
};

/******************************************************************************/
/********************* functions for Verbose_Window class *********************/ 
/******************************************************************************/

Verbose_Window::Verbose_Window(unsigned long type)
{
 memset(this, 0, sizeof(*this));
 window_type = type;
 LOGFONT *font = create_screen_font(); 
 screen_font = CreateFontIndirect(font);        //Select font
 font->lfWeight = FW_BOLD;                      //Create bold font
 bold_font = CreateFontIndirect(font);          //Select bold font
 //create symbolic binary font
 font->lfHeight = (-1 * 10);
 font->lfWeight = FW_THIN;
 delete font;                                   //housekeeping
};

Verbose_Window::~Verbose_Window()
{//record last used values in the registry
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_WRITE, &hKey);
 //record loci of upper left corner in registry
 RegSetValueEx(hKey, REG_TOP_VERBOSE_CORNER, 0, REG_DWORD,
                                                   (LPBYTE)&top, sizeof(DWORD));
 RegSetValueEx(hKey, REG_LEFT_VERBOSE_CORNER, 0, REG_DWORD, 
                                                  (LPBYTE)&left, sizeof(DWORD));
 RegCloseKey(hKey);
 //Housekeeping
 ReleaseDC(hwnd, hdc);                //is paired with GetDC(HWND)
 DestroyWindow(hwnd);                 //clears window from screen
 DeleteObject(screen_font);
 DeleteObject(bold_font);
};

HWND Verbose_Window::create_verbose_window()
{
 height = VERBOSE_HEIGHT;
 width = VERBOSE_WIDTH;
 //Read the display mode top and left window locus values from the registry
 unsigned long value_size = sizeof(DWORD);
 HKEY hKey;
 RegOpenKeyEx(HKEY_CURRENT_USER, REG_SHREDDER_SETTINGS, 0, KEY_READ, &hKey);
 if(RegQueryValueEx(hKey, REG_TOP_VERBOSE_CORNER, 0, NULL, (LPBYTE)&top, 
                                                  &value_size) != ERROR_SUCCESS)
    top = VERBOSE_TOP;

 if(RegQueryValueEx(hKey, REG_LEFT_VERBOSE_CORNER, 0, NULL, (LPBYTE)&left, 
                                                  &value_size) != ERROR_SUCCESS)
    left = VERBOSE_LEFT;
 RegCloseKey(hKey);
 WNDCLASSEX window = register_verbose_gui();
 RegisterClassEx(&window);
 hwnd = CreateWindowEx(0, PROGRESS_GUI, PROGRAM_NAME,
           WS_SYSMENU | WS_CAPTION | WS_VISIBLE, left, top, width, height, 
                               HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);
 hdc = GetDC(hwnd);               //forces function to cycle over and over
 SetTextColor(hdc, TEXT_COLOUR);
 SetBkColor(hdc, BACKGROUND_COLOUR);  //Erases previous text
// SetBkMode(hdc, TRANSPARENT);
 return hwnd;
};

void Verbose_Window::display_verbose_text()
{//updates the frame of window, called as part of BeginPaint(hwnd, &ps);
 PAINTSTRUCT ps;
 BeginPaint(hwnd, &ps); 
 EndPaint(hwnd, &ps);                 //Not required for DrawText() 
 //updates window proper
 SelectObject(hdc, bold_font);        //set font to bold: keep before Rect rect
 RECT rect;                           //Define a window for the message
 rect.top = 5, rect.left = 5; 
 rect.bottom = VERBOSE_HEIGHT - 10, rect.right = VERBOSE_WIDTH - 10;
 if(window_type == SHREDDING_WINDOW)
 {
    DrawText(hdc, VERBOSE_HEADER, -1, &rect, DT_LEFT); 
    SelectObject(hdc, screen_font);   //reset font to normal: keep before rect +
    //position and write the texts
    rect.left += 10;
    rect.top += (2 * SCREEN_FONT_HEIGHT);
    DrawText(hdc, *verbose_folder_text, -1, &rect, DT_LEFT); 
    rect.top += (4 * SCREEN_FONT_HEIGHT / 3);
    DrawText(hdc, *verbose_file_text, -1, &rect, DT_LEFT); 
 }
 else if(window_type == MOVING_WINDOW) 
 {
    DrawText(hdc, MOVE_FILE_HEADER, -1, &rect, DT_LEFT); 
    //draw a white rectangle
    rect.top += ((5 * SCREEN_FONT_HEIGHT) / 2); 
    rect.bottom = rect.top + 35;
    rect.left = 30, rect.right = VERBOSE_WIDTH - 200;
    FillRect(hdc, &rect, CreateSolidBrush(COLORREF(BACKGROUND_WHITE)));
    //reset text colour and keep background white
    SetTextColor(hdc, TEXT_BLACK);
    SetBkMode(hdc, TRANSPARENT);
    rect.top += 3;
    //rolling pointer to advance text
    char *text = SYMBOLIC_TEXT;
    DrawText(hdc, text + text_locus, -1, &rect, DT_LEFT); 
    rect.top += 15;
    DrawText(hdc, text + text_locus + 14, -1, &rect, DT_LEFT); 
    text_locus += 2;
    if(text_locus > 200)
    text_locus = 0;
    //reset text colour and return to default background
    SetBkMode(hdc, OPAQUE);
    SetTextColor(hdc, TEXT_COLOUR);
    SelectObject(hdc, screen_font);   //reset font to normal text
 }
 else if(window_type == SHREDDING_HIDDEN_WINDOW)
 {
    DrawText(hdc, VERBOSE_HEADER, -1, &rect, DT_LEFT); 
    SelectObject(hdc, screen_font);   //reset font to normal text
    //create next line of text
    rect.left += 10;
    rect.top += (2 * SCREEN_FONT_HEIGHT);
    DrawText(hdc, *verbose_file_text, -1, &rect, DT_LEFT); 
 }
// EndPaint(hwnd, &ps);                 //traditionally end paint is placed here 
};

void Verbose_Window::update_verbose_loci()
{
 if(!hwnd)                  //Move called during CreateWindowEx(... )
    return;                 //avoids extracting a value from unused memory 
 RECT rect;
 GetWindowRect(hwnd, &rect);
 left = rect.left;
 top = rect.top;
 //Update window to eliminate content ghositing following rapid movement
 InvalidateRect(hwnd, NULL, TRUE);   
 UpdateWindow(hwnd);
/*a single alternative to the two functions above is
 RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE); */
};

/******************************************************************************/
/***************************** Spinner components *****************************/ 
/******************************************************************************/
//Spinner compnents
static Spinner_structure *spinner_thread = NULL;     //also serves as mutex

void start_spinner(HWND hwnd, long x_mid, long y_mid, long diameter, 
                                                                COLORREF colour)
{
 spinner_thread = new Spinner_structure;        //mutex set 
 spinner_thread->root_hwnd = hwnd;
 spinner_thread->x_mid = x_mid;                 //coordinates of center
 spinner_thread->y_mid = y_mid;
 spinner_thread->diameter = diameter;           //diameter of ellipse 
 spinner_thread->colour = colour;               //diameter of ellipse 
 spinner_thread->h_thread = CreateThread(NULL, 0, run_spinner_thread, 
                                               (void *)spinner_thread, 0, NULL);
};

void stop_spinner()
{
 HANDLE h_thread = spinner_thread->h_thread;    //housekeeping
 delete spinner_thread;   
 spinner_thread = NULL;                         //mutex released 
 CloseHandle(h_thread);
};

unsigned long WINAPI run_spinner_thread(void *thread_structure)
{
 Spinner_structure *spinner_thread = (Spinner_structure *)thread_structure;
 HDC h_spinner = GetDC(spinner_thread->root_hwnd);
 //define elipse outline
 HPEN ellipse_properties = CreatePen(PS_SOLID, 2, spinner_thread->colour);
 SelectObject(h_spinner, ellipse_properties); 
 //make ellipse center transparent
 LOGBRUSH brush;
 memset(&brush, 0, sizeof(LOGBRUSH));
 brush.lbStyle = BS_NULL;
 HBRUSH ellipse_fill = CreateBrushIndirect(&brush);
 SelectObject(h_spinner, ellipse_fill); 
 //define the properties of the ellipse 
 RECT rect;
 long half_width = spinner_thread->diameter / 2; 
 long width_limit = half_width; 
 bool grow = true;
 rect.top = spinner_thread->y_mid - half_width - 1;
 rect.bottom = rect.top + spinner_thread->diameter + 1;
 //animate the ellipse to make it spin
 while(spinner_thread)
 {  
    rect.left = spinner_thread->x_mid - half_width - 2;
    rect.right = spinner_thread->x_mid + half_width;
    InvalidateRect(spinner_thread->root_hwnd, &rect, TRUE);   
    UpdateWindow(spinner_thread->root_hwnd);
    Ellipse(h_spinner, rect.left + 2, rect.top + 1, 
                                               rect.right - 1, rect.bottom - 2);
    //prepare the ellipse for the next frame
    if(half_width == width_limit)
       grow = false;
    if(half_width == 3)     //(3) is an arbitray value, chose by appearance
       grow = true;
    if(grow)
       half_width++;
    else
       half_width--;
    Sleep(45);
 }
 //housekeeping
 DeleteObject(ellipse_properties);
 DeleteObject(ellipse_fill);
 ReleaseDC(spinner_thread->root_hwnd, h_spinner);    //paired with GetDC(HWND)
};
