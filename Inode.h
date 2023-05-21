#ifndef INODE_H
#define INODE_H

#include "Buf.h"

using namespace std;

/*
* DiskInode类
* 磁盘Inode，位于文件存储设备外存Inode区中
* 与每个文件唯一对应，记录该文件的控制信息
* DiskInode长度为64字节，每个盘块可以存放8个DiskInode节点
*/
class DiskInode {
	/*=== Static Consts ===*/
public:
	static const int d_padding_size = 3;
	static const int d_mode_init = 0;
	static const int d_nlink_init = 0;

	/*=== Functions ===*/
public:
	DiskInode();
	~DiskInode();

	/*=== Members ===*/
public:
	unsigned int d_mode; // 状态标志位
	int d_nlink; // 文件联接数
	int d_size; // 文件大小，单位字节
	int d_addr[10]; // 文件逻辑块号和物理块号转化索引
	int d_paddings[d_padding_size]; // 填充字节
};



/*
* Inode类
* 内存索引Inode节点
* 系统中每一个打开的文件、当前访问目录、挂载的子文件系统都对应唯一的内存Inode
*/
class Inode {
	/*=== Enums ===*/
public:
	enum InodeFlag {
		ILOCK = 0x1, // 锁
		IUPD = 0x2, // 内存inode节点被修改过，需要更新相应的外存inode
		IACC = 0x4, // 
		IMOUNT = 0x8, //
		IWANT = 0x10,
		ITEXT = 0x20
	};

	/*=== Static Consts ===*/
public:
	static const unsigned int IALLOC = 0x8000; // 文件被使用
	static const unsigned int IFMT = 0x6000; // 文件类型掩码
	static const unsigned int IFDIR = 0x4000; // 文件类型：目录文件
	static const unsigned int IDCHR = 0x2000; // 字符设备特殊类型文件
	static const unsigned int IFBLK = 0x6000; // 块设备特殊类型文件，为0表示常规数据文件
	static const unsigned int ILARG = 0x1000; // 文件长度类型：大型或巨型文件
	static const unsigned int IREAD = 0x100; // 读权限
	static const unsigned int IWRITE = 0x80; // 写权限


	static const int i_flag_init = 0;
	static const int i_mode_init = 0;
	static const int i_count_init = 0;
	static const int i_nlink_init = 0;
	static const int i_size_init = 0;
	static const int i_number_init = -1;

	static const int BLOCK_SIZE = 512; // 盘块大小
	static const int ADDRESS_PER_BLOCK = BLOCK_SIZE / sizeof(int); // 每个索引表包含的物理盘块号的个数

	static const int SMALL_FILE_INDEX = 6;
	static const int LARGE_FILE_INDEX = 8;
	static const int HUGE_FILE_INDEX = 10;

	static const int SMALL_FILE_BLOCK_NUM = 6; // 小型文件所占盘快数
	static const int LARGE_FILE_BLOCK_NUM = 128 * 2 + 6; // 大型文件所占盘块数
	static const int HUGE_FILE_BLOCK_NUM = 128 * 128 * 2 + 128 * 2 + 6; // 巨型文件所占盘块数

	/*=== Functions ===*/
private:
	bool checkUPD();
	void setUPD();
	void clearLARG();

public:
	Inode();
	~Inode();

	int Bmap(const int& lbn);
	void ReadI();
	void WriteI();
	void IUpdate(const int& time);
	void ITrunc();
	void ICopy(Buf* bp, const int& inumber);
	void Clean();

	/*=== Members ===*/
public:
	unsigned int i_flag; // 状态标志位
	unsigned int i_mode; // 文件工作方式
	int i_count;
	int i_nlink;
	int i_number; // 外村inode区节点的编号
	int i_size; // 文件大小，单位字节
	int i_addr[10]; // 文件逻辑块号和物理块号转换表
	
};

#endif