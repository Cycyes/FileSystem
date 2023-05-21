#ifndef BUF_H
#define BUF_H

using namespace std;

/*
* Buf��
* ��¼�����ʹ�����
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
	unsigned int b_flags; // ������ƿ�ı�־λ

	/* ˫������ */
	Buf* b_prev;
	Buf* b_next;

	int b_wcount; // ��Ҫ���͵��ֽ���
	unsigned char* b_addr; // ָ��û�����ƿ�������Ļ��������׵�ַ
	int b_blkno; // �����߼����

	int b_paddings[b_paddings_size]; // ����ֽ���32
};

#endif
