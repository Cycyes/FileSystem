#ifndef FILE_H
#define FILE_H

#include "Inode.h"

using namespace std;

/*
* File类
* 该结构记录打开文件的读、写请求类型，文件读写位置
*/
class File {
	/*=== Static Consts ===*/
public:
	static const int f_count_init = 0;
	static const int f_flag_init = 0;
	static const int f_offset_init = 0;

	/*=== Enums ===*/
public:
	enum FileFlags {
		FREAD = 0x1, // 读请求
		FWRITE = 0x2 //写请求
	};

	/*=== Functions ===*/
public:
	File();
	~File();

	/*=== Members ===*/
public:
	unsigned int f_flag; // 对打开文件的读、写操作要求
	int f_count; // 当前引用该文件控制块的进程数
	Inode* f_inode; // 指向打开文件的内存Inode指针
	int f_offset; // 文件读写位置指针
};



/*
* OpenFiles类
* 进程打开文件描述符表
* 进程的u结构中包含了OpenFiles类的一个对象，维护当前进程的所有打开的文件
*/
class OpenFiles {
	/*=== Static Consts ===*/
public:
	static const int OPEN_FILES_NUM = 100; // 进程允许的最大文件数

	/*=== Functions ===*/
public:
	OpenFiles();
	~OpenFiles();

	/* 在打开文件描述符表中分配一个空闲表项 */
	int AllocFreeSlot();
	/* 根据fd找到对应的打开文件控制块File结构 */
	File* GetF(const int& fd);
	/* 为已分配到空闲描述符fd和已分配的打开文件表中空闲File对象建立勾连关系 */
	void SetF(const int& fd, File* fp);

	/*=== Members ===*/
private:
	File* ProcessOpenFileTable[OPEN_FILES_NUM];
};



class IOParameter {
	/*=== Functions ===*/
public:
	IOParameter();
	~IOParameter();

	/*=== Members ===*/
public:
	unsigned char* m_Base; // 当前读、写用户目标区域的首地址
	int m_Offset; // 当前读、写文件的字节偏移量
	int m_Count; // 当前还剩余的读、写字节数量
};

#endif