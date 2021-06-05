#ifndef FILESYSTEM_FILESYSTEM_H
#define FILESYSTEM_FILESYSTEM_H
#include "common.h"

class FileSystem {
public:
    FileSystem();
    void init_file();
    void run();
    bool login(std::string& user, std::string& pwd);
    int create(char* filename);
    int open(char* fileName);
    int close(char* fileName);
    int write(char* buff, size_file len);
    int read(char* buff);
    int del(char* fileName);
    int mkdir(char* fileName);
    int rmdir(char* fileName);
    int cd(char* name);
    void dir();
    void format();
    void showCurrentDir();
    void print();
    void enter();
    ~FileSystem();

private:
    std::unordered_map<std::string , std::string> userList;    /*账户*/
    std::string user;             /*当前用户*/
    std::string codes[15];        /*命令集*/
    char* currentDir;             /*当前目录名*/
    FAT* fat;                     /*FAT表起始地址*/
    Direct* root;                 /*根目录起始地址*/
    Direct* cur_dir;              /*当前目录*/
    OpenTable u_opentable{};      /*文件打开表*/
    int fd;                       /*文件描述符*/
    char* fdisk;                  /*内存中的磁盘起始地址*/
};


#endif //FILESYSTEM_FILESYSTEM_H
