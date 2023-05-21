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

	int phyBlkno; // 物理盘块号

	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;
	FileSystem& fileSystem = globalFileSystem;

	/* 判断lbn合理范围 */
	if (lbn >= HUGE_FILE_BLOCK_NUM) {
		user.u_error = User::myEFBIG;
		return 0;
	}

	if (lbn < SMALL_FILE_BLOCK_NUM) { // 小型文件
		Buf* bp;
		phyBlkno = this->i_addr[lbn]; // 直接从i_addr读取

		/* 如果还没分配该物理盘块，则分配空闲盘块 */
		if (phyBlkno == 0 && (bp = fileSystem.Alloc()) != NULL) {
			bufferManager.Bdwrite(bp); // 设立延迟写
			phyBlkno = bp->b_blkno; // 更新物理盘块号
			/* 更新inode节点 */
			this->i_addr[lbn] = phyBlkno;
			this->setUPD();
		}
	
		return phyBlkno;
	}
	else { // 大型、巨型文件
		Buf* firstBp;
		Buf* secondBp;

		/* 计算逻辑块号lbn对应的i_addr中的索引 */
		int index = lbn < LARGE_FILE_BLOCK_NUM ? (lbn - SMALL_FILE_BLOCK_NUM) / ADDRESS_PER_BLOCK + SMALL_FILE_INDEX : (lbn - LARGE_FILE_BLOCK_NUM) / (ADDRESS_PER_BLOCK * ADDRESS_PER_BLOCK) + LARGE_FILE_INDEX;

		phyBlkno = this->i_addr[index]; // 取第一级物理盘块

		/* 判断是否分配 */
		if (phyBlkno == 0) { // 未分配
			this->setUPD(); // 设置IUPD
			/* 分配空闲盘块 */
			if ((firstBp = fileSystem.Alloc()) == NULL) {
				return 0;
			}
			/* 更新Inode节点 */
			this->i_addr[index] = firstBp->b_blkno; // 设置i_addr
		}
		else { // 已分配
			firstBp = bufferManager.Bread(phyBlkno); // 读取phyBlkno物理盘块进入firstBp缓存
		}

		int* indexTable = (int*)firstBp->b_addr; // 取缓冲区缓冲内容

		if (index >= LARGE_FILE_INDEX) { // 巨型文件
			index = ((lbn - LARGE_FILE_BLOCK_NUM) / ADDRESS_PER_BLOCK) % ADDRESS_PER_BLOCK; // 计算二级间接索引
			phyBlkno = indexTable[index]; // 取物理盘块号
			/* 判断是否分配 */
			if (phyBlkno == 0) { // 未分配
				/* 分配空闲盘块 */
				if ((secondBp = fileSystem.Alloc()) == NULL) { // 分配失败
					bufferManager.Brelse(firstBp); // 释放第一次申请的空间
					return 0;
				}
				indexTable[index] = secondBp->b_blkno; // 更新缓存
				bufferManager.Bdwrite(firstBp); // 设置延迟写
			}
			else {
				bufferManager.Brelse(firstBp); // 释放缓存
				secondBp = bufferManager.Bread(phyBlkno); // 将其读入缓存
			}

			firstBp = secondBp;
			indexTable = (int*)secondBp->b_addr;
		}

		index = lbn < LARGE_FILE_BLOCK_NUM ? (lbn - SMALL_FILE_BLOCK_NUM) % ADDRESS_PER_BLOCK : (lbn - LARGE_FILE_BLOCK_NUM) % ADDRESS_PER_BLOCK; // 计算一级间接索引
		
		if ((phyBlkno = indexTable[index]) == 0 && (secondBp = fileSystem.Alloc()) != NULL) { // 分配空闲盘块成功
			phyBlkno = secondBp->b_blkno; // 读取分配得到的块号

			indexTable[index] = phyBlkno;
			bufferManager.Bdwrite(secondBp);
			bufferManager.Bdwrite(firstBp);
		}
		else {
			bufferManager.Brelse(firstBp); // 释放一级简介索引缓存
		}

		return phyBlkno;
	}
}

void Inode::ReadI() {

	int lbn; // 文件逻辑块号
	int bn; // lbn对应的物理盘块号
	int offset; // 当前字符块内起始传送位置
	int n; // 本次需要传送的字节数

	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;

	/* 判断需要读取的字节数 */
	if (user.u_IOParam.m_Count == 0) {
		return;
	}

	//  this->i_flag |= Inode::IACC;

	/* 读取 */
	while (user.u_error == User::myNOERROR && user.u_IOParam.m_Count != 0) {
		/* 根据user的IOParam计算lbn, bn, offset */
		lbn = bn = user.u_IOParam.m_Offset / BLOCK_SIZE;
		offset = user.u_IOParam.m_Offset % BLOCK_SIZE;
		n = min(BLOCK_SIZE - offset, user.u_IOParam.m_Count);

		int remain = this->i_size - user.u_IOParam.m_Offset;
		if (remain <= 0) { // 已读到文件尾
			return;
		}
		n = min(n, remain); // 更新n
		if ((bn = this->Bmap(lbn)) == 0) { // 找不到对应的物理盘块
			return;
		}

		Buf* bp = bufferManager.Bread(bn); // 读取盘块到缓存bp中

		/* 读操作：从缓存读到用户目标区 */
		memcpy(user.u_IOParam.m_Base, bp->b_addr + offset, n);

		/* 更新user结构中的IOParam */
		user.u_IOParam.m_Base += n;
		user.u_IOParam.m_Offset += n;
		user.u_IOParam.m_Count -= n;

		bufferManager.Brelse(bp); // 释放缓存
	}
}

void Inode::WriteI() {

	int lbn; // 文件逻辑块号
	int bn; // lbn对应的物理盘块号
	int offset; // 当前字符块内起始传送位置
	int n; // 本次需要传送的字节数

	Buf* bp;

	/* 引用全局变量 */
	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;

	this->setUPD(); // 设置Inode需要更新

	/* 判断需要读取的字节数 */
	if (user.u_IOParam.m_Count == 0) {
		return;
	}

	while (user.u_error == User::myNOERROR && user.u_IOParam.m_Count != 0) {
		/* 更新lbn，offset，n */
		lbn = user.u_IOParam.m_Offset / BLOCK_SIZE;
		offset = user.u_IOParam.m_Offset % BLOCK_SIZE;
		n = min(BLOCK_SIZE - offset, user.u_IOParam.m_Count);

		/* 分配物理盘块 */
		if ((bn = this->Bmap(lbn)) == 0) {
			return;
		}

		/* 判断是否正好写满一个盘块 */
		if (n == BLOCK_SIZE) { // 写满
			bp = bufferManager.GetBlk(bn); // 分配缓存
		}
		else {
			bp = bufferManager.Bread(bn); // 先读后写（读出该块以保护不需要重写的数据）
		}

		/* 写操作：从用户目标区拷贝到缓冲区 */
		memcpy((unsigned char*)(bp->b_addr + offset), user.u_IOParam.m_Base, n);

		/* 更新user */
		user.u_IOParam.m_Base += n;
		user.u_IOParam.m_Offset += n;
		user.u_IOParam.m_Count -= n;

		if (user.u_error != User::myNOERROR) {
			bufferManager.Brelse(bp); // 写过程出错，释放缓存
		}

		bufferManager.Bdwrite(bp); // 设置延迟写

		/* 改变文件长度 */
		if (this->i_size < user.u_IOParam.m_Offset) {
			this->i_size = user.u_IOParam.m_Offset;
		}

		this->setUPD(); // 设置UPD
	}
}

void Inode::IUpdate(const int& time) {

	BufferManager& bufferManager = globalBufferManager;

	if (this->checkUPD()) {
		/* 将DiskInode读取到缓冲区 */
		Buf* bp = bufferManager.Bread(FileSystem::INODE_ZONE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);

		/* 将内存Inode的信息复制到diskInode中 */
		DiskInode diskInode;
		diskInode.d_mode = this->i_mode;
		diskInode.d_nlink = this->i_nlink;
		diskInode.d_size = this->i_size;
		for (int i = 0; i < 10; i++) {
			diskInode.d_addr[i] = this->i_addr[i];
		}

		/* 用diskInode覆盖缓存中的外存Inode */
		memcpy((unsigned char*)bp->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode), &diskInode, sizeof(DiskInode));

		/* 将缓存写回磁盘 */
		bufferManager.Bwrite(bp);
	}
}

void Inode::ITrunc() {

	/* 引用全局变量 */
	BufferManager& bufferManager = globalBufferManager;
	FileSystem& fileSystem = globalFileSystem;

	for (int i = HUGE_FILE_INDEX - 1; i >= 0; i--) {
		/* 判断是否存在索引项 */
		if (this->i_addr[i]) {
			/* 判断是否是间接索引 */
			if (i >= SMALL_FILE_INDEX && i < HUGE_FILE_INDEX) {
				Buf* bp = bufferManager.Bread(this->i_addr[i]); // 将简介索引表读入缓存
				int* indexTable = (int*)bp->b_addr; // 取缓冲区首地址
				/* 遍历每一条记录 */
				for (int j = ADDRESS_PER_BLOCK - 1; j >= 0; j--) {
					/* 判断是否存在该索引项 */
					if (indexTable[j]) {
						/* 判断是否是二级间接索引表 */
						if (i >= LARGE_FILE_INDEX && i < HUGE_FILE_INDEX) {
							Buf* bpN = bufferManager.Bread(indexTable[j]);
							int* indexTableN = (int*)bpN->b_addr;
							for (int k = ADDRESS_PER_BLOCK - 1; k >= 0; k--) {
								if (indexTableN[k]) {
									fileSystem.Free(indexTableN[k]); // 释放指定的磁盘块
								}
							}
							bufferManager.Brelse(bpN); // 释放缓存
						}
						fileSystem.Free(indexTable[j]); // 释放指定的磁盘块
					}
				}
				bufferManager.Brelse(bp); // 释放缓存
			}
			fileSystem.Free(this->i_addr[i]); // 释放指定的磁盘块
			this->i_addr[i] = 0;
		}
	}
	/* 更新inode */
	this->i_size = 0;
	this->i_nlink = 1;
	this->clearLARG();
	this->setUPD();
}

void Inode::ICopy(Buf* bp, const int& inumber) {
	/* 取外存Inode节点到diskInode */
	DiskInode& diskInode = *(DiskInode*)(bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode));

	/* 将diskInode复制到内存Inode节点 */
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