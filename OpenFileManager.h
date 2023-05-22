#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include "File.h"

/*
* OpenFileTable系统打开文件表
* 管理内核系统对打开文件机构的管理，为进程打开文件建立内核数据结构之间的勾连关系
*/
class OpenFileTable {
	/*=== Static Consts ===*/
public:
	static const int FILE_SIZE = 100;

	/*=== Functions ===*/
public:
	OpenFileTable();
	~OpenFileTable();

	/* 在系统代开文件表中分配一个空闲的File结构 */
	File* FAlloc();

	/* 对打开文件控制块File结构的引用计数f_count减1，减为0时释放File */
	void CloseF(File* fp);

	void Format();

	/*=== Members ===*/
public:
	File m_File[FILE_SIZE]; // 系统打开文件表
};



/*
* 内存Inode表
*/
class InodeTable {
	/*=== Static Consts ===*/
public:
	static const int INODE_SIZE = 100;

	/*=== Functions ===*/
public:
	InodeTable();
	~InodeTable();

	/* 检查inumber的外存Inode是否有相应的内存Inode，若有返回索引 */
	int IsLoaded(const int& inumber);

	/* 在内存Inode表中寻找一i个空闲的内存Inode */
	Inode* GetFreeInode();

	/* 根据inumber从外存Inode中获取相应的Inode并读入内存 */
	Inode* IGet(const int& inumber);

	/* 减少该内存Inode的引用次数，如果此Inode无目录项指向它且五金城引用，则释放文件占用的磁盘块 */
	void IPut(Inode* ip);

	/* 将所有修改过的内存Inode更新到对应的外存Inode中 */
	void UpdateInodeTable();

	
	void Format();

	/*=== Members ===*/
public:
	Inode m_Inode[INODE_SIZE]; // 内存Inode数组
	FileSystem* m_FileSystem; // 对应全局globalFileSystem
};

#endif