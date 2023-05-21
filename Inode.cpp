#include "Global.h"

#include <algorithm>
#include <cstring>

/*============================== DiskInode ==============================*/
DiskInode::DiskInode() {

	this->d_mode = d_mode_init;
	this->d_nlink = d_mode_init;
	this->d_size = 0;
	memset(this->d_addr, 0, sizeof(this->d_addr));
}

DiskInode::~DiskInode() {

}
/*============================== DiskInode ==============================*/



/*============================== Inode ==============================*/
Inode::Inode() {

	this->i_flag = i_flag_init;
	this->i_mode = i_mode_init;
	this->i_count = i_count_init;
	this->i_nlink = i_nlink_init;
	this->i_size = i_size_init;
	this->i_number = i_number_init;
	memset(this->i_addr, 0, sizeof(this->i_addr));
}

Inode::~Inode() {

}

bool Inode::checkUPD() {
	
	return this->i_flag & IUPD;
}

void Inode::setUPD() {

	this->i_flag |= IUPD;
}

void Inode::clearLARG() {

	this->i_mode &= ~(ILARG);
}

int Inode::Bmap(const int& lbn) {

	int phyBlkno; // �����̿��

	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;
	FileSystem& fileSystem = globalFileSystem;

	/* �ж�lbn����Χ */
	if (lbn >= HUGE_FILE_BLOCK_NUM) {
		user.u_error = User::myEFBIG;
		return 0;
	}

	if (lbn < SMALL_FILE_BLOCK_NUM) { // С���ļ�
		Buf* bp;
		phyBlkno = this->i_addr[lbn]; // ֱ�Ӵ�i_addr��ȡ

		/* �����û����������̿飬���������̿� */
		if (phyBlkno == 0 && (bp = fileSystem.Alloc()) != NULL) {
			bufferManager.Bdwrite(bp); // �����ӳ�д
			phyBlkno = bp->b_blkno; // ���������̿��
			/* ����inode�ڵ� */
			this->i_addr[lbn] = phyBlkno;
			this->setUPD();
		}
	
		return phyBlkno;
	}
	else { // ���͡������ļ�
		Buf* firstBp;
		Buf* secondBp;

		/* �����߼����lbn��Ӧ��i_addr�е����� */
		int index = lbn < LARGE_FILE_BLOCK_NUM ? (lbn - SMALL_FILE_BLOCK_NUM) / ADDRESS_PER_BLOCK + SMALL_FILE_INDEX : (lbn - LARGE_FILE_BLOCK_NUM) / (ADDRESS_PER_BLOCK * ADDRESS_PER_BLOCK) + LARGE_FILE_INDEX;

		phyBlkno = this->i_addr[index]; // ȡ��һ�������̿�

		/* �ж��Ƿ���� */
		if (phyBlkno == 0) { // δ����
			this->setUPD(); // ����IUPD
			/* ��������̿� */
			if ((firstBp = fileSystem.Alloc()) == NULL) {
				return 0;
			}
			/* ����Inode�ڵ� */
			this->i_addr[index] = firstBp->b_blkno; // ����i_addr
		}
		else { // �ѷ���
			firstBp = bufferManager.Bread(phyBlkno); // ��ȡphyBlkno�����̿����firstBp����
		}

		int* indexTable = (int*)firstBp->b_addr; // ȡ��������������

		if (index >= LARGE_FILE_INDEX) { // �����ļ�
			index = ((lbn - LARGE_FILE_BLOCK_NUM) / ADDRESS_PER_BLOCK) % ADDRESS_PER_BLOCK; // ��������������
			phyBlkno = indexTable[index]; // ȡ�����̿��
			/* �ж��Ƿ���� */
			if (phyBlkno == 0) { // δ����
				/* ��������̿� */
				if ((secondBp = fileSystem.Alloc()) == NULL) { // ����ʧ��
					bufferManager.Brelse(firstBp); // �ͷŵ�һ������Ŀռ�
					return 0;
				}
				indexTable[index] = secondBp->b_blkno; // ���»���
				bufferManager.Bdwrite(firstBp); // �����ӳ�д
			}
			else {
				bufferManager.Brelse(firstBp); // �ͷŻ���
				secondBp = bufferManager.Bread(phyBlkno); // ������뻺��
			}

			firstBp = secondBp;
			indexTable = (int*)secondBp->b_addr;
		}

		index = lbn < LARGE_FILE_BLOCK_NUM ? (lbn - SMALL_FILE_BLOCK_NUM) % ADDRESS_PER_BLOCK : (lbn - LARGE_FILE_BLOCK_NUM) % ADDRESS_PER_BLOCK; // ����һ���������
		
		if ((phyBlkno = indexTable[index]) == 0 && (secondBp = fileSystem.Alloc()) != NULL) { // ��������̿�ɹ�
			phyBlkno = secondBp->b_blkno; // ��ȡ����õ��Ŀ��

			indexTable[index] = phyBlkno;
			bufferManager.Bdwrite(secondBp);
			bufferManager.Bdwrite(firstBp);
		}
		else {
			bufferManager.Brelse(firstBp); // �ͷ�һ�������������
		}

		return phyBlkno;
	}
}

void Inode::ReadI() {

	int lbn; // �ļ��߼����
	int bn; // lbn��Ӧ�������̿��
	int offset; // ��ǰ�ַ�������ʼ����λ��
	int n; // ������Ҫ���͵��ֽ���

	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;

	/* �ж���Ҫ��ȡ���ֽ��� */
	if (user.u_IOParam.m_Count == 0) {
		return;
	}

	//  this->i_flag |= Inode::IACC;

	/* ��ȡ */
	while (user.u_error == User::myNOERROR && user.u_IOParam.m_Count != 0) {
		/* ����user��IOParam����lbn, bn, offset */
		lbn = bn = user.u_IOParam.m_Offset / BLOCK_SIZE;
		offset = user.u_IOParam.m_Offset % BLOCK_SIZE;
		n = min(BLOCK_SIZE - offset, user.u_IOParam.m_Count);

		int remain = this->i_size - user.u_IOParam.m_Offset;
		if (remain <= 0) { // �Ѷ����ļ�β
			return;
		}
		n = min(n, remain); // ����n
		if ((bn = this->Bmap(lbn)) == 0) { // �Ҳ�����Ӧ�������̿�
			return;
		}

		Buf* bp = bufferManager.Bread(bn); // ��ȡ�̿鵽����bp��

		/* ���������ӻ�������û�Ŀ���� */
		memcpy(user.u_IOParam.m_Base, bp->b_addr + offset, n);

		/* ����user�ṹ�е�IOParam */
		user.u_IOParam.m_Base += n;
		user.u_IOParam.m_Offset += n;
		user.u_IOParam.m_Count -= n;

		bufferManager.Brelse(bp); // �ͷŻ���
	}
}

void Inode::WriteI() {

	int lbn; // �ļ��߼����
	int bn; // lbn��Ӧ�������̿��
	int offset; // ��ǰ�ַ�������ʼ����λ��
	int n; // ������Ҫ���͵��ֽ���

	Buf* bp;

	/* ����ȫ�ֱ��� */
	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;

	this->setUPD(); // ����Inode��Ҫ����

	/* �ж���Ҫ��ȡ���ֽ��� */
	if (user.u_IOParam.m_Count == 0) {
		return;
	}

	while (user.u_error == User::myNOERROR && user.u_IOParam.m_Count != 0) {
		/* ����lbn��offset��n */
		lbn = user.u_IOParam.m_Offset / BLOCK_SIZE;
		offset = user.u_IOParam.m_Offset % BLOCK_SIZE;
		n = min(BLOCK_SIZE - offset, user.u_IOParam.m_Count);

		/* ���������̿� */
		if ((bn = this->Bmap(lbn)) == 0) {
			return;
		}

		/* �ж��Ƿ�����д��һ���̿� */
		if (n == BLOCK_SIZE) { // д��
			bp = bufferManager.GetBlk(bn); // ���仺��
		}
		else {
			bp = bufferManager.Bread(bn); // �ȶ���д�������ÿ��Ա�������Ҫ��д�����ݣ�
		}

		/* д���������û�Ŀ���������������� */
		memcpy((unsigned char*)(bp->b_addr + offset), user.u_IOParam.m_Base, n);

		/* ����user */
		user.u_IOParam.m_Base += n;
		user.u_IOParam.m_Offset += n;
		user.u_IOParam.m_Count -= n;

		if (user.u_error != User::myNOERROR) {
			bufferManager.Brelse(bp); // д���̳����ͷŻ���
		}

		bufferManager.Bdwrite(bp); // �����ӳ�д

		/* �ı��ļ����� */
		if (this->i_size < user.u_IOParam.m_Offset) {
			this->i_size = user.u_IOParam.m_Offset;
		}

		this->setUPD(); // ����UPD
	}
}

void Inode::IUpdate(const int& time) {

	BufferManager& bufferManager = globalBufferManager;

	if (this->checkUPD()) {
		/* ��DiskInode��ȡ�������� */
		Buf* bp = bufferManager.Bread(FileSystem::INODE_ZONE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);

		/* ���ڴ�Inode����Ϣ���Ƶ�diskInode�� */
		DiskInode diskInode;
		diskInode.d_mode = this->i_mode;
		diskInode.d_nlink = this->i_nlink;
		diskInode.d_size = this->i_size;
		for (int i = 0; i < 10; i++) {
			diskInode.d_addr[i] = this->i_addr[i];
		}

		/* ��diskInode���ǻ����е����Inode */
		memcpy((unsigned char*)bp->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode), &diskInode, sizeof(DiskInode));

		/* ������д�ش��� */
		bufferManager.Bwrite(bp);
	}
}

void Inode::ITrunc() {

	/* ����ȫ�ֱ��� */
	BufferManager& bufferManager = globalBufferManager;
	FileSystem& fileSystem = globalFileSystem;

	for (int i = HUGE_FILE_INDEX - 1; i >= 0; i--) {
		/* �ж��Ƿ���������� */
		if (this->i_addr[i]) {
			/* �ж��Ƿ��Ǽ������ */
			if (i >= SMALL_FILE_INDEX && i < HUGE_FILE_INDEX) {
				Buf* bp = bufferManager.Bread(this->i_addr[i]); // �������������뻺��
				int* indexTable = (int*)bp->b_addr; // ȡ�������׵�ַ
				/* ����ÿһ����¼ */
				for (int j = ADDRESS_PER_BLOCK - 1; j >= 0; j--) {
					/* �ж��Ƿ���ڸ������� */
					if (indexTable[j]) {
						/* �ж��Ƿ��Ƕ������������ */
						if (i >= LARGE_FILE_INDEX && i < HUGE_FILE_INDEX) {
							Buf* bpN = bufferManager.Bread(indexTable[j]);
							int* indexTableN = (int*)bpN->b_addr;
							for (int k = ADDRESS_PER_BLOCK - 1; k >= 0; k--) {
								if (indexTableN[k]) {
									fileSystem.Free(indexTableN[k]); // �ͷ�ָ���Ĵ��̿�
								}
							}
							bufferManager.Brelse(bpN); // �ͷŻ���
						}
						fileSystem.Free(indexTable[j]); // �ͷ�ָ���Ĵ��̿�
					}
				}
				bufferManager.Brelse(bp); // �ͷŻ���
			}
			fileSystem.Free(this->i_addr[i]); // �ͷ�ָ���Ĵ��̿�
			this->i_addr[i] = 0;
		}
	}
	/* ����inode */
	this->i_size = 0;
	this->i_nlink = 1;
	this->clearLARG();
	this->setUPD();
}

void Inode::ICopy(Buf* bp, const int& inumber) {
	/* ȡ���Inode�ڵ㵽diskInode */
	DiskInode& diskInode = *(DiskInode*)(bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode));

	/* ��diskInode���Ƶ��ڴ�Inode�ڵ� */
	this->i_mode = diskInode.d_mode;
	this->i_nlink = diskInode.d_nlink;
	this->i_size = diskInode.d_size;
	/*
	for (int i = 0; i < HUGE_FILE_INDEX; i++) {
		this->i_addr[i] = diskInode.d_addr[i];
	}
	*/
	memcpy(this->i_addr, diskInode.d_addr, sizeof(i_addr));
}

void Inode::Clean() {
	this->i_mode = i_mode_init;
	this->i_nlink = i_nlink_init;
	this->i_size = i_size_init;
	memset(this->i_addr, 0, sizeof(this->i_addr));
}
/*============================== Inode ==============================*/