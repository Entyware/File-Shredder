#ifndef TEXT_H
#define TEXT_H

//Global language definitions
#define PROGRAM_NAME             "File Shredder"
#define ABOUT_PROGRAM_NAME       "About File Shredder"
#define REGISTER_PROGRAM         "Register File Shredder"

#define SHREDDING_TITLE          "File Shredding"
#define SHREDDING_MESSAGE        "Proceed with shredding files?\r\n\
(recovery of shredded data is not possible)"

#define MENU_OPTIONS             "&Options"

#define MENU_MODE                "M&ode"
#define MENU_SILENT              "&Silent"
#define MENU_CONFIRM             "&Request confirmation"
#define MENU_VERBOSE             "&Verbose"

#define MENU_METHOD              "M&ethod"
#define MENU_OVERT               "&Delete selected files directly"
#define MENU_COVERT              "&Hide files before deletion"

#define MENU_CLOSE               "&Close"

#define MENU_ABOUT               "&About"

//Registry text
#define REG_SHREDDER_SETTINGS    "Software\\File Shredder\\Settings"
#define REG_USER_STARTUP         "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_USER_AGREEMENT       "registration details"
#define REG_DISPLAY_MODE         "display mode"
#define REG_DISPLAY_METHOD       "display method"
#define REG_TOP_GUI_CORNER       "top_GUI_corner"
#define REG_LEFT_GUI_CORNER      "left_GUI_corner"
#define REG_TOP_VERBOSE_CORNER   "top_verbose_GUI_corner"
#define REG_LEFT_VERBOSE_CORNER  "left_verbose_GUI_corner"
#define REG_DELETE_TEMP_FOLDER   "delete temp folder"
#define REG_DEL_PROGRAM_FOLDER   "delete program folder"
#define REG_TEMP_FOLDER_NAME     "delete on startup"
#define REG_PROGRAM_NAME         "File Shredder.exe"

//complete unfinished hidden shredding
#define COMPLETE_SHREDDING        "Complete_shredding"

//Main GUI text
#define SHRED_HEADER            "A brief guide."

#define SHRED_HELP_00            "File Shredder deletes files on a hard disk \
drive in a \r\nmanner that is intended to make any recovery of the \r\ndata \
impossible." 
#define SHRED_HELP_01            "To shred files; select all the files and \
folders to be shredded"
#define SHRED_HELP_02a           ", \r\n"
#define SHRED_HELP_02b           " \r\nand "
#define SHRED_HELP_04            "drag these over the 'File Shredder' icon"
#define SHRED_HELP_05a           " and then confirm \r\nthat you wish to shred \
them when prompted to do so."
#define SHRED_HELP_05b           "."
#define SHRED_HELP_06            "There are three modes of operation offering \
different levels \r\nof feedback to users and two methods, either hiding all \
the \r\nfiles before deleting them or removing the files one by one."

#define REGISTER_HELP_01         "This program has the capability to delete \
files that \r\n\
are essential if Windows is to keep running. I have \r\n\
read the above and I accept and agree that any \r\n\
licence to use this program is only granted on the \r\n\
understanding that it performs 'as is'."


#define BUTTON_DECLINE           "decline"
#define BUTTON_ACCEPT            "accept"

//Verbose window text
#define VERBOSE_HEADER           "Shredding "
#define MOVE_FILE_HEADER         "Moving Data "
#define MOVE_FILE                "File "
#define ONE_OF                   " of "
#define MOVE_FILES               " files."


#define SYMBOLIC_TEXT            "0 1 1 1 0 0 1 0 0 1 0 1 1 0 1 1 0 0 1 0 0 0 \
1 0 1 1 1 0 0 1 0 0 1 0 1 1 0 1 1 0 0 1 0 0 0 1 0 1 0 0 1 0 1 1 0 1 0 1 1 1 0 \
0 1 0 0 1 0 1 1 0 1 1 0 0 1 0 0 0 1 1 0 1 1 0 0 1 0 0 0 1 1 1 0 1 1 0 0 1 0 0 \
0 1 0 1 1 1 0 0 1 0 0 1 0 1 1 0 1 1 0 0 1 0 0 0 1 1 1 0 0 1 0 0 1 0 1 1 0 1 1 \
0 0 1 0 0 0 1 0 1 1 1 1 0 1 1 0 1 1 0 0 1 0 0 0 1 0 1 0 0 1 0 1 1 0 1 0 1 1 1 \
0 1 0 0 1 0 1 1 0 1 0 1 1 1 0 0 0 1 1 0 1 1 0 0 1 0 0 0 1 1 1 0 1 1 "

//File and folder paths
#define PATH_TEMP                "\\temp"
#define PATH_PROGRAM             "\\File Shredder"

//About window text
#define FILE_SHREDDER_VERSION   "File Shredder, version 001.00.030"
#define FILE_SHREDDER_COPYRIGHT "File Shredder, copyright © 2019 Alan Entwistle"
#define CURRENT_VERSION         "version 001.00"
#define ABOUT_USAGE             "Licence && terms of useage"
#define ABOUT_CREDITS           "Random numbers are generated using a Mersenne \
Twister © 2003 \r\nwritten by Richard J. Wagner, building on work by \
Makoto Matsumoto, \r\nTakuji Nishimura, and Shawn Cokus. Many thanks to \
these people \r\nfor making their excellent work so easily accessible."

#define ABOUT_LEGAL             "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT \r\n\
HOLDER 'AS IS' AND ANY EXPRESS OR IMPLIED \r\n\
WARRANTIES, LIMITED TO, THE IMPLIED WARRANTIES OF \r\n\
MERCHANTABILITY  AND FITNESS FOR A PARTICULAR \r\n\
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE \r\n\
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR \r\n\
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,\r\n\
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT \r\n\
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR \r\n\
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR \r\n\
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON \r\n\
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, \r\n\
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE \r\n\
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE \r\n\
OF THIS SOFTWARE, EVEN IF ADVISED OF THE \r\n\
POSSIBILITY OF SUCH DAMAGE."

#define CALENDAR_REG        "Registered at "
#define CALENDAR_ONT        " on the "
#define CALENDAR_COL        ":"
#define CALENDAR_JAN        "January "
#define CALENDAR_FEB        "February "
#define CALENDAR_MAR        "March "
#define CALENDAR_APR        "April "
#define CALENDAR_MAY        "May "
#define CALENDAR_JUN        "June "
#define CALENDAR_JLY        "July "
#define CALENDAR_AUG        "August "
#define CALENDAR_SEP        "September "
#define CALENDAR_OCT        "October "
#define CALENDAR_NOV        "November "
#define CALENDAR_DEC        "December "
#define CALENDAR_ST         "st of "
#define CALENDAR_ND         "nd of "
#define CALENDAR_RD         "rd of "
#define CALENDAR_TH         "th of "

#endif
