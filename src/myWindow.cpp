#include <myWindow.h>
#include <utils.h>

#define REG_COLOR_NUM 1
#define CUS_COLOR_NUM 2
#define HIGHLIGHT_COLOR_NUM 3
#define KEYWORD_COLOR_NUM 4

/* Initialize the parameters in MyWindow. */
MyWindow::MyWindow(int _line){
    line = _line;
    last_file_line = line-3;
    information_line = line-2;
    command_line = line-1;
    num_cols = 8;
    input_method_cols = 29;
    input_method_rows = 7;
}

/* Initialize the Window, the return value is a boolean,
   which represents whether the initlization succeeded or not. */
bool MyWindow::initWindow(){

    initscr(); /* Start curses mode */

    if(line == -1) {
        getmaxyx(stdscr, line, cols);
        last_file_line = line-3;
        information_line = line-2;
        command_line = line-1;
    }

    raw();
    noecho();
    keypad(stdscr, true); /*stdscr is the regular window*/

    // init color
    start_color();
    init_pair(REG_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);
    init_pair(CUS_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);
    init_pair(HIGHLIGHT_COLOR_NUM, COLOR_RED, COLOR_YELLOW);
    init_pair(KEYWORD_COLOR_NUM, COLOR_MAGENTA, COLOR_BLACK);

    // set stdscr color
    wbkgd(stdscr, COLOR_PAIR(REG_COLOR_NUM));
    wrefresh(stdscr);

    // check window size is illegal
    // LINES and COLS store the total line and cols of stdscr
    if (LINES < line) {
        fprintf(stderr, "window line size is small than the size needed.");
        endwin();
        return false;
    }
    cols = COLS;

    win = newwin(line, cols, 0, 0);
    keypad(win, true);
    wbkgd(win, COLOR_PAIR(CUS_COLOR_NUM)); /*set customized window color*/
    wrefresh(win);

    return true;
}

/* Clear a line on the window without refreshing */
void MyWindow::clearLine(int pos){
    wmove(win, pos, 0);
    wclrtoeol(win);
}

/* Print a line and Bold and color the keywords in the string if isFile is true. */
void MyWindow::printStr(const std::string &str, bool isFile){
    std::string output_str;
    for(int i=0;i<str.size();i++){
        output_str += str[i];
        if(str[i] == '%') output_str += '%'; // Escape character
    }
    int pos, y;
    getyx(win, pos, y);
    wprintw(win, output_str.c_str());
    if(!isFile) return; // Don't highlight if it is Information line or Command line.
    for(auto& word : keywords){
        for(int p = str.find(word, 0);p != std::string::npos;p = str.find(word, p+1)) {
            // Check if we find a substring instead of a word.
            if(utils::notSeparate(str[p + word.size()])) continue;
            if(p != 0 && utils::notSeparate(str[p-1])) continue;
            // Bold and color the keyword.
            moveCursor(Cursor(pos, p + num_cols));
            wchgat(win, word.size(), A_BOLD, KEYWORD_COLOR_NUM, NULL);
        }
    }
}

/* Print a string with highlight on the some words. */
void MyWindow::printStrWithHighlight(const std::string &str, const std::string& word){
    std::string output_str;
    for(int i=0;i<str.size();i++){
        output_str += str[i];
        if(str[i] == '%') output_str += '%'; // Escape character
    }
    int pos, y;
    getyx(win, pos, y);
    wprintw(win, output_str.c_str());
    for(auto& word : keywords){
        for(int p = str.find(word, 0);p != std::string::npos;p = str.find(word, p+1)) {
            // Check if we find a substring instead of a word.
            if(utils::notSeparate(str[p + word.size()])) continue;
            if(p != 0 && utils::notSeparate(str[p-1])) continue;
            // Bold and color the keyword.
            moveCursor(Cursor(pos, p + num_cols));
            wchgat(win, word.size(), A_BOLD, KEYWORD_COLOR_NUM, NULL);
        }
    }
    // Highlight of the word has a higher priority than keywords.
    for(int p = str.find(word, 0);p != std::string::npos;p = str.find(word, p+1)) {
        // We don't care if it is a substring or not when highlighting.
        moveCursor(Cursor(pos, p + num_cols));
        wchgat(win, word.size(), A_BOLD, HIGHLIGHT_COLOR_NUM, NULL);
    }
}

/* Print a line which is a part of the file. */
void MyWindow::printLine(int pos, const std::string &str){
    clearLine(pos);
    printStr(str);
}

/* Print Line number. */
void MyWindow::printLineNumber(int pos){
    printStr(utils::int2str(pos, num_cols-3));
    waddch(win, '|');
    waddch(win, ' ');
}

/* Print a File on the screen with line number on. */
void MyWindow::printFile(const std::vector<std::pair<int, std::string>> &file){
    for(int i=0;i<file.size();i++){
        clearLine(i);
        // Print the line number if and only if it is 'the first line' of the current line.
        if(i == 0 || file[i].first != file[i-1].first) printLineNumber(file[i].first+1);
        else printLineNumber(0); // No line number.
        printStr(file[i].second);
    }
    // If the number of the lines of the file is less than the number should be represented
    // Add empty lines.
    for(int i=file.size();i<=last_file_line;i++){
        clearLine(i);
        printLineNumber(0);
    }
    refresh();
}


/* Print a File on the screen with line number on and highlight on some words. */
void MyWindow::printFileWithHighlight(const std::vector<std::pair<int, std::string>> &file, const std::string& word){
    for(int i=0;i<file.size();i++){
        clearLine(i);
        // Print the line number if and only if it is 'the first line' of the current line.
        if(i == 0 || file[i].first != file[i-1].first) printLineNumber(file[i].first+1);
        else printLineNumber(0); // No line number.
        printStrWithHighlight(file[i].second, word);
    }
    // If the number of the lines of the file is less than the number should be represented
    // Add empty lines.
    for(int i=file.size();i<=last_file_line;i++){
        clearLine(i);
        printLineNumber(0);
    }
    refresh();
}

/* Print the information line with mode and file name and cursor. */
void MyWindow::printInformation(int mode, const std::string &file_name, const Cursor &cursor){
    std::string str = "Mode: ";
    if(mode == INSERT_MODE) str += "INSERT";
    else if(mode == NORMAL_MODE) str += "NORMAL";
    else if(mode == COMMAND_MODE) str += "COMMAND";
    str += ", File: " + file_name + ", ";
    str += "Row:" + utils::int2str(cursor.x+1, 2, true) + ", Col:" + utils::int2str(cursor.y+1, 2, true);
    printLine(information_line, str);
    refresh();
}

/* Print the command line. */
void MyWindow::printCommand(const std::string &command){
    std::string str = ":";
    str += command;
    // for(int i=0;i<command.size();i++) {
    //     str += command[i];
    //     if(command[i] == '%') str += '%';
    // }
    clearLine(command_line);
    printStr(str, false);
    refresh();
}

/* Move cursor to the place where the 'cursor' is.*/
void MyWindow::moveCursor(const Cursor& cursor){
    wmove(win, cursor.x, cursor.y);
    refresh();
}

/* Move the cursor to (x, y) */
void MyWindow::moveCursor(int x, int y){
    moveCursor(Cursor(x, y));
}

/* Refresh the window. */
void MyWindow::refresh(){
    wrefresh(win);
}

/* Read a single char from the keyboard and return the char. */
char MyWindow::getchar(){
    return getch();
}

/* Find the postion of the up left corner of the input method window. */
Cursor MyWindow::getInputMethodCursor(){
    int x, y;
    getyx(win, x, y);
    if(x + input_method_rows > last_file_line) x = x - input_method_rows;
    else x = x+1;
    if(y + input_method_cols >= cols) y = cols - input_method_cols;
    else y = y;
    return Cursor(x, y); 
}

/* Create a subwindow for the input method. */
void MyWindow::createInputMethodWindow(){
    Cursor cursor = getInputMethodCursor();
    input_method_win = newwin(input_method_rows, input_method_cols, cursor.x, cursor.y);
    box(input_method_win, 0, 0);
    wrefresh(input_method_win);
}

/* Destory the subwindow for the input method. */
void MyWindow::destoryInputMethodWindow(){
    delwin(input_method_win);
}

/* Print the window for the input method. */
void MyWindow::showInputMethod(const std::vector<std::string> &words){
    for(int i=0;i<input_method_cols;i++){
        wmove(input_method_win, i, 0);
        wclrtoeol(input_method_win);
    }
    // Generate the boarder for the input method.
    box(input_method_win, 0, 0);
    wmove(input_method_win, 1, 1);
    for(int i=0;i<words.size();i++){
        wmove(input_method_win, i + 1, 1);
        // Print the word number
        waddch(input_method_win, i + '1');
        waddch(input_method_win, '.');
        // Fold when the word is too long
        if(words[i].size() > input_method_cols - 4){
            wprintw(input_method_win, words[i].substr(0, input_method_cols - 7).c_str());
            waddch(input_method_win, '.');
            waddch(input_method_win, '.');
            waddch(input_method_win, '.');
        }else wprintw(input_method_win, words[i].c_str());
    }
    wrefresh(input_method_win);
}

/* Destory the whole window when exit. */
MyWindow::~MyWindow(){
    endwin();
}