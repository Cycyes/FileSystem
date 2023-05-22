#ifndef FILE_H
#define FILE_H

#include "Inode.h"

using namespace std;

/*
* File��
* �ýṹ��¼���ļ��Ķ���д�������ͣ��ļ���дλ��
*/
class File {
	/*=== Static Consts ===*/
public:
	static const int f_count_init = 0;
	static const int f_flag_init = 0;
	static const int f_offset_init = 0;

	/*=== Enums ===*/
public:
	enum FileFlags {
		FREAD = 0x1, // ������
		FWRITE = 0x2 //д����
	};

	/*=== Functions ===*/
public:
	File();
	~File();

	/*=== Members ===*/
public:
	unsigned int f_flag; // �Դ��ļ��Ķ���д����Ҫ��
	int f_count; // ��ǰ���ø��ļ����ƿ�Ľ�����
	Inode* f_inode; // ָ����ļ����ڴ�Inodeָ��
	int f_offset; // �ļ���дλ��ָ��
};



/*
* OpenFiles��
* ���̴��ļ���������
* ���̵�u�ṹ�а�����OpenFiles���һ������ά����ǰ���̵����д򿪵��ļ�
*/
class OpenFiles {
	/*=== Static Consts ===*/
public:
	static const int OPEN_FILES_NUM = 100; // �������������ļ���

	/*=== Functions ===*/
public:
	OpenFiles();
	~OpenFiles();

	/* �ڴ��ļ����������з���һ�����б��� */
	int AllocFreeSlot();
	/* ����fd�ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ */
	File* GetF(const int& fd);
	/* Ϊ�ѷ��䵽����������fd���ѷ���Ĵ��ļ����п���File������������ϵ */
	void SetF(const int& fd, File* fp);

	/*=== Members ===*/
private:
	File* ProcessOpenFileTable[OPEN_FILES_NUM];
};



class IOParameter {
	/*=== Functions ===*/
public:
	IOParameter();
	~IOParameter();

	/*=== Members ===*/
public:
	unsigned char* m_Base; // ��ǰ����д�û�Ŀ��������׵�ַ
	int m_Offset; // ��ǰ����д�ļ����ֽ�ƫ����
	int m_Count; // ��ǰ��ʣ��Ķ���д�ֽ�����
};

#endif