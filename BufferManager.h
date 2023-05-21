#ifndef BUFFMANAGER_H
#define BUFFMANAGER_H

#include "Buf.h"
#include "DeviceManager.h"

#include <unordered_map>

using namespace std;

class BufferManager {
	/*=== Static Consts ===*/
public:
	static const int buf_num = 100;

	/*=== Functions ===*/
private:
	void init();

	void Bdetach(Buf* bp);
	void Binsert(Buf* bp);


public:
	BufferManager();
	~BufferManager();

	Buf* GetBlk(const int& blkno); // 申请一块缓存，用于读写设备dev上的字符块blkno
	void Brelse(Buf* bp);
	Buf* Bread(const int& blkno); // 读一个磁盘块，blkno为目标磁盘块逻辑块号
	void Bwrite(Buf* bp); // 写一个磁盘块
	void Bdwrite(Buf* bp); // 延迟写磁盘块
	void Bclear(Buf* bp); // 清空缓冲区内容
	void Bflush(); //将队列中延迟写的缓存全部输出到磁盘
	void Bformat();
	
	/*=== Members ===*/
private:
	Buf* bFreeList; // 自由缓存队列控制块
	Buf m_Buf[buf_num]; // 缓存队列
	unordered_map<int, Buf*> m_map; // 根据blkno快速查找Buf
	DeviceManager* m_DeviceManager; // 指向设备管理模块全局对象
};

#endif

