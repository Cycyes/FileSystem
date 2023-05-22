#include "Global.h"
#include "FileSystem.h"

#include <iostream>
#include <cstring>

using namespace std;

/*============================== SuperBlock ==============================*/
SuperBlock::SuperBlock() {

}

SuperBlock::~SuperBlock() {

}
/*============================== SuperBlock ==============================*/



/*============================== FileSystem ==============================*/
FileSystem::FileSystem() {

	this->m_SuperBlock = &globalSuperBlock;
	this->m_BufferManager = &globalBufferManager;
	this->m_DeviceManager = &globalDeviceManager;

	if (!this->m_DeviceManager->IsExist()) {
		this->Format();
	}
	else {
		this->LoadSuperBlock();
	}
}

FileSystem::~FileSystem() {
	this->Update();
}

void FileSystem::FormatSuperBlock() {

	this->m_SuperBlock->s_isize = inode_block_num;
	this->m_SuperBlock->s_fsize = disk_block_num;
	this->m_SuperBlock->s_nfree = nfree_init;
	this->m_SuperBlock->s_free[0] = -1;
	this->m_SuperBlock->s_ninode = ninode_init;
}

void FileSystem::LoadSuperBlock() {

	m_DeviceManager->read(m_SuperBlock, sizeof(SuperBlock), superblock_offset); // ��ȡSuperBlock
}

void FileSystem::Update() {

	/* ����ȫ�ֱ��� */
	InodeTable& inodeTable = globalINodeTable;

	/* ��superblockд�� */
	for (int i = 0; i < superblock_block_num; i++) {
		Buf* bp = this->m_BufferManager->GetBlk(i); // ���̿���뻺��
		memcpy(bp->b_addr, (int*)this->m_SuperBlock + i * 128 , block_size); // ���»���
		m_BufferManager->Bwrite(bp); // д��
	}

	/* ��InodeTableд�ش��� */
	inodeTable.UpdateInodeTable();

	/* �������ӳ�д��bufferд�ش��� */
	m_BufferManager->Bflush();
}

void FileSystem::Format() {

	/* ��SuperBlock��ʼ����д����� */
	this->FormatSuperBlock();
	this->m_DeviceManager->Open();
	this->m_DeviceManager->write(this->m_SuperBlock, sizeof(SuperBlock), superblock_offset);

	/* ����Ŀ¼��ʼ����д����� */
	DiskInode root;
	root.d_mode |= Inode::IALLOC | Inode::IFDIR;
	root.d_nlink = 1;
	this->m_DeviceManager->write(&root, sizeof(root));

	/* ��ʼ��SuperBlock��s_inode����д�����inode�� */
	DiskInode diskInode;
	for (int i = 1; i < inode_num; i++) {
		if (this->m_SuperBlock->s_ninode < s_inode_size) {
			this->m_SuperBlock->s_inode[this->m_SuperBlock->s_ninode++] = i;
		}
		this->m_DeviceManager->write(&diskInode, sizeof(diskInode));
	}

	char block[block_size], buffer[block_size];
	memset(block, 0, sizeof(block));

	for (int i = 0; i < data_block_num; i++) {
		if (this->m_SuperBlock->s_nfree >= s_free_size) {
			memcpy(buffer, &this->m_SuperBlock->s_nfree, sizeof(this->m_SuperBlock->s_free) + sizeof(int));
			this->m_DeviceManager->write(buffer, block_size);
			this->m_SuperBlock->s_nfree = nfree_init;
		}
		else {
			this->m_DeviceManager->write(block, block_size);
		}
		this->m_SuperBlock->s_free[this->m_SuperBlock->s_nfree++] = i + data_offset;
	}

	/* ����д��superblock */
	this->m_DeviceManager->write(this->m_SuperBlock, sizeof(SuperBlock), superblock_offset);
}

Inode* FileSystem::IAlloc() {

	/* ����ȫ�ֱ��� */
	User& user = globalUser;
	InodeTable& inodeTable = globalINodeTable;

	int inumber; // ���䵽�Ŀ������Inode���

	/* �ж�SuperBlock�Ƿ���ֱ�ӹ���Ŀ���Inode */
	if (this->m_SuperBlock->s_ninode <= 0) {
		inumber = NOT_FOUND;

		/* ��������Inode���Ĵ��̿� */
		for (int i = 0; i < this->m_SuperBlock->s_isize; i++) {
			Buf* bp = this->m_BufferManager->Bread(inode_offset + i); // ������Inode���뻺��
			int* addrp = (int*)bp->b_addr; // ȡ���������׵�ַ
			/* ����ô����е��������Inode */
			for (int j = 0; j < inode_num_per_block; j++) {
				inumber++;
				int mode = *(addrp + j * sizeof(DiskInode) / sizeof(int)); // ȡ��������j��DiskInode��mode

				/* �ж��Ƿ�ռ�� */
				if (mode) {
					continue;
				}

				/* �ж�δ���µĴ���Inode���ڴ�Inode�� */
				if (inodeTable.IsLoaded(inumber) == NOT_FOUND) {
					/* ������������� */
					this->m_SuperBlock->s_inode[this->m_SuperBlock->s_ninode++] = inumber;
					/* �ж��Ƿ�װ�� */
					if (this->m_SuperBlock->s_ninode >= s_inode_size) {
						break;
					}
				}
			}
			this->m_BufferManager->Brelse(bp); // �ͷŻ���
			/* �ж��Ƿ�װ�� */
			if (this->m_SuperBlock->s_ninode >= s_inode_size) {
				break;
			}
		}

		/* û���ҵ��κο���Inode */
		if (m_SuperBlock->s_ninode <= 0) {
			user.u_error = User::myENOSPC;
			return NULL;
		}
	}

	inumber = this->m_SuperBlock->s_inode[--this->m_SuperBlock->s_ninode];

	/* ������е��ڴ�Inode */
	Inode* ip = inodeTable.IGet(inumber);
	if (ip == NULL) {
		cerr << "û�ÿ��е��ڴ�Inode" << endl;
		return NULL;
	}
	ip->Clean();

	return ip;
}

void FileSystem::IFree(const int& inumber) {

	/* �ж�ֱ�ӹ�������Inode */
	if (this->m_SuperBlock->s_ninode >= s_inode_size) {
		return; // ֱ��ɢ���ڴ���Inode��
	}

	/* ���뵽ֱ�ӹ�������Inode */
	this->m_SuperBlock->s_inode[this->m_SuperBlock->s_ninode++] = inumber;
}

Buf* FileSystem::Alloc() {

	/* ����ȫ�ֱ��� */
	User& user = globalUser;

	int blkno = this->m_SuperBlock->s_free[--this->m_SuperBlock->s_nfree]; // ��������ջ��ȡ���д��̿��

	/* ������� */
	if (blkno <= 0) {
		m_SuperBlock->s_nfree = 0;
		user.u_error = User::myENOSPC;
		return NULL;
	}

	/* �ж��Ƿ�ջ�� */
	if (this->m_SuperBlock->s_nfree <= 0) {
		Buf* bp = this->m_BufferManager->Bread(blkno); // ��ջ�����̿���뻺��
		int* addrp = (int*)bp->b_addr; // ȡ�����׵�ַ

		/* ����m_SpuerBlock */
		this->m_SuperBlock->s_nfree = *addrp++; // ��������̿���
		memcpy(this->m_SuperBlock->s_free, addrp, sizeof(this->m_SuperBlock->s_free)); // ��������̿��

		this->m_BufferManager->Brelse(bp); // �ͷŻ���
	}

	/* ��������̿��Ӧ�Ļ��� */
	Buf* bp = this->m_BufferManager->GetBlk(blkno);
	if (bp) {
		this->m_BufferManager->Bclear(bp); // ������
	}

	return bp;
}

void FileSystem::Free(const int& blkno) {
	
	/* �ж�ֱ�ӹ���Ŀ����̿��Ƿ����� */
	if (this->m_SuperBlock->s_nfree >= s_free_size) {
		/* ����ǰ��ǰ�̿����blkno */
		Buf* bp = this->m_BufferManager->GetBlk(blkno); // ���仺��
		int* addrp = (int*)bp->b_addr; // ���濪ʼλ��
		*addrp++ = this->m_SuperBlock->s_nfree; // ���̿���д�뻺��
		memcpy(addrp, this->m_SuperBlock->s_free, sizeof(int) * s_free_size); // ���̿��д�뻺��
		this->m_BufferManager->Bwrite(bp); // д�ش���
		/* ����superblock */
		this->m_SuperBlock->s_nfree = 0;
	}
	/* blkno��������̿� */
	this->m_SuperBlock->s_free[this->m_SuperBlock->s_nfree++] = blkno;
}

/*============================== FileSystem ==============================*/