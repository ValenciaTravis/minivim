#ifndef __MY_APPLICATION__

#define __MY_APPLICATION__

#include <myWindow.h>
#include <myFile.h>
#include <deque>
#include <multiTrie.h>


class Application{
public:
    Application(const std::string &, bool, bool, int _line=-1);

    bool initialize();
    void initializeCppKeywords();
    void getPageContent();
    void printFile();
    void findEndOfLastLine(int &, int &);
    void insertMode();    
    void normalMode();
    void commandMode();
    void run();
    void up();
    void down();
    void left();
    void right();
    void backspace();
    void del();
    void enter();
    void insCh(char);
    void refresh();
    bool checkSave();
    void delLine();
    void moveBegin();
    void moveEnd();
    void addTab();
    void jmpLine(const std::string &);
    void printFileWithHighlight(const std::string &);
    void substitute(const std::string &);
    void substituteQuery(const std::string &);
    void refreshWithHighlight(const std::string &);
    void substituteReplace(const std::string &, const std::string &);
    void moveNxtWord();
    void movePreWord();
    void addBraket(char, char);
    void addSmallBracket();
    void addMiddleBracket();
    void addBigBracket();
    void addAngleBracket();
    void addSingleQuotation();
    void addDoubleQuotation();
    void inputMethod(const std::string &);
    void checkTab();
    void insSeparateCh(char c);
    void insWordCh(char c);
    void addWordsOfString(const std::string &);
    void delWordsOfString(const std::string &);
    void moveCursor(const Cursor &);
    void moveCursor(const Cursor &, const Cursor &);

// private:
    int line, cols, page_line, page_cols, command_cols, mode;
    MyWindow win;
    MyFile file;
    bool read_only, truncated, quit;
    Cursor cursor, win_cursor;
    std::vector<std::pair<int, std::string>> page_content;
    std::deque<std::string> command_history;
    multiTrie tr;
};

#endif