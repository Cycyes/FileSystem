#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include "File.h"

/*
* OpenFileTableϵͳ���ļ���
* �����ں�ϵͳ�Դ��ļ������Ĺ���Ϊ���̴��ļ������ں����ݽṹ֮��Ĺ�����ϵ
*/
class OpenFileTable {
	/*=== Static Consts ===*/
public:
	static const int FILE_SIZE = 100;

	/*=== Functions ===*/
public:
	OpenFileTable();
	~OpenFileTable();

	/* ��ϵͳ�����ļ����з���һ�����е�File�ṹ */
	File* FAlloc();

	/* �Դ��ļ����ƿ�File�ṹ�����ü���f_count��1����Ϊ0ʱ�ͷ�File */
	void CloseF(File* fp);

	void Format();

	/*=== Members ===*/
public:
	File m_File[FILE_SIZE]; // ϵͳ���ļ���
};



/*
* �ڴ�Inode��
*/
class InodeTable {
	/*=== Static Consts ===*/
public:
	static const int INODE_SIZE = 100;

	/*=== Functions ===*/
public:
	InodeTable();
	~InodeTable();

	/* ���inumber�����Inode�Ƿ�����Ӧ���ڴ�Inode�����з������� */
	int IsLoaded(const int& inumber);

	/* ���ڴ�Inode����Ѱ��һi�����е��ڴ�Inode */
	Inode* GetFreeInode();

	/* ����inumber�����Inode�л�ȡ��Ӧ��Inode�������ڴ� */
	Inode* IGet(const int& inumber);

	/* ���ٸ��ڴ�Inode�����ô����������Inode��Ŀ¼��ָ�������������ã����ͷ��ļ�ռ�õĴ��̿� */
	void IPut(Inode* ip);

	/* �������޸Ĺ����ڴ�Inode���µ���Ӧ�����Inode�� */
	void UpdateInodeTable();

	
	void Format();

	/*=== Members ===*/
public:
	Inode m_Inode[INODE_SIZE]; // �ڴ�Inode����
	FileSystem* m_FileSystem; // ��Ӧȫ��globalFileSystem
};

#endif