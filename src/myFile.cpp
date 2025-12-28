#include <myFile.h>
#include <regex>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <utils.h>

/* Initialize the parameters in MyFile. */
MyFile::MyFile(const std::string& _path){
    path = _path;
    file_name = "";
    isCreated = 0;
    isChanged = 0;
}

/* Open a file. */
bool MyFile::open(bool truncated){
    
    std::vector<int> pos;
    pos.push_back(-1);
    for(int i=0;i<path.size();i++) if(path[i] == '/') pos.push_back(i);
    pos.push_back(path.size());
    for(int i=0;i<pos.size()-1;i++) {
        if(isVaildName(path.substr(pos[i]+1, pos[i+1]-pos[i]-1))) continue;
        std::cerr << "Invaild Path." << std::endl;
        return false;
    }

    // get file name
    int p = pos[pos.size()-2];
    if(p == std::string::npos) file_name = path;
    else file_name = path.substr(p+1);

    // printf("filename = %s\n", file_name.c_str());

    // open the file and read the content
    if(!truncated && access(path.c_str(), NULL) == 0){
        // printf("file exists.\n");
        std::ifstream file;
        file.open(path);
        if(!file.is_open()) return false;
        std::string str;
        while(!file.eof()) {
            getline(file, str);
            if(str.size() && str[str.size()-1] == 13) content.push_back(str.substr(0, str.length()-1));
            else content.push_back(str);
        }
        file.close();
    }else content.push_back(""), isCreated = true;

    // for(auto str : content) std::cout << str << std::endl;

    return true;
}

/* Check if a directory or a file name is vaild */
bool MyFile::isVaildName(const std::string &str){
    std::regex reg_express("[\\/:*?\"<>|]");
    return !std::regex_search(str, reg_express);
}

/* Create directory by path.  */
bool MyFile::createDirectory(){
    int len = path.length();
    char tmp_dir_path[256] = {};
    int cnt = 0;
    for(int i=0;i<len;i++){
        tmp_dir_path[i] = path[i];
        if(tmp_dir_path[i] != '/') continue;
        ++cnt;
        tmp_dir_path[i] = 0;
        if(access(tmp_dir_path, NULL) != 0){
            int ret = mkdir(tmp_dir_path, 0755);
            if(ret == -1) {
                std::cerr << "Failed to create Directory. Please check your input path." << std::endl;
                return false;
            }
        }
        tmp_dir_path[i] = '/';
    }
    return true;
}

/* Add a char at the file_cursor. */ 
void MyFile::insCh(char c){
    isChanged = true;
    int x = file_cursor.x, y = file_cursor.y;
    std::string str1 = content[x].substr(0, y), str2 = content[x].substr(y);
    content[x] = str1 + c + str2;
    file_cursor.y++;
}

/* Add a string at the file_cursor. */
void MyFile::insStr(const std::string &str){
    isChanged = true;
    int x = file_cursor.x, y = file_cursor.y;
    std::string str1 = content[x].substr(0, y), str2 = content[x].substr(y);
    content[x] = str1 + str + str2;
    file_cursor.y += str.size();
}

/* Delete a char at the file_cursor */
void MyFile::delCh(){
    isChanged = true;
    int x = file_cursor.x, y = file_cursor.y;
    std::string str1 = content[x].substr(0, y-1), str2 = content[x].substr(y);
    content[x] = str1 + str2;
    file_cursor.y--;
}

/* Merge the line at the file_cursor and the previous line
   and move the file_cursor to the end of the previous line. */
void MyFile::mergePreLine(){
    isChanged = true;
    int x = file_cursor.x, y = file_cursor.y;
    int yy = content[x-1].length();
    // printf("len = %d %s\n", (int)content[x-1].length(), content[x-1].c_str());
    content[x-1] += content[x];
    delLine();
    moveCursor(x-1, yy);
}

/* Add a new line at the file_cursor. If there is any space
   at the start of the last line, copy it to this new line. */
void MyFile::newLine(){
    isChanged = true;
    int x = file_cursor.x, y = file_cursor.y;
    std::string str1 = content[x].substr(0, y), str2 = "";
    for(int i=0;i<str1.size();i++){
        if(str1[i] != ' ') break;
        str2 += ' ';
    }
    int new_y = str2.size();
    str2 += content[x].substr(y);
    content[x] = str1;
    content.insert(content.begin() + x + 1, str2);
    moveCursor(x+1, new_y);
}

/* Delete the whole line at the file_cursor. */
void MyFile::delLine(){
    // There must be at least one line.
    if(content.size() == 1 && content[0].size() == 0) return;
    isChanged = true;
    if(content.size() == 1) {content[0] = ""; moveCursor(0, 0); return;}
    int x = file_cursor.x;
    // If the line deleted is the last line, jump to the end of the last line after deleting.
    if(x == content.size() - 1) {content.pop_back(); moveCursor(content.size()-1, content.back().size()); return;}
    for(int i=x+1;i<content.size();i++) std::swap(content[i-1], content[i]);
    content.pop_back();
    // Otherwise, jump the the end of next line.
    moveCursor(x, content[x].size());
}

/* Move the file_cursor to (x, y) */
void MyFile::moveCursor(int x, int y){
    y = std::min(y, (int)content[x].size());
    file_cursor = Cursor(x, y);
}

/* Move the file_cursor to cursor */
void MyFile::moveCursor(const Cursor& cursor){
    file_cursor = cursor;
    file_cursor.y = std::min(file_cursor.y, (int)content[file_cursor.x].size());
}

/* Return a boolean represents whether the file is created now.*/
bool MyFile::created(){
    return isCreated;
}

/* Return a boolean represents whether the file is changed after last save. */
bool MyFile::changed(){
    return isChanged;
}

/* Save the file, return a boolean represents 
   whether the save is successful. */
bool MyFile::save(){

    if(isCreated) {
        if(!createDirectory()) return false;
        isCreated = false;
    }

    std::ofstream file;
    file.open(path, std::ios::ate);
    
    if(!file.is_open()) return false;

    for(int i=0;i<(int)content.size()-1;i++) file << content[i] << std::endl;
    file << content.back();

    file.close();
    
    isChanged = false;

    return true;
}

/* Find the last occurence of word before the file_cursor
   if there is no occurence, return (-1, -1). */
Cursor MyFile::findPre(const std::string& word){
    int x = file_cursor.x, y = file_cursor.y;
    std::string current_line = content[x].substr(0, y+word.size()-1);
    int pos = current_line.rfind(word);
    if(pos != std::string::npos) return Cursor(x, pos);
    for(int i=x-1;i>=0;i--){
        pos = content[i].rfind(word);
        if(pos != std::string::npos) return Cursor(i, pos);
    }
    return Cursor(-1, -1);
}

/* Find the first occurence of word before the file_cursor
   if there is no occurence, return (-1, -1). */
Cursor MyFile::findNxt(const std::string& word){
    int x = file_cursor.x, y = file_cursor.y;
    std::string current_line = content[x].substr(std::min(y+1, (int)content[x].size()));
    // printf("#####y = %d, %s\n", std::min(y+1, (int)content[x].size()), content[x].substr(3).c_str());
    int pos = current_line.find(word);
    if(pos != std::string::npos) return Cursor(x, pos+y+1);
    for(int i=x+1;i<content.size();i++){
        pos = content[i].find(word);
        if(pos != std::string::npos) return Cursor(i, pos);
    }
    return Cursor(-1, -1);
}

void MyFile::replace(int len, const std::string &subs){
    isChanged = true;
    int x = file_cursor.x, y = file_cursor.y;
    std::string str1 = content[x].substr(0, y), str2 = content[x].substr(y+len);
    content[x] = str1 + subs + str2;
}

Cursor MyFile::findNxtWord(){
    int x = file_cursor.x, y = file_cursor.y;
    std::pair<int, int> ret = utils::findCurrentWord(content[x], y);
    ret = utils::findNxtWord(content[x], ret.first + ret.second);
    if(ret.second != 0) return Cursor(x, ret.first);
    for(int i=x+1;i<content.size();i++){
        ret = utils::findNxtWord(content[i], 0);
        if(ret.second != 0) return Cursor(i, ret.first);
    }
    return Cursor(-1, -1);
}

Cursor MyFile::findPreWord(){
    int x = file_cursor.x, y = file_cursor.y;
    std::pair<int, int> ret = utils::findCurrentWord(content[x], y);
    ret = utils::findPreWord(content[x], ret.first-1);
    if(ret.second != 0) return Cursor(x, ret.first);
    for(int i=x-1;i>=0;i--){
        if(content[i].size() == 0) continue;
        ret = utils::findPreWord(content[i], content[i].size()-1);
        if(ret.second != 0) return Cursor(i, ret.first);
    }
    return Cursor(-1, -1);
}

/* Find the current word. The file_cursor must be inside the words. */
std::string MyFile::findCurrentWord1(){
    int x = file_cursor.x, y = file_cursor.y;
    std::pair<int, int> result = utils::findCurrentWord(content[x], y);
    if(result.second == 0) return "";
    return content[x].substr(result.first, result.second);
}

/* Find the current word. The file_cursor may be after the words
   but can not be the first of the words.*/
std::string MyFile::findCurrentWord2(){
    int x = file_cursor.x, y = file_cursor.y;
    if(y == 0) return "";
    std::pair<int, int> result = utils::findCurrentWord(content[x], y-1);
    if(result.second == 0) return "";
    return content[x].substr(result.first, result.second);
}