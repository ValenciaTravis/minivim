#include <fstream>
#include <myApplication.h>
#include <ncurses.h>
#include <utils.h>
#include <assert.h>

#define DOWN 2
#define UP 3
#define LEFT 4
#define RIGHT 5
#define BACKSPACE 7
#define DEL 74
#define ENTER 10
#define ESC 27
#define TAB 9

/* Initialize the window and the file. */
Application::Application(const std::string &path, bool _truncated, bool _read_only, int _line) : win(_line), file(path){
    win.initWindow();
    truncated = _truncated;
    read_only = _read_only;
    line = LINES;
    cols = COLS;
    page_cols = COLS - 8;
    page_line = line - 2;
    command_cols = COLS - 1;
    quit = false;
}

/* Initialize all the cpp keywords and add all the words in the file into the multiTrie.
   Initialize the whole screen. */
bool Application::initialize(){
    if(!file.open(truncated)) return false;
    if(truncated) file.isChanged = true;
    if(file.file_name.length() >= 4 && file.file_name.substr(file.file_name.length()-4) == ".cpp") 
        initializeCppKeywords();

    Cursor ret_cursor;
    file.moveCursor(0, 0);
    if(file.findCurrentWord1().size() != 0) ret_cursor = Cursor(0, 0);
    else ret_cursor = file.findNxtWord();

    while(ret_cursor.x != -1){
        file.moveCursor(ret_cursor);
        std::string str = file.findCurrentWord1();
        tr.insert(str);
        ret_cursor = file.findNxtWord();
    }

    mode = 1;
    cursor = Cursor(0, 0);
    file.moveCursor(0, 0);
    printFile();
    win.printInformation(mode, file.file_name, file.file_cursor);
    win.moveCursor(win_cursor);
    return true;
}

/* Initialize (possibly, all?) the keywords in cpp. */
void Application::initializeCppKeywords(){
    win.keywords = {"if", "else", "while", "signed", "throw", "union", "this", "int", "char", "double",
                "unsigned", "const", "goto", "virtual", "for", "float", "break", "auto", "class", 
                "operator", "case", "do", "long", "typedef", "static", "friend", "template", "default",
                "new", "void", "register", "extern", "return", "enum", "inline", "try", "short", "continue",
                "sizeof", "switch", "private", "protected", "asm", "catch", "delete", "public", "volatile", 
                "struct", "include", "using", "namespace"};
    for(auto& word : win.keywords) tr.insert(word);
    std::ifstream file;
    file.open("words_alpha.txt");
    std::string str;
    assert(file.is_open());
    while(!file.eof()) {
        getline(file, str);
        if(str.size() && str[str.size()-1] == 13) tr.insert(str.substr(0, str.length()-1));
        else tr.insert(str);
    }
}

/* Print the File. */
void Application::printFile(){
    getPageContent();
    // for(auto [x, y] : page_content) printf("%d %s\n", x, y.c_str());
    win.printFile(page_content);
}

/* Print the File with highlight. */
void Application::printFileWithHighlight(const std::string& word){
    getPageContent();
    win.printFileWithHighlight(page_content, word);
}

/* Get the part of the file which show be represented on the screen. */
void Application::getPageContent(){
    page_content.clear();
    int x = file.file_cursor.x, y = file.file_cursor.y;
    int win_x = win_cursor.x + 1;
    y -= y % page_cols + 1;
    findEndOfLastLine(x, y); 
    int cnt = 1;
    // Count the Lines before (including) the cursor line.
    while(cnt < win_x && x >= 0) {
        ++cnt; y -= page_cols;
        findEndOfLastLine(x, y);
    }
    win_cursor = Cursor(cnt-1, file.file_cursor.y % page_cols + win.num_cols);
    if(cnt == win_x){ 
        // If there exists win_x lines before (including) the cursor line.
        page_content.resize(win_x);
        int x = file.file_cursor.x, y = file.file_cursor.y;
        for(int i=cnt-1;i>=0;i--){
            y -= y % page_cols;
            page_content[i] = std::make_pair(x, file.content[x].substr(y, page_cols));
            y--;
            findEndOfLastLine(x, y);
        }
        x = file.file_cursor.x, y = file.file_cursor.y;
        y -= y % page_cols; y += page_cols;
        if(y + 1 > file.content[x].size()) x++, y = 0;
        for(int i=win_x;i < page_line && x < file.content.size();i++){
            page_content.push_back(std::make_pair(x, file.content[x].substr(y, page_cols)));
            y += page_cols;
            if(y + 1 > file.content[x].size()) x++, y = 0;
        }
    }else{ 
        // If there doesn't exist win_x lines before (including) the cursor line.
        // Which means that we only have to start from the start of the file.
        for(int i=0;i<file.content.size() && page_content.size() < page_line;i++){
            int len = file.content[i].size();
            if(len == 0) {page_content.push_back(std::make_pair(i, "")); continue;}
            if(i == file.file_cursor.x && len == file.file_cursor.y) len++;
            int pos = 0;
            while(pos < len && page_content.size() < page_line) {
                page_content.push_back(std::make_pair(i, file.content[i].substr(pos, page_cols))), pos += page_cols;
            }
        }
    }
}

/* Find the End of the last line. */
void Application::findEndOfLastLine(int &x, int &y){
    if(y >= 0) return;
    x--;
    if(x >= 0) y = std::max((int)file.content[x].size()-1, 0);
}

/* Move the Cursor on the screen with a start place and an end place. */
void Application::moveCursor(const Cursor &cursor_start, const Cursor &cursor_end){
    int x = cursor_start.x, y = cursor_start.y;
    int new_x = cursor_end.x, new_y = cursor_end.y;
    if(Cursor(x, y) <= Cursor(new_x, new_y)){
        y -= y % page_cols; y += page_cols;
        if(x == new_x && new_y < y) return;
        if(y + 1 > file.content[x].size()) x++, y = 0;
        win_cursor.x++;
        while(x < new_x || (x == new_x && y + page_cols - 1 < new_y)){
            y += page_cols;
            if(y + 1 > file.content[x].size()) x++, y = 0;
            win_cursor.x++;
        }
        if(win_cursor.x >= page_line) win_cursor.x = page_line - 1;
    }else{
        y -= y % page_cols;
        while(x > new_x || (x == new_x && y > new_y)){
            win_cursor.x--;
            y--;
            findEndOfLastLine(x, y);
            y -= y % page_cols;
        }
        if(win_cursor.x < 0) win_cursor.x = 0;
    }
}

/* Move the cursor with the cursor get from the file. */
void Application::moveCursor(const Cursor &cursor){
    Cursor cursor_start = file.file_cursor;
    file.moveCursor(cursor);
    Cursor cursor_end = file.file_cursor;
    moveCursor(cursor_start, cursor_end);
}

/* Print and Refresh everything on the screen. */
void Application::refresh(){
    printFile();
    win.printInformation(mode, file.file_name, file.file_cursor);
    win.moveCursor(win_cursor);
}

/* Print and Refresh everything on the screen with highlight on some words. */
void Application::refreshWithHighlight(const std::string &word){
    printFileWithHighlight(word);
    win.printInformation(mode, file.file_name, file.file_cursor);
    win.moveCursor(win_cursor);
}

/* Move the cursor up, if possible. */
void Application::up(){
    if(cursor.x == 0) return; // Return if it is the start of the file
    cursor.x--;
    moveCursor(cursor);
    refresh();
}

/* Move the cursor down, if possible. */
void Application::down(){
    if(cursor.x == file.content.size() - 1) return; // Return if it is the end of the file
    cursor.x++;
    moveCursor(cursor);
    refresh();
}

/* Move the cursor right, if possible. */
void Application::right(){
    cursor = file.file_cursor;
    // Return if it is end of the last line
    if(cursor.x == file.content.size() - 1 && cursor.y == file.content.back().size()) return;
    cursor.y++;
    if(cursor.y > file.content[cursor.x].size()) cursor.x++, cursor.y = 0;
    moveCursor(cursor);
    refresh();
}

/* Move the cursor Left, if possible. */
void Application::left(){
    cursor = file.file_cursor;
    // Return if it is the start of the first line
    if(cursor.x == 0 && cursor.y == 0) return;
    cursor.y--;
    if(cursor.y < 0) cursor.x--, cursor.y = file.content[cursor.x].size();
    moveCursor(cursor);
    refresh();
}

/* Delete the char before the cursor. */
void Application::backspace(){
    cursor = file.file_cursor;
    // Return if it is the start of the first line
    if(cursor.x == 0 && cursor.y == 0) return;  
    Cursor cursor_start = file.file_cursor;
    if(cursor.y == 0) { // If it is the start of the current line.
        // Delete the words on the current line and last line from the multiTrie.
        delWordsOfString(file.content[cursor.x]);
        delWordsOfString(file.content[cursor.x-1]);
        if(file.content[cursor.x-1].size() % page_cols != 0
           || !file.content[cursor.x-1].size())
            win_cursor.x = std::max(win_cursor.x-1, 0);
        file.mergePreLine();
        // Add the words on the current line back to the multiTrie. 
        addWordsOfString(file.content[cursor.x-1]);
    }
    else {
        // Delete the words on the current line from the multiTrie.
        delWordsOfString(file.content[cursor.x]);
        file.delCh();
        // Add the words on the current line back to the multiTrie. 
        addWordsOfString(file.content[cursor.x]);
        Cursor cursor_end = file.file_cursor;
        moveCursor(cursor_start, cursor_end);
    }
    cursor = file.file_cursor;
    refresh();
}

/* Delete the char after the cursor. */
void Application::del(){
    cursor = file.file_cursor;
    // Return if it is the end of the File
    if(cursor.x == file.content.size() - 1 && cursor.y == file.content.back().size()) return;
    right(); backspace();
}

/* Add a new line after the cursor. */
void Application::enter(){
    int x = file.file_cursor.x;
    if(file.file_cursor.y % page_cols != 0 || file.file_cursor.y == 0)
        win_cursor.x = std::min(win_cursor.x+1, page_line-1);
    // Delete the words on the current line.
    delWordsOfString(file.content[x]);
    file.newLine();
    // Add the words on the current line and the next line.
    addWordsOfString(file.content[x]);
    addWordsOfString(file.content[x+1]);
    cursor = file.file_cursor;
    refresh();
}

/* Add the words in the string into the multiTrie. */
void Application::addWordsOfString(const std::string &str){
    for(auto result = utils::findNxtWord(str, 0); result.second != 0;
        result = utils::findNxtWord(str, result.first + result.second)){
        // The first char of the word can not be a number.
        if(isdigit(str[result.first])) continue;
        tr.insert(str.substr(result.first, result.second));

    }
}

/* Delete the words in the string from the multiTrie. */
void Application::delWordsOfString(const std::string &str){
    for(auto result = utils::findNxtWord(str, 0); result.second != 0;
        result = utils::findNxtWord(str, result.first + result.second)){
        // The first char of the word can not be a number.
        if(isdigit(str[result.first])) continue;
        tr.del(str.substr(result.first, result.second));

    }
}

/* Add a char at the file_cursor. */
void Application::insCh(char c){
    Cursor cursor_start = file.file_cursor;
    int x = file.file_cursor.x;
    delWordsOfString(file.content[x]);
    file.insCh(c);
    addWordsOfString(file.content[x]);
    Cursor cursor_end = file.file_cursor;
    moveCursor(cursor_start, cursor_end);
    cursor = file.file_cursor;
    refresh();
}

/* Add a tab at the current line. */
void Application::addTab(){
    int y = file.file_cursor.y;
    for(int i=y%4;i<4;i++) file.insCh(' ');
    cursor = file.file_cursor;
    refresh();
}

/* Add a bracket with auto-completing another half. */
void Application::addBraket(char left, char right){
    int x = file.file_cursor.x, y = file.file_cursor.y;
    if(y < file.content[x].size() && file.content[x][y] == right) {
        insCh(left);
        return;
    }
    insCh(left), insCh(right);
    file.moveCursor(file.file_cursor.x, file.file_cursor.y-1);
    cursor = file.file_cursor;
    refresh();
}

/* Add small Bracket. */
void Application::addSmallBracket(){
    addBraket('(', ')');
}

/* Add middle Bracket. */
void Application::addMiddleBracket(){
    addBraket('[', ']');
}

/* Add big Bracket. */
void Application::addBigBracket(){
    addBraket('{', '}');
}

/* Add angle Bracket. */
void Application::addAngleBracket(){
    // addBraket('<', '>');
    insCh('<');
}

/* Add single quotation. */
void Application::addSingleQuotation(){
    addBraket('\'', '\'');
}

/* Add Double quotation. */
void Application::addDoubleQuotation(){
    addBraket('\"', '\"');
}

/* Execute the input method. */
void Application::inputMethod(const std::string &word){
    win.createInputMethodWindow();
    win.moveCursor(win_cursor);

    // When completing the current word, current word should not be counted in the multiTrie.
    tr.del(word);

    // Get the words from the multiTrie.
    int page_cnt = 0, page_rows = win.input_method_rows-2;
    std::vector<std::string> words = tr.findPrefix(word, 0, page_rows-1);
    
    // If words is empty, prompt the User that there is no words.
    if(words.size() == 0) {
        wmove(win.input_method_win, win.input_method_rows / 2, (win.input_method_cols - 17) / 2);
        wprintw(win.input_method_win, "No word matches.");
        wrefresh(win.input_method_win);
        win.moveCursor(win_cursor);
        char c = win.getchar();
        if(c == TAB) { // Two tabs.
            win.destoryInputMethodWindow();
            tr.insert(word); // Add the word back to the multiTrie.
            addTab(); 
            return;
        }
        win.destoryInputMethodWindow();
        tr.insert(word); // Add the word back to the multiTrie.
        refresh();
        return;
    }

    // If there is only one word, immediately complete the word.
    if(words.size() == 1){
        // If the word is already completed, add a tab.
        if(word.size() == words[0].size()) {
            win.destoryInputMethodWindow();
            tr.insert(word);
            addTab();
            return;
        }
        Cursor cursor_start = file.file_cursor;
        file.insStr(words[0].substr(word.size()));
        Cursor cursor_end = file.file_cursor;
        moveCursor(cursor_start, cursor_end);
        tr.insert(words[0]);
        win.destoryInputMethodWindow();
        refresh();
        return;
    }

    // Show the words list on the screen
    win.showInputMethod(words);
    win.moveCursor(win_cursor);

    char c = win.getchar();

    // Two tabs.
    if(c == TAB) {
        win.destoryInputMethodWindow();
        tr.insert(word);
        addTab();
        return;
    }

    for(;c != ESC;c = win.getchar()){
        if('1' <= c && c <= '1' + words.size() - 1){
            // Complete the word with the number chosen.
            Cursor cursor_start = file.file_cursor;
            file.insStr(words[c - '1'].substr(word.size()));
            Cursor cursor_end = file.file_cursor;
            moveCursor(cursor_start, cursor_end);
            tr.insert(words[c - '1']);
            break;
        } else if(c == UP || c == LEFT || c == '-'){
            // Go to the previous page of words.
            if(page_cnt == 0) continue; // If it is the start page of the words.
            page_cnt--;
            words = tr.findPrefix(word, page_cnt * page_rows, page_cnt * page_rows + page_rows - 1);
            win.showInputMethod(words);
            win.moveCursor(win_cursor);
        }
        else if(c == DOWN || c == RIGHT || c == '='){
            // Go to the next page of words.
            page_cnt++;
            // If it is the last page of the words.
            std::vector<std::string> tmp = tr.findPrefix(word, page_cnt * page_rows, page_cnt * page_rows + page_rows - 1);
            if(tmp.size() == 0) {page_cnt--; continue;}
            words = tmp;
            win.showInputMethod(words);
            win.moveCursor(win_cursor);
        }
    }    
    
    // Add the word back if the user didn't use the completion.
    if(c == ESC) tr.insert(word);

    win.destoryInputMethodWindow();
    refresh();
}

/* Check if there exists a word at the cursor.
   If there isn't, add a tab, otherwise jump to the end of the word
   and call the input method.  */
void Application::checkTab(){
    int x = file.file_cursor.x, y = file.file_cursor.y;
    if(y == 0) {addTab(); return;}
    std::pair<int, int> result = utils::findCurrentWord(file.content[x], y-1);
    if(result.second == 0 || isdigit(file.content[x][result.first])) {addTab(); return;}
    file.moveCursor(x, result.first + result.second);
    cursor = file.file_cursor;
    refresh();
    inputMethod(file.content[x].substr(result.first, result.second));
}

/* Insert mode. */
void Application::insertMode(){
    while(true){
        char c = win.getchar();
        // printf("%d\n", (int)c);

        if(c == BACKSPACE) backspace();
        else if(c == DEL) del();
        else if(c == UP) up();
        else if(c == DOWN) down();
        else if(c == LEFT) left();
        else if(c == RIGHT) right();
        else if(c == ENTER) enter();
        else if(c == TAB) checkTab();
        else if(c == '(') addSmallBracket();
        else if(c == '[') addMiddleBracket();
        else if(c == '{') addBigBracket();
        else if(c == '<') addAngleBracket();
        else if(c == '\'') addSingleQuotation();
        else if(c == '\"') addDoubleQuotation();
        else if(32 <= c && c <= 126) insCh(c);
        else if(c == ESC) {mode = NORMAL_MODE; break;}
    }
}

/* Delete the current line. */
void Application::delLine(){
    Cursor cursor_start = file.file_cursor;
    delWordsOfString(file.content[file.file_cursor.x]);
    file.delLine();
    Cursor cursor_end = file.file_cursor;
    moveCursor(cursor_start, cursor_end);
    cursor = file.file_cursor;
    refresh();
}

/* Move to the begin of the current line. */
void Application::moveBegin(){
    int x = file.file_cursor.x, y = 0;
    while(y < file.content[x].size() && file.content[x][y] == ' ') y++;
    if(y >= file.file_cursor.y) y = 0;
    cursor = Cursor(x, y);
    moveCursor(cursor);
    refresh();
}

/* Move to the end of the current line. */
void Application::moveEnd(){
    cursor = Cursor(file.file_cursor.x, file.content[file.file_cursor.x].size());
    moveCursor(cursor);
    refresh();
}

/* Move to the start of next word, if exists. */
void Application::moveNxtWord(){
    Cursor ret_cursor = file.findNxtWord();
    if(ret_cursor.x == -1) return;
    moveCursor(ret_cursor);
    cursor = file.file_cursor;
    refresh();
}

/* Move the the start of the last word, if exists. */
void Application::movePreWord(){
    Cursor ret_cursor = file.findPreWord();
    if(ret_cursor.x == -1) return;
    moveCursor(ret_cursor);
    cursor = file.file_cursor;
    refresh();
}

/* Normal mode. */
void Application::normalMode(){
    char c = win.getchar(), last = '\0';
    while(true){

        if(c == UP) up();
        else if(c == DOWN) down();
        else if(c == LEFT) left();
        else if(c == RIGHT) right();
        else if(c == 'i' && !read_only) {mode = INSERT_MODE; break;}
        else if(c == ':') {mode = COMMAND_MODE; break;}
        else if(c == 'd' && last == 'd' && !read_only) delLine(), c = '\0';
        else if(c == '0') moveBegin();
        else if(c == '$') moveEnd();
        else if(c == 'w') moveNxtWord();
        else if(c == 'b') movePreWord();

        last = c;
        c = win.getchar();
    }
}

/* Check if you have changed the file before the last save. */
bool Application::checkSave(){
    if(!file.isChanged) return true;
    while(true){
        std::string command = "You haven't saved yet. Are to sure to quit? [y/n]: ";
        win.printCommand(command);
        char c = win.getchar();
        for(;c!=ENTER;c=win.getchar()){
            if(c == BACKSPACE) {
                if(command.size() > 51) command = command.substr(0, command.size()-1); 
                win.printCommand(command); continue;
            }
            if(32 > c || c > 126) continue;
            if(command.size() < command_cols) command += c;
            win.printCommand(command);
        }
        if(command.size() != 52) continue;
        if(command[51] == 'y' || command[51] == 'Y') return true;
        if(command[51] == 'n' || command[51] == 'N') return false;
    }
    return false;
}

/* Jump to the line by line number, if exists. */
void Application::jmpLine(const std::string &command){
    int num = 0;
    for(int i=0;i<command.size();i++){
        if(!isdigit(command[i])) return;
        num = num * 10 + command[i] - '0';
    }
    if(num <= 0 || num > file.content.size()) return;
    cursor = Cursor(num-1, 0);
    moveCursor(cursor);
    refresh();
}

/* Find all the word in the file, and highlight them. */
void Application::substituteQuery(const std::string &word){
    int x = file.file_cursor.x, y = file.file_cursor.y;
    int pos = file.content[x].find(word, y);
    // Find the word on current line.
    Cursor ret_cursor = (pos == std::string::npos) ? Cursor(-1, -1) : Cursor(x, pos);
    // Find the word after current line.
    if(ret_cursor.x == -1 && ret_cursor.y == -1) ret_cursor = file.findNxt(word);
    // Find the word before current line.
    if(ret_cursor.x == -1 && ret_cursor.y == -1) ret_cursor = file.findPre(word);
    // There doesn't exist any word.
    if(ret_cursor.x == -1 && ret_cursor.y == -1) return;
    moveCursor(ret_cursor);
    cursor = file.file_cursor;
    refreshWithHighlight(word);
    for(char c = win.getchar();c != ESC;c = win.getchar()){
        if(c == UP) { // Move to the previous word, if exists.
            ret_cursor = file.findPre(word);
            if(ret_cursor.x == -1 && ret_cursor.y == -1) continue;
            moveCursor(ret_cursor);
            cursor = file.file_cursor;
            refreshWithHighlight(word);
        }else if(c == DOWN) { // Move to the next word, if exists.
            ret_cursor = file.findNxt(word);
            if(ret_cursor.x == -1 && ret_cursor.y == -1) continue;
            moveCursor(ret_cursor);
            cursor = file.file_cursor;
            refreshWithHighlight(word);
        }
    }
    refresh();
}

/* Find all the word in the file, and highlight them. 
   If the user presses enter, replace them. */
void Application::substituteReplace(const std::string &word, const std::string &subs){
    if(read_only) return;
    int x = file.file_cursor.x, y = file.file_cursor.y;
    int pos = file.content[x].find(word, y);
    // Find the word on current line.
    Cursor ret_cursor = (pos == std::string::npos) ? Cursor(-1, -1) : Cursor(x, pos);
    // Find the word after current line.
    if(ret_cursor.x == -1 && ret_cursor.y == -1) ret_cursor = file.findNxt(word);
    // Find the word before current line.
    if(ret_cursor.x == -1 && ret_cursor.y == -1) ret_cursor = file.findPre(word);
    // There doesn't exist any word.
    if(ret_cursor.x == -1 && ret_cursor.y == -1) return;
    moveCursor(ret_cursor);
    cursor = file.file_cursor;
    refreshWithHighlight(word);
    for(char c = win.getchar();c != ESC;c = win.getchar()){
        if(c == UP) { // Move to the previous word, if exists.
            ret_cursor = file.findPre(word);
            if(ret_cursor.x == -1 && ret_cursor.y == -1) continue;
            moveCursor(ret_cursor);
            cursor = file.file_cursor;
            refreshWithHighlight(word);
        }else if(c == DOWN) { // Move to the next word, if exists.
            ret_cursor = file.findNxt(word);
            if(ret_cursor.x == -1 && ret_cursor.y == -1) continue;
            moveCursor(ret_cursor);
            cursor = file.file_cursor;
            refreshWithHighlight(word);
        }else if(c == ENTER){ // Replace current word with subs.
            Cursor cursor_start = file.file_cursor;
            int x = file.file_cursor.x;
            delWordsOfString(file.content[x]);
            file.replace(word.size(), subs);
            addWordsOfString(file.content[x]);
            ret_cursor = file.findNxt(word);
            if(ret_cursor.x == -1 && ret_cursor.y == -1) ret_cursor = file.findPre(word);
            if(ret_cursor.x == -1 && ret_cursor.y == -1) {refresh(); return;}
            Cursor cursor_end = ret_cursor;
            moveCursor(cursor_start, cursor_end);
            file.file_cursor = cursor_end;
            cursor = file.file_cursor;
            refreshWithHighlight(word);
        }
    }
    refresh();
}

/* Substite command. */
void Application::substitute(const std::string &str){
    // Check if the command is valid.
    if(str[0] != '\"') return;
    int pos1 = str.find('\"', 1);
    if(pos1 == std::string::npos) return;
    std::string word = str.substr(1, pos1-1);
    if(word.size() == 0) return;
    if(word.size() + 2 == str.size()) {substituteQuery(word); return;} // Only Query
    if(str.find('\"', pos1+1) != pos1+2 || str[pos1+1] != ' ') return;
    int pos2 = str.find('\"', pos1+3);
    if(pos2 != str.size() - 1) return;
    std::string subs = str.substr(pos1+3, pos2-pos1-3); // Replacement
    substituteReplace(word, subs);
}

/* Command mode. */
void Application::commandMode(){
    command_history.push_back("");
    int x = command_history.size() - 1, y = 0;
    win.printCommand(command_history[x]);
    char c = win.getchar();
    for(;c != ENTER;c=win.getchar()) {
        if(c == ESC) {mode = NORMAL_MODE; break;}
        if(c == BACKSPACE) {
            if(command_history[x].size() && y != 0) {
                command_history[x] = command_history[x].substr(0, y-1) + command_history[x].substr(y);
                y--;
            }
            win.printCommand(command_history[x]); win.moveCursor(win.command_line, y+1); continue;
        }
        if(c == UP){
            if(x) x--, y = command_history[x].size();
            win.printCommand(command_history[x]); continue;
        }
        if(c == DOWN){
            if(x + 1 < command_history.size()) x++, y = command_history[x].size();
            win.printCommand(command_history[x]); continue;
        }
        if(c == RIGHT){
            if(y != command_history[x].size()) y++;
            win.printCommand(command_history[x]); win.moveCursor(win.command_line, y+1); continue;
        }
        if(c == LEFT){
            if(y != 0) y--;
            win.printCommand(command_history[x]); win.moveCursor(win.command_line, y+1);continue;
        }
        if(32 > c || c > 126) continue;
        if(command_history[x].size() < command_cols) {
            command_history[x] = command_history[x].substr(0, y) + c + command_history[x].substr(y);
            y++;
        }
        win.printCommand(command_history[x]);
        win.moveCursor(win.command_line, y+1);
    }
    if(c == ESC) {command_history.pop_back(); win.clearLine(win.command_line); return;}
    if(x != command_history.size() - 1) command_history.back() = command_history[x]; 
    if(command_history.size() > 50) command_history.pop_front();
    if(command_history[x] == "w") file.save();
    else if(command_history[x] == "q") {if(checkSave()) {quit = true;}}
    else if(command_history[x] == "q!") {quit = true;}
    else if(command_history[x] == "wq") {file.save(); quit = true;}
    else if(command_history[x].substr(0, 4) == "jmp ") {jmpLine(command_history[x].substr(4));}
    else if(command_history[x].substr(0, 4) == "sub ") {substitute(command_history[x].substr(4));}
    win.clearLine(win.command_line);
    mode = NORMAL_MODE;
    // win.printCommand("");
}

void Application::run(){
    while(!quit){
        refresh();
        if(mode == INSERT_MODE) insertMode();
        else if(mode == NORMAL_MODE) normalMode();
        else if(mode == COMMAND_MODE) commandMode();
    }
}