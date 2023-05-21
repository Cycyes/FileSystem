#include "BufferManager.h"

#include <iostream>
#include <cstring>

using namespace std;

/*=== ����ȫ�ֱ��� ===*/
extern DeviceManager globalDeviceManager;

/*========== private ===========*/
void BufferManager::init() {

	for (int i = 0; i < buf_num; i++) {
		/* ����b_prev */
		if (i) {
			m_Buf[i].b_prev = m_Buf + i - 1;
		}
		else {
			/* ��ʼ�����ɶ��н��ڻ�������ͷ��ǰ�� */
			m_Buf[i].b_prev = bFreeList; 
			bFreeList->b_next = m_Buf;
		}

		/* ����b_next */
		if (i + 1 < buf_num) {
			m_Buf[i].b_next = m_Buf + i + 1;
		}
		else {
			/* ��ʼ�����ɶ��н��ڻ�������β�ĺ��� */
			m_Buf[i].b_next = bFreeList;
			bFreeList->b_prev = m_Buf + buf_num - 1;
		}
	}
}

/* �ڶ�����ȡ��bp */
void BufferManager::Bdetach(Buf* bp) {

	if (bp->b_next != NULL) {
		/* ��ǰ���Ԫ��������Ԫ������ */
		bp->b_prev->b_next = bp->b_next; 
		bp->b_next->b_prev = bp->b_prev;
		bp->b_prev = NULL;
		bp->b_next = NULL;
	}
}

/* �������ɶ��еĶ�β */
void BufferManager::Binsert(Buf* bp) {

	if (bp->b_next == NULL) {
		/* �������ɶ��еĶ�β */
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
		cerr << "����bFreeListʧ��" << endl;
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

	/* ���ҵ���ֱ�ӷ��� */
	if (this->m_map.find(blkno) != this->m_map.end()) {
		bp = this->m_map[blkno];
		this->Bdetach(bp); // bp������
		return bp;
	}

	/* �������ɶ��� */
	bp = this->bFreeList->b_next; // ȡ���ɶ��е���Ԫ
	if (bp == bFreeList) {
		cerr << "bFreeList���в���" << endl;
		return NULL;
	}

	this->Bdetach(bp); // bp������

	/* ����map */
	this->m_map.erase(bp->b_blkno); // ɾ����Ԫ��
	this->m_map[blkno] = bp;
	
	/* �ӳ�д */
	if (bp->checkDELWRI()) {
		m_DeviceManager->write(bp->b_addr, Buf::buffer_size, bp->b_blkno * Buf::buffer_size);
	}

	/* ����bp */
	bp->b_blkno = blkno;
	bp->clearDELWRI();
	bp->clearDONE();

	return bp;
}

/* ��bp������� */
void BufferManager::Brelse(Buf* bp) {

	if (bp->b_next == NULL) {
		/* �������ɶ��еĶ�β����prev�� */
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
	this->Brelse(bp); // �������ɶ���
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