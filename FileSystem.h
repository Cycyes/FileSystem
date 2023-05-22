#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "BufferManager.h"
#include "Inode.h"

const int s_free_size = 100;
const int s_inode_size = 100;
const int s_paddings_size = 52;

/*
* DirectoryEntry类
* 目录项
*/
class DirectoryEntry {
	/*=== Static Consts ===*/
public:
	static const int dir_name_size = 28;

	/*=== Members ===*/
public:
	int m_ino;
	char name[dir_name_size];
};


/*
* SuperBlock类
* 文件系统存储资源管理
*/
class SuperBlock {
	/*=== Functions ===*/
public:
	SuperBlock();
	~SuperBlock();

	/*=== Members ===*/
public:
	int s_isize; // 外存Inode区占用的盘块数
	int s_fsize; // 盘块总数
	int s_nfree; // 直接管理的空闲盘块数
	int s_free[s_free_size]; // 直接管理的空下盘块索引表
	int s_ninode; // 直接管理的外存Inode数
	int s_inode[s_inode_size]; // 直接管理的空闲外存Inode索引表
	int s_paddings[s_paddings_size]; // 填充至1024字节，占用两个盘块
};



/*
* FileSystem类
* 管理文件存储设备中各类存储资源、磁盘块、外存Inode的分配，释放
*/
class FileSystem {
	/*=== Static Consts ===*/
public:
	static const int superblock_offset = 0; // superblock开始位置
	static const int superblock_block_num = 2; // superblock所占盘块数

	static const int inode_offset = 2; // indoe区开始位置
	static const int inode_num_per_block = 8; // 每个盘块的Inode个数
	static const int inode_block_num = 1022; // inode所占盘块数
	static const int inode_num = inode_num_per_block * inode_block_num; // inode总节点数

	static const int disk_block_num = 16384; // 磁盘所占盘块数
	static const int block_size = 512; // 盘块大小512字节

	static const int data_offset = 1024; // 数据文件区开始位置
	static const int data_block_num = disk_block_num - inode_block_num - superblock_block_num; // 数据文件区所占盘快数

	static const int nfree_init = 0;
	static const int ninode_init = 0;

	/*=== Functions ===*/
private:
	void FormatSuperBlock();
	void LoadSuperBlock();
	void Update();

public:
	FileSystem();
	~FileSystem();
	
	void Format();

	/* 分配一个空闲外存Inode，并对应分配内存Inode */
	Inode* IAlloc();

	/* 释放inumber外存Inode */
	void IFree(const int& inumber);

	/* 分配空闲磁盘块，并申请对应的缓存 */
	Buf* Alloc();

	/* 释放blkno磁盘 */
	void Free(const int& blkno);


	/*=== Members ===*/
private:
	SuperBlock* m_SuperBlock;
	BufferManager* m_BufferManager;
	DeviceManager* m_DeviceManager;
};

#endif