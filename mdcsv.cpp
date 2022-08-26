#include<iostream>
#include<cstdio>
#include<fstream>
#include<sstream>
#include<cstring>
#include<ctime>
//color of terminal output
#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define WHITE 37
#define CLEAR 0
using namespace std;
ifstream fin;
ofstream fout;
stringstream ss;

namespace displayMessage{
    void help(){
        printf("Markdown/CSV converter\n\
    This program can convert Markdown and CSV spreadsheets\n\
    Usage: mdcsv command file [option]\n\
    commands:\n\
    csv2md: convert CSV spreadsheet into Markdown format\n\
    md2csv: convert Markdown spreadsheet into CSV format\n\
    help: display this help message\n\
    options:\n\
    -f: print the result in a file instead of the console\n");
    }
    inline int color(int col,bool effectOnBackground=0){
        /*
Color	Foreground Code    Background Code
Black   30	40
Red     31	41
Green   32	42
Yellow  33	43
Blue    34	44
Magenta 35	45
Cyan    36	46
White   37	47

Option      	Code	Description
Reset           0	Back to normal (remove all styles)
Bold            1	Bold the text
Underline       4	Underline text
Inverse         7	Interchange colors of background and foreground
Strikethrough   9   Strikethrough the text
Bold off        21	Normal from bold
Underline off   24	Normal from Underline
Inverse off     27	Reverse of the Inverse

https://www.tutorialspoint.com/how-to-output-colored-text-to-a-linux-terminal
        */
       if(!effectOnBackground) printf("\033[%dm",col);
       else printf("\033[%dm",col+10);
       return 0;
    }
    void invalidSyntax(){
        color(RED);
        printf("Invalid syntax found in the command\n");
        color(CLEAR);
        exit(1);
    }
    void fileNotFound(string path){
        color(RED);
        printf("Error: ");
        color(9);
        printf("%s",path.data());
        color(CLEAR);
        color(RED);
        printf(" does not exist\n");
        color(CLEAR);
        exit(1);
    }
}

string delExt(string raw){//delete the filename extension
    string ret;
    int idxOfDot=-1;
    for(int i=0;i<raw.size();i++){//find the index of . in the string
        if(raw[i]=='.') idxOfDot=i;
    }
    if(idxOfDot==-1) return raw;//file without extension
    for(int i=0;i<idxOfDot;i++){//use substring before . only
        ss<<raw[i];
    }
    ss>>ret;
    ss.clear();
    return ret;
}
string queryExt(string raw){//query the filename extension, "." included(almost same as delExt())
    string ret;
    int idxOfDot=-1;
    for(int i=0;i<raw.size();i++){
        if(i==raw.size()-1&&raw[i]=='.') idxOfDot=i;//avoid invalid memory reference of raw[raw.size()]
        if(raw[i]=='.'&&((raw[i+1]>='a' && raw[i+1]<='z')||(raw[i+1]>='A' && raw[i+1]<='Z')||(raw[i+1]>='0'&&raw[i+1]<='9'))) idxOfDot=i;
        //avoid relative path to be identified as a extension
    }
    if(idxOfDot==-1) return "";
    for(int i=idxOfDot;i<raw.size();i++){
        ss<<raw[i];
    }
    ss>>ret;
    ss.clear();
    return ret;
}
void saperatePathFile(string raw,string &path,string &file){//saperate a directory into a path and a filename,similar with the delExt()
    int pos=-1;
    for(int i=0;i<raw.size();i++){
        if(raw[i]=='/') pos=i;
    }
    if(pos==-1){
        path="";
        file=raw;
        return;
    }
    for(int i=0;i<=pos;i++){
        ss<<raw[i];
    }
    ss>>path;
    ss.clear();
    for(int i=pos+1;i<raw.size();i++){
        ss<<raw[i];
    }
    ss>>file;
    ss.clear();
    return;
}
namespace csv2md{//convert CSV to MD
    void divide(string raw,char divCh){//divide string raw with character divCh(here comma)
        string cur;
        int length=0,line=0,column=0;
        cout<<'|';
        for(int i=0;i<raw.size();i++){
            if(raw[i]==divCh){
                cout<<'|';
                continue;
            }
            cout<<raw[i];
        }
        cout<<"|\n";
    }
    void label(){//add a label at the beginning of the file
    //https://blog.csdn.net/sweettool/article/details/76167654
        time_t tm;
        struct tm *tminfo;
        time(&tm);
        tminfo=localtime(&tm);
        printf("<!-- converted at %02d-%02d-%04d %02d:%02d.%02d -->\n",tminfo->tm_mon+1,tminfo->tm_mday,tminfo->tm_year+1900,tminfo->tm_hour,tminfo->tm_min,tminfo->tm_sec);
    }
    void heading(int level,string text){//add a heading to the MD file
        while(level--) cout<<'#';
        cout<<' ';
        cout<<text<<endl;
    }
    void defineFormat(string raw,char divCh){//output the format line of a MD spreadsheet(|---|---|---|)
        int colcnt=0;
        for(int i=0;i<raw.size();i++){//count the columns in the spreadsheet
            if(raw[i]==divCh){
                colcnt++;
            }
        }
        cout<<'|';
        for(int i=0;i<=colcnt;i++){
            cout<<"---|";
        }
        cout<<endl;
    }
    void read(string targetFileName,string targetFilePath){
        string line;
        bool isFirstLine=1,isSecondLine=0;
        while(getline(fin,line)){//read a line at a time
            if(isFirstLine){
                isFirstLine=0;
                isSecondLine=1;
            }
            else if(isSecondLine){//formatting need to be done at the second line of a table
                defineFormat(line,',');
                isSecondLine=0;
            }
            divide(line,',');
        }
    }

    void conv(string targetFileName,string TargetFilePath,bool enableOutputToFile){
        string targetFileNameWithoutExt;
        targetFileNameWithoutExt=delExt(targetFileName);
        fin.open(targetFileName.data(),ios::in);
        if(!fin.is_open()){
            displayMessage::fileNotFound(TargetFilePath+targetFileName);
        }
        if(enableOutputToFile){//writing result to file is required
            freopen((targetFileNameWithoutExt+".md").data(),"w",stdout);
        }
        label();
        heading(1,targetFileNameWithoutExt);
        read(targetFileName,TargetFilePath);
        fin.close();
        freopen("/dev/tty","w",stdout);
        if(enableOutputToFile){
            printf("Completed converting %s.csv to %s.md\n",targetFileNameWithoutExt.data(),targetFileNameWithoutExt.data());
        }
    }
}

namespace md2csv{//mostly same as above
    int numOfCol=0;
    void divide(string raw,char divCh,bool updateCnt=0){
        string cur;
        int length=0,line=0,column=0,curCol=0;
        for(int i=0;i<raw.size();i++){
            if(raw[i]=='|'&&i==0) continue;
            if(raw[i]==divCh){
                if(updateCnt){
                    curCol++;
                    numOfCol++;
                }
                else curCol++;
                cout<<',';
                continue;
            }
            cout<<raw[i];
        }
        for(;curCol<numOfCol;curCol++){//complete the comma,as CSV requires same amount of comma on every line
            cout<<',';
        }
        cout<<"\n";
    }
    bool pre_judge(string raw){//judge that is the current line a part of the table
        for(int i=0;i<raw.size();i++){
            if(raw[i]=='|'){
                return 1;
            }
        }
        return 0;
    }
    void read(string targetFileName,string targetFilePath){
        string line;
        bool isFirstLine=1,isSecondLine=0;
        while(getline(fin,line)){
            if(!pre_judge(line)) continue;//ignore every line that is not a part of a table
            if(isFirstLine){
                isFirstLine=0;
                isSecondLine=1;
                divide(line,'|',1);
                continue;
            }
            else if(isSecondLine){//the second line of a table is the format line, which need to be ignored
                isSecondLine=0;
                continue;
            }
            divide(line,'|');
        }
    }

    void conv(string targetFileName,string TargetFilePath,bool enableOutputToFile){
        string targetFileNameWithoutExt;
        targetFileNameWithoutExt=delExt(targetFileName);
        fin.open(targetFileName.data(),ios::in);
        if(!fin.is_open()){
            displayMessage::fileNotFound(TargetFilePath+targetFileName);
        }
        if(enableOutputToFile){
            freopen((targetFileNameWithoutExt+".csv").data(),"w",stdout);
        }
        read(targetFileName,TargetFilePath);
        fin.close();
        freopen("/dev/tty","w",stdout);
        if(enableOutputToFile){
            printf("Completed converting %s.md to %s.csv\n",targetFileNameWithoutExt.data(),targetFileNameWithoutExt.data());
        }
    }
}

struct Command{
    bool mode;
    string path;
    bool otf;//output to file
    void execute(){
        string pathWithoutFileName,fileName;
        saperatePathFile(path,pathWithoutFileName,fileName);
        if(!mode){//md2csv
            md2csv::conv(fileName,pathWithoutFileName,otf);
        }else{//csv2md
            csv2md::conv(fileName,pathWithoutFileName,otf);
        }
    }
};
namespace StepByStepInteract{
    string queryCommand(string output){
        printf("%s",output.data());
        char buf[32767];
        scanf("%s",buf);
        for(int i=0;i<strlen(buf);i++){
            ss<<buf[i];
        }
        string ret;
        ss>>ret;
        ss.clear();
        return ret;
    }
    void sbsi(){
        Command command;
        queryMode:;
        string mode=queryCommand("Input working mode(md2csv or csv2md)\n");
        if(mode=="md2csv") command.mode=0;
        else if(mode=="csv2md") command.mode=1;
        else{
            displayMessage::color(RED);
            printf("Invalid input\n");
            displayMessage::color(CLEAR);
            goto queryMode;
        }
        queryPath:;
        string path=queryCommand("Input the path of required file\n");
        fin.open(path.data());
        if(!fin.is_open()){
            displayMessage::color(RED);
            printf("Please enter a correct path\n");
            displayMessage::color(CLEAR);
            goto queryPath;
        }else fin.close();
        string fileExt=queryExt(path);
        if((command.mode&&!(fileExt=="csv"||fileExt=="CSV"))){
            checkExtCsv:;
            string con=queryCommand("Inputed file name does not have the correct extension(\""+fileExt+"\" detected, \".csv\" expected). Continue?[Y/n]\n");
            if(con.size()>1) goto checkExtCsv;
            else if(con[0]=='Y'||con[0]=='y') goto savePath;
            else if(con[0]=='N'||con[0]=='n') goto queryPath;
            else goto checkExtCsv;
        }
        if((!command.mode&&!(fileExt=="md"||fileExt=="MD"))){
            checkExtMd:;
            string con=queryCommand("Inputed file name does not have the correct extension(\""+fileExt+"\" detected, \".md\" expected). Continue?[Y/n]\n");
            if(con.size()>1) goto checkExtMd;
            else if(con[0]=='Y'||con[0]=='y') goto savePath;
            else if(con[0]=='N'||con[0]=='n') goto queryPath;
            else goto checkExtMd;
        }
        savePath:;
        command.path=path;
        queryOTF:;
        string otf=queryCommand("Do you want to output to terminal or file(terminal or file)\n");
        if(otf=="terminal") command.otf=0;
        else if(otf=="file") command.otf=1;
        else{
            displayMessage::color(RED);
            printf("Invalid input\n");
            displayMessage::color(CLEAR);
            goto queryOTF;
        }
        command.execute();
    }
}

int main(int argv,char **argc){
    string argcstr[argv];
    for(int i=0;i<argv;i++){//convert the arguments from char* to string
        argcstr[i]=argc[i];
    }
    if(argv<=1){//no arguments included
        StepByStepInteract::sbsi();
    }
    else if(argcstr[1]=="help"){
        displayMessage::help();
    }
    else if(argcstr[1]=="md2csv"){
        if(argv<=2){//require at least 2 arguments
            displayMessage::invalidSyntax();
        }
        bool fileOut=0;
        if(argv==4){
            if(argcstr[3]=="-f"||argcstr[3]=="-F"){//output to file
                fileOut=1;
            }
            else{
                displayMessage::invalidSyntax();
            }
        }
        string raw=argc[2],path,file;
        saperatePathFile(raw,path,file);
        md2csv::conv(file,path,fileOut);
    }else if(argcstr[1]=="csv2md"){//same as above
        if(argv<=2){
            displayMessage::invalidSyntax();
        }
        bool fileOut=0;
        if(argv==4){
            if(argcstr[3]=="-f"||argcstr[3]=="-F"){
                fileOut=1;
            }
            else{
                displayMessage::invalidSyntax();
            }
        }
        string raw=argc[2],path,file;
        saperatePathFile(raw,path,file);
        csv2md::conv(file,path,fileOut);
    }
    else{
        displayMessage::invalidSyntax();
    }
    return 0;
}