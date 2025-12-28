#ifndef __MY_FILE__
#define __MY_FILE__

#include <cursor.h>
#include <fstream>
#include <string>
#include <vector>

class MyFile{
public:
    MyFile(const std::string &);
    bool open(bool);
    bool isVaildName(const std::string &);
    bool createDirectory();
    void insCh(char);
    void insStr(const std::string &);
    void delCh();
    void newLine();
    void delLine();
    void mergePreLine();
    void mergeNxtLine();
    void moveCursor(int, int);
    void moveCursor(const Cursor &);
    bool created();
    bool changed();
    bool save();
    Cursor findPre(const std::string &);
    Cursor findNxt(const std::string &);
    void replace(int, const std::string &);
    Cursor findPreWord();
    Cursor findNxtWord();
    std::string findCurrentWord1();
    std::string findCurrentWord2();

    Cursor file_cursor;
    std::vector<std::string> content;
    int isCreated;
    bool isChanged;
    std::string path, file_name;
};

#endif