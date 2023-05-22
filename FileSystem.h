#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "BufferManager.h"
#include "Inode.h"

const int s_free_size = 100;
const int s_inode_size = 100;
const int s_paddings_size = 52;

/*
* DirectoryEntry��
* Ŀ¼��
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
* SuperBlock��
* �ļ�ϵͳ�洢��Դ����
*/
class SuperBlock {
	/*=== Functions ===*/
public:
	SuperBlock();
	~SuperBlock();

	/*=== Members ===*/
public:
	int s_isize; // ���Inode��ռ�õ��̿���
	int s_fsize; // �̿�����
	int s_nfree; // ֱ�ӹ���Ŀ����̿���
	int s_free[s_free_size]; // ֱ�ӹ���Ŀ����̿�������
	int s_ninode; // ֱ�ӹ�������Inode��
	int s_inode[s_inode_size]; // ֱ�ӹ���Ŀ������Inode������
	int s_paddings[s_paddings_size]; // �����1024�ֽڣ�ռ�������̿�
};



/*
* FileSystem��
* �����ļ��洢�豸�и���洢��Դ�����̿顢���Inode�ķ��䣬�ͷ�
*/
class FileSystem {
	/*=== Static Consts ===*/
public:
	static const int superblock_offset = 0; // superblock��ʼλ��
	static const int superblock_block_num = 2; // superblock��ռ�̿���

	static const int inode_offset = 2; // indoe����ʼλ��
	static const int inode_num_per_block = 8; // ÿ���̿��Inode����
	static const int inode_block_num = 1022; // inode��ռ�̿���
	static const int inode_num = inode_num_per_block * inode_block_num; // inode�ܽڵ���

	static const int disk_block_num = 16384; // ������ռ�̿���
	static const int block_size = 512; // �̿��С512�ֽ�

	static const int data_offset = 1024; // �����ļ�����ʼλ��
	static const int data_block_num = disk_block_num - inode_block_num - superblock_block_num; // �����ļ�����ռ�̿���

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

	/* ����һ���������Inode������Ӧ�����ڴ�Inode */
	Inode* IAlloc();

	/* �ͷ�inumber���Inode */
	void IFree(const int& inumber);

	/* ������д��̿飬�������Ӧ�Ļ��� */
	Buf* Alloc();

	/* �ͷ�blkno���� */
	void Free(const int& blkno);


	/*=== Members ===*/
private:
	SuperBlock* m_SuperBlock;
	BufferManager* m_BufferManager;
	DeviceManager* m_DeviceManager;
};

#endif