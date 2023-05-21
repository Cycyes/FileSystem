#include "BufferManager.h"

#include <iostream>
#include <cstring>

using namespace std;

/*=== 引用全局变量 ===*/
extern DeviceManager globalDeviceManager;

/*========== private ===========*/
void BufferManager::init() {

	for (int i = 0; i < buf_num; i++) {
		/* 更新b_prev */
		if (i) {
			m_Buf[i].b_prev = m_Buf + i - 1;
		}
		else {
			/* 初始化自由队列接在缓存数组头的前面 */
			m_Buf[i].b_prev = bFreeList; 
			bFreeList->b_next = m_Buf;
		}

		/* 更新b_next */
		if (i + 1 < buf_num) {
			m_Buf[i].b_next = m_Buf + i + 1;
		}
		else {
			/* 初始化自由队列接在缓存数组尾的后面 */
			m_Buf[i].b_next = bFreeList;
			bFreeList->b_prev = m_Buf + buf_num - 1;
		}
	}
}

/* 在队列中取出bp */
void BufferManager::Bdetach(Buf* bp) {

	if (bp->b_next != NULL) {
		/* 将前面的元素与后面的元素连接 */
		bp->b_prev->b_next = bp->b_next; 
		bp->b_next->b_prev = bp->b_prev;
		bp->b_prev = NULL;
		bp->b_next = NULL;
	}
}

/* 插入自由队列的对尾 */
void BufferManager::Binsert(Buf* bp) {

	if (bp->b_next == NULL) {
		/* 插入自由队列的对尾 */
		bp->b_prev = bFreeList->b_prev;
		bp->b_next = bFreeList;
		bFreeList->b_prev->b_next = bp;
		bFreeList->b_prev = bp;
	}
}
/*========== private ===========*/



/*========== public ==========*/
BufferManager::BufferManager() {
	this->bFreeList = new Buf;
	if (!this->bFreeList) {
		cerr << "申请bFreeList失败" << endl;
		exit(ERROR_NEW);
	}
	this->init();
	m_DeviceManager = &globalDeviceManager;
}

BufferManager::~BufferManager() {
	this->Bflush();
	if (this->bFreeList) {
		delete bFreeList;
	}
}

Buf* BufferManager::GetBlk(const int& blkno) {
	Buf* bp;

	/* 若找到，直接分配 */
	if (this->m_map.find(blkno) != this->m_map.end()) {
		bp = this->m_map[blkno];
		this->Bdetach(bp); // bp出队列
		return bp;
	}

	/* 分配自由队列 */
	bp = this->bFreeList->b_next; // 取自由队列的首元
	if (bp == bFreeList) {
		cerr << "bFreeList队列不足" << endl;
		return NULL;
	}

	this->Bdetach(bp); // bp出队列

	/* 更新map */
	this->m_map.erase(bp->b_blkno); // 删除旧元素
	this->m_map[blkno] = bp;
	
	/* 延迟写 */
	if (bp->checkDELWRI()) {
		m_DeviceManager->write(bp->b_addr, Buf::buffer_size, bp->b_blkno * Buf::buffer_size);
	}

	/* 更新bp */
	bp->b_blkno = blkno;
	bp->clearDELWRI();
	bp->clearDONE();

	return bp;
}

/* 将bp插入队列 */
void BufferManager::Brelse(Buf* bp) {

	if (bp->b_next == NULL) {
		/* 插入自由队列的对尾（即prev） */
		bp->b_prev = bFreeList->b_prev;
		bp->b_next = bFreeList;
		bFreeList->b_prev->b_next = bp;
		bFreeList->b_prev = bp;
	}
}

Buf* BufferManager::Bread(const int& blkno) {

	Buf* bp = this->GetBlk(blkno);

	if (bp->checkDELWRI() || bp->checkDONE()) {
		return bp;
	}

	m_DeviceManager->read(bp->b_addr, Buf::buffer_size, bp->b_blkno * Buf::buffer_size);

	bp->setDONE();

	return bp;
}

void BufferManager::Bwrite(Buf* bp) {
	bp->clearDELWRI();
	m_DeviceManager->write(bp->b_addr, Buf::buffer_size, bp->b_blkno * Buf::buffer_size);
	bp->setDONE();
	this->Brelse(bp); // 放入自由队列
}

void BufferManager::Bdwrite(Buf* bp) {
	bp->setDELWRI();
	bp->setDONE();
	this->Brelse(bp);
}

void BufferManager::Bformat() {
	Buf b;
	for (int i = 0; i < buf_num; i++) {
		memcpy(m_Buf + i, &b, sizeof(Buf));
	}
	this->init();
}

void BufferManager::Bclear(Buf* bp) {
	memset(bp->b_addr, 0, Buf::buffer_size);
}

void BufferManager::Bflush() {
	Buf* bp = NULL;
	for (int i = 0; i < buf_num; i++) {
		bp = &m_Buf[i];
		if (bp->checkDELWRI()) {
			m_DeviceManager->write(bp->b_addr, Buf::buffer_size, bp->b_blkno * Buf::buffer_size);
			bp->clearDELWRI();
			bp->setDONE();
		}
	}
}
/*========== public ==========*/