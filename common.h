#ifndef FILESYSTEM_COMMON_H
#define FILESYSTEM_COMMON_H

#define BLOCK_SIZE 1024                         /*块大小*/
#define DIR_LENGTH 100                          /*最长文件（目录）名*/
#define MSD 16                                 /*每个目录的最大文件数量（包括目录）*/
#define FILE_OPEN_NUM 5                        /*可同时打开的文件数量*/
#define MEM 1024 * 1024                         /*1M磁盘空间*/
#define BLOCK_NUM MEM / BLOCK_SIZE              /*磁盘块的数目*/
#define FAT_SIZE BLOCK_NUM * sizeof(FAT)        /*FAT表的大小*/
#define ROOT_BLOCK_NO FAT_SIZE / BLOCK_SIZE + 1 /*根目录起始盘块号*/
#define ROOT_BLOCK_SIZE sizeof(Direct)          /*根目录大小*/
#define MAX_WRITE 1024 * 128                    /*最大允许写入的字节数*/
#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <string>
#include <list>
#include <cstdlib>
#include <fstream>

using size_file = unsigned int;

struct FAT {
    int item;           /*存放文件下一个磁盘块号*/
    bool is_free;       /*磁盘块是否空闲*/
};

struct Direct {
    struct FCB {
        char name[16];      /*文件（目录）名*/
        bool isDir;         /*是否为目录*/
        size_file size;     /*文件大小*/
        int first;           /*起始盘块号*/
        bool isRoot;        /*是否是根目录*/
    } direct_item[MSD + 2];
};

struct OpenTable {
    struct opentableitem {
        char name[16];          /*文件名*/
        int first;               /*起始盘块号*/
        size_file size;         /*文件大小*/
    } openitem[FILE_OPEN_NUM];
    int cur_size;               /*当前打开的文件大小*/
};

#endif //FILESYSTEM_COMMON_H
