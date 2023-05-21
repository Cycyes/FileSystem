#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "Common.h"

#include <cstdio>

/*
* DeviceManager类
* 负责读写磁盘文件
* 采用C方式（C++读写文件与C方式有些许区别）
*/
class DeviceManager {
    /*=== Static Consts===*/
public:
    static const char* DISK_FILE_NAME; // 磁盘镜像文件名

    /*=== Members ===*/
private:
    FILE* fp; // 磁盘文件指针

    /*=== Functions ===*/
public:
    DeviceManager(); // 构造函数
    ~DeviceManager(); // 析构函数

    bool IsExist(); // 检查镜像文件是否存在
    void Open(); // 打开镜像文件
    void write(const void* buffer, unsigned int size, int offset = -1, unsigned int origin = SEEK_SET); // 写磁盘
    void read(void* buffer, unsigned int size, int offset = -1, unsigned int origin = SEEK_SET); // 读磁盘
};

#endif