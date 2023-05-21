#ifndef INODE_H
#define INODE_H

#include "Buf.h"

using namespace std;

/*
* DiskInode��
* ����Inode��λ���ļ��洢�豸���Inode����
* ��ÿ���ļ�Ψһ��Ӧ����¼���ļ��Ŀ�����Ϣ
* DiskInode����Ϊ64�ֽڣ�ÿ���̿���Դ��8��DiskInode�ڵ�
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
	unsigned int d_mode; // ״̬��־λ
	int d_nlink; // �ļ�������
	int d_size; // �ļ���С����λ�ֽ�
	int d_addr[10]; // �ļ��߼���ź�������ת������
	int d_paddings[d_padding_size]; // ����ֽ�
};



/*
* Inode��
* �ڴ�����Inode�ڵ�
* ϵͳ��ÿһ���򿪵��ļ�����ǰ����Ŀ¼�����ص����ļ�ϵͳ����ӦΨһ���ڴ�Inode
*/
class Inode {
	/*=== Enums ===*/
public:
	enum InodeFlag {
		ILOCK = 0x1, // ��
		IUPD = 0x2, // �ڴ�inode�ڵ㱻�޸Ĺ�����Ҫ������Ӧ�����inode
		IACC = 0x4, // 
		IMOUNT = 0x8, //
		IWANT = 0x10,
		ITEXT = 0x20
	};

	/*=== Static Consts ===*/
public:
	static const unsigned int IALLOC = 0x8000; // �ļ���ʹ��
	static const unsigned int IFMT = 0x6000; // �ļ���������
	static const unsigned int IFDIR = 0x4000; // �ļ����ͣ�Ŀ¼�ļ�
	static const unsigned int IDCHR = 0x2000; // �ַ��豸���������ļ�
	static const unsigned int IFBLK = 0x6000; // ���豸���������ļ���Ϊ0��ʾ���������ļ�
	static const unsigned int ILARG = 0x1000; // �ļ��������ͣ����ͻ�����ļ�
	static const unsigned int IREAD = 0x100; // ��Ȩ��
	static const unsigned int IWRITE = 0x80; // дȨ��


	static const int i_flag_init = 0;
	static const int i_mode_init = 0;
	static const int i_count_init = 0;
	static const int i_nlink_init = 0;
	static const int i_size_init = 0;
	static const int i_number_init = -1;

	static const int BLOCK_SIZE = 512; // �̿��С
	static const int ADDRESS_PER_BLOCK = BLOCK_SIZE / sizeof(int); // ÿ������������������̿�ŵĸ���

	static const int SMALL_FILE_INDEX = 6;
	static const int LARGE_FILE_INDEX = 8;
	static const int HUGE_FILE_INDEX = 10;

	static const int SMALL_FILE_BLOCK_NUM = 6; // С���ļ���ռ�̿���
	static const int LARGE_FILE_BLOCK_NUM = 128 * 2 + 6; // �����ļ���ռ�̿���
	static const int HUGE_FILE_BLOCK_NUM = 128 * 128 * 2 + 128 * 2 + 6; // �����ļ���ռ�̿���

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
	unsigned int i_flag; // ״̬��־λ
	unsigned int i_mode; // �ļ�������ʽ
	int i_count;
	int i_nlink;
	int i_number; // ���inode���ڵ�ı��
	int i_size; // �ļ���С����λ�ֽ�
	int i_addr[10]; // �ļ��߼���ź�������ת����
	
};

#endif