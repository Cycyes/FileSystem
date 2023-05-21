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

	Buf* GetBlk(const int& blkno); // ����һ�黺�棬���ڶ�д�豸dev�ϵ��ַ���blkno
	void Brelse(Buf* bp);
	Buf* Bread(const int& blkno); // ��һ�����̿飬blknoΪĿ����̿��߼����
	void Bwrite(Buf* bp); // дһ�����̿�
	void Bdwrite(Buf* bp); // �ӳ�д���̿�
	void Bclear(Buf* bp); // ��ջ���������
	void Bflush(); //���������ӳ�д�Ļ���ȫ�����������
	void Bformat();
	
	/*=== Members ===*/
private:
	Buf* bFreeList; // ���ɻ�����п��ƿ�
	Buf m_Buf[buf_num]; // �������
	unordered_map<int, Buf*> m_map; // ����blkno���ٲ���Buf
	DeviceManager* m_DeviceManager; // ָ���豸����ģ��ȫ�ֶ���
};

#endif

