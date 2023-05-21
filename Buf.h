#ifndef BUF_H
#define BUF_H

using namespace std;

/*
* Buf类
* 记录缓存的使用情况
*/
class Buf {
	/*=== Static Consts ===*/
public:
	static const int b_flags_init = 0;
	static const int b_wcount_init = 0;
	static const int b_blkno_init = -1;
	static const int b_paddings_size = 2;
	static const int buffer_size = 512;

	/*=== Enums ===*/
public:
	enum BufFlag {
		B_DONE = 0x1,
		B_DELWRI = 0x2
	};

	/*=== Functions ===*/
public:
	Buf();
	Buf(const Buf& b);
	~Buf();

	bool checkDELWRI();
	bool checkDONE();
	void clearDELWRI();
	void clearDONE();
	void setDELWRI();
	void setDONE();

	/*=== Members ===*/
public:
	unsigned int b_flags; // 缓存控制块的标志位

	/* 双向链表 */
	Buf* b_prev;
	Buf* b_next;

	int b_wcount; // 需要传送的字节数
	unsigned char* b_addr; // 指向该缓存控制块所管理的缓冲区的首地址
	int b_blkno; // 磁盘逻辑块号

	int b_paddings[b_paddings_size]; // 填充字节至32
};

#endif
