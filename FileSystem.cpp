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

	m_DeviceManager->read(m_SuperBlock, sizeof(SuperBlock), superblock_offset); // 读取SuperBlock
}

void FileSystem::Update() {

	/* 引用全局变量 */
	InodeTable& inodeTable = globalINodeTable;

	/* 将superblock写回 */
	for (int i = 0; i < superblock_block_num; i++) {
		Buf* bp = this->m_BufferManager->GetBlk(i); // 读盘块进入缓存
		memcpy(bp->b_addr, (int*)this->m_SuperBlock + i * 128 , block_size); // 更新缓存
		m_BufferManager->Bwrite(bp); // 写回
	}

	/* 将InodeTable写回磁盘 */
	inodeTable.UpdateInodeTable();

	/* 将所有延迟写的buffer写回磁盘 */
	m_BufferManager->Bflush();
}

void FileSystem::Format() {

	/* 将SuperBlock初始化并写入磁盘 */
	this->FormatSuperBlock();
	this->m_DeviceManager->Open();
	this->m_DeviceManager->write(this->m_SuperBlock, sizeof(SuperBlock), superblock_offset);

	/* 将根目录初始化并写入磁盘 */
	DiskInode root;
	root.d_mode |= Inode::IALLOC | Inode::IFDIR;
	root.d_nlink = 1;
	this->m_DeviceManager->write(&root, sizeof(root));

	/* 初始化SuperBlock的s_inode，并写入磁盘inode区 */
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

	/* 更新写入superblock */
	this->m_DeviceManager->write(this->m_SuperBlock, sizeof(SuperBlock), superblock_offset);
}

Inode* FileSystem::IAlloc() {

	/* 引用全局变量 */
	User& user = globalUser;
	InodeTable& inodeTable = globalINodeTable;

	int inumber; // 分配到的空闲外存Inode编号

	/* 判断SuperBlock是否还有直接管理的空闲Inode */
	if (this->m_SuperBlock->s_ninode <= 0) {
		inumber = NOT_FOUND;

		/* 遍历磁盘Inode区的磁盘块 */
		for (int i = 0; i < this->m_SuperBlock->s_isize; i++) {
			Buf* bp = this->m_BufferManager->Bread(inode_offset + i); // 将磁盘Inode读入缓存
			int* addrp = (int*)bp->b_addr; // 取缓冲区的首地址
			/* 盘里该磁盘中的所有外存Inode */
			for (int j = 0; j < inode_num_per_block; j++) {
				inumber++;
				int mode = *(addrp + j * sizeof(DiskInode) / sizeof(int)); // 取缓冲区第j个DiskInode的mode

				/* 判断是否被占用 */
				if (mode) {
					continue;
				}

				/* 判断未更新的磁盘Inode在内存Inode中 */
				if (inodeTable.IsLoaded(inumber) == NOT_FOUND) {
					/* 加入空闲索引表 */
					this->m_SuperBlock->s_inode[this->m_SuperBlock->s_ninode++] = inumber;
					/* 判断是否装满 */
					if (this->m_SuperBlock->s_ninode >= s_inode_size) {
						break;
					}
				}
			}
			this->m_BufferManager->Brelse(bp); // 释放缓存
			/* 判断是否装满 */
			if (this->m_SuperBlock->s_ninode >= s_inode_size) {
				break;
			}
		}

		/* 没有找到任何空闲Inode */
		if (m_SuperBlock->s_ninode <= 0) {
			user.u_error = User::myENOSPC;
			return NULL;
		}
	}

	inumber = this->m_SuperBlock->s_inode[--this->m_SuperBlock->s_ninode];

	/* 分配空闲的内存Inode */
	Inode* ip = inodeTable.IGet(inumber);
	if (ip == NULL) {
		cerr << "没用空闲的内存Inode" << endl;
		return NULL;
	}
	ip->Clean();

	return ip;
}

void FileSystem::IFree(const int& inumber) {

	/* 判断直接管理的外存Inode */
	if (this->m_SuperBlock->s_ninode >= s_inode_size) {
		return; // 直接散落在磁盘Inode区
	}

	/* 加入到直接管理的外存Inode */
	this->m_SuperBlock->s_inode[this->m_SuperBlock->s_ninode++] = inumber;
}

Buf* FileSystem::Alloc() {

	/* 引用全局变量 */
	User& user = globalUser;

	int blkno = this->m_SuperBlock->s_free[--this->m_SuperBlock->s_nfree]; // 从索引表栈顶取空闲磁盘块号

	/* 分配出错 */
	if (blkno <= 0) {
		m_SuperBlock->s_nfree = 0;
		user.u_error = User::myENOSPC;
		return NULL;
	}

	/* 判断是否栈空 */
	if (this->m_SuperBlock->s_nfree <= 0) {
		Buf* bp = this->m_BufferManager->Bread(blkno); // 将栈顶磁盘块读入缓存
		int* addrp = (int*)bp->b_addr; // 取缓存首地址

		/* 更新m_SpuerBlock */
		this->m_SuperBlock->s_nfree = *addrp++; // 读入空闲盘块数
		memcpy(this->m_SuperBlock->s_free, addrp, sizeof(this->m_SuperBlock->s_free)); // 读入空闲盘块号

		this->m_BufferManager->Brelse(bp); // 释放缓存
	}

	/* 分配空闲盘块对应的缓存 */
	Buf* bp = this->m_BufferManager->GetBlk(blkno);
	if (bp) {
		this->m_BufferManager->Bclear(bp); // 清理缓存
	}

	return bp;
}

void FileSystem::Free(const int& blkno) {
	
	/* 判断直接管理的空闲盘块是否已满 */
	if (this->m_SuperBlock->s_nfree >= s_free_size) {
		/* 将当前空前盘块读入blkno */
		Buf* bp = this->m_BufferManager->GetBlk(blkno); // 分配缓存
		int* addrp = (int*)bp->b_addr; // 缓存开始位置
		*addrp++ = this->m_SuperBlock->s_nfree; // 将盘块数写入缓存
		memcpy(addrp, this->m_SuperBlock->s_free, sizeof(int) * s_free_size); // 将盘块号写入缓存
		this->m_BufferManager->Bwrite(bp); // 写回磁盘
		/* 更新superblock */
		this->m_SuperBlock->s_nfree = 0;
	}
	/* blkno加入空闲盘块 */
	this->m_SuperBlock->s_free[this->m_SuperBlock->s_nfree++] = blkno;
}

/*============================== FileSystem ==============================*/