#ifndef __MY_WINDOW__
#define __MY_WINDOW__

#define INSERT_MODE 0
#define NORMAL_MODE 1
#define COMMAND_MODE 2

#include <ncurses.h>
#include <cursor.h>
#include <vector>
#include <string>

/**
 * Type: MyWindow
 * ------------------------------------------------------------
 * A type representing the current window displaying.
 */

class MyWindow{  
public:
    MyWindow(int line=10);
    bool initWindow();
    void clearLine(int);
    void printStr(const std::string &, bool isFile=true);
    void printLine(int, const std::string &);
    void printLineNumber(int);
    void printFile(const std::vector<std::pair<int, std::string>> &);
    void printInformation(int, const std::string &, const Cursor &);
    void printCommand(const std::string &);
    void moveCursor(const Cursor& cursor);
    void moveCursor(int, int);
    void refresh();
    char getchar();
    void printFileWithHighlight(const std::vector<std::pair<int, std::string>> &, const std::string &);
    void printStrWithHighlight(const std::string &, const std::string &);
    Cursor getInputMethodCursor();
    void createInputMethodWindow();
    void destoryInputMethodWindow();
    void showInputMethod(const std::vector<std::string> &);
    ~MyWindow();
// private:
    int line, cols, information_line, command_line, last_file_line;
    int num_cols, input_method_cols, input_method_rows;
    std::vector<std::string> keywords;
    WINDOW *win, *input_method_win;
};

#endif