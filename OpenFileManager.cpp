#include "Global.h"

#include <iostream>
#include <cstring>

/*============================== OpenFileTable ==============================*/
OpenFileTable::OpenFileTable() {

}

OpenFileTable::~OpenFileTable() {

}

File* OpenFileTable::FAlloc() {

	/* 引用全局变量 */
	User& user = globalUser;

	int fd = user.u_ofiles.AllocFreeSlot(); // 在进程打开文件描述符表中取空闲项

	if (fd < 0) {
		return NULL;
	}

	/* 遍历系统打开文件表 */
	for (int i = 0; i < FILE_SIZE; i++) {
		/* 判断是否是空闲File */
		if (this->m_File[i].f_count == 0) {
			/* 建立进程打开文件描述符与系统打开文件表之间的勾连关系 */
			user.u_ofiles.SetF(fd, &this->m_File[i]);
			/* 更新m_File[i] */
			this->m_File[i].f_count++;
			this->m_File[i].f_offset = 0;

			return (&this->m_File[i]);
		}
	}

	user.u_error = User::myENFILE;
	return NULL;
}

void OpenFileTable::CloseF(File* fp) {

	/* 引用全局变量 */
	InodeTable& inodeTable = globalINodeTable;

	fp->f_count--;
	if (fp->f_count <= 0) {
		inodeTable.IPut(fp->f_inode);
	}
}

void OpenFileTable::Format() {

	File file;
	for (int i = 0; i < FILE_SIZE; i++) {
		memcpy(m_File + i, &file, sizeof(File));
	}
}
/*============================== OpenFileTable ==============================*/



/*============================== InodeTable ==============================*/
InodeTable::InodeTable() {

	this->m_FileSystem = &globalFileSystem;
}

InodeTable::~InodeTable() {

}

int InodeTable::IsLoaded(const int& inumber) {

	/* 遍历内存Inode表 */
	for (int i = 0; i < INODE_SIZE; i++) {
		if (this->m_Inode[i].i_number == inumber && this->m_Inode[i].i_count != 0) {
			return i;
		}
	}
	return NOT_FOUND;
}

Inode* InodeTable::GetFreeInode() {

	/* 遍历内存Inode表 */
	for (int i = 0; i < INODE_SIZE; i++) {
		/* 判断该内存Inode引用是否为0 */
		if (this->m_Inode[i].i_count == 0) {
			return m_Inode + i;
		}
	}
	return NULL;
}

Inode* InodeTable::IGet(const int& inumber) {

	/* 引用全局变量 */
	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;

	Inode* ip;
	
	int index = this->IsLoaded(inumber); // 取内存inode的index

	/* 判断是否存在内存Inode */
	if (index >= 0) {
		ip = m_Inode + index;
		ip->i_count++;
		return ip;
	}
	else {
		ip = this->GetFreeInode(); // 在内存Inode表中分配空闲Indoe

		if (ip == NULL) {
			cerr << "内存InodeTable已满！" << endl;
			user.u_error = User::myENFILE;
			return NULL;
		}
		else {
			/* 更新ip */
			ip->i_number = inumber;
			ip->i_count++;
			/* 将该外存Inode读入缓冲区 */
			Buf* bp = bufferManager.Bread(FileSystem::inode_offset + inumber / FileSystem::inode_num_per_block);
			/* 将缓冲区拷贝到内存Inode */
			ip->ICopy(bp, inumber);
			bufferManager.Brelse(bp); // 释放缓存
			return ip;
		}
	}
}

void InodeTable::IPut(Inode* ip) {

	if (ip->i_count == 1) {
		if (ip->i_nlink <= 0) { // 没有目录路径指向它
			ip->ITrunc(); // 释放该文件占用的数据盘快
			ip->i_mode = 0;
			this->m_FileSystem->IFree(ip->i_number); // 释放对应的外存Inode
		}

		ip->IUpdate(); // 更新外存Inode信息

		/* 更新内存Inode */
		ip->i_flag = Inode::i_flag_init;
		ip->i_number = Inode::i_number_init;
	}

	ip->i_count--; // 减少引用
}

void InodeTable::UpdateInodeTable() {

	/* 遍历内存Inode表 */
	for (int i = 0; i < INODE_SIZE; i++) {
		if (this->m_Inode[i].i_count) {
			this->m_Inode[i].IUpdate();
		}
	}
}

void InodeTable::Format() {

	Inode inode;
	for (int i = 0; i < INODE_SIZE; i++) {
		memcpy(m_Inode + i, &inode, sizeof(Inode));
	}
}
/*============================== InodeTable ==============================*/