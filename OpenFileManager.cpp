#include "Global.h"

#include <iostream>
#include <cstring>

/*============================== OpenFileTable ==============================*/
OpenFileTable::OpenFileTable() {

}

OpenFileTable::~OpenFileTable() {

}

File* OpenFileTable::FAlloc() {

	/* ����ȫ�ֱ��� */
	User& user = globalUser;

	int fd = user.u_ofiles.AllocFreeSlot(); // �ڽ��̴��ļ�����������ȡ������

	if (fd < 0) {
		return NULL;
	}

	/* ����ϵͳ���ļ��� */
	for (int i = 0; i < FILE_SIZE; i++) {
		/* �ж��Ƿ��ǿ���File */
		if (this->m_File[i].f_count == 0) {
			/* �������̴��ļ���������ϵͳ���ļ���֮��Ĺ�����ϵ */
			user.u_ofiles.SetF(fd, &this->m_File[i]);
			/* ����m_File[i] */
			this->m_File[i].f_count++;
			this->m_File[i].f_offset = 0;

			return (&this->m_File[i]);
		}
	}

	user.u_error = User::myENFILE;
	return NULL;
}

void OpenFileTable::CloseF(File* fp) {

	/* ����ȫ�ֱ��� */
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

	/* �����ڴ�Inode�� */
	for (int i = 0; i < INODE_SIZE; i++) {
		if (this->m_Inode[i].i_number == inumber && this->m_Inode[i].i_count != 0) {
			return i;
		}
	}
	return NOT_FOUND;
}

Inode* InodeTable::GetFreeInode() {

	/* �����ڴ�Inode�� */
	for (int i = 0; i < INODE_SIZE; i++) {
		/* �жϸ��ڴ�Inode�����Ƿ�Ϊ0 */
		if (this->m_Inode[i].i_count == 0) {
			return m_Inode + i;
		}
	}
	return NULL;
}

Inode* InodeTable::IGet(const int& inumber) {

	/* ����ȫ�ֱ��� */
	User& user = globalUser;
	BufferManager& bufferManager = globalBufferManager;

	Inode* ip;
	
	int index = this->IsLoaded(inumber); // ȡ�ڴ�inode��index

	/* �ж��Ƿ�����ڴ�Inode */
	if (index >= 0) {
		ip = m_Inode + index;
		ip->i_count++;
		return ip;
	}
	else {
		ip = this->GetFreeInode(); // ���ڴ�Inode���з������Indoe

		if (ip == NULL) {
			cerr << "�ڴ�InodeTable������" << endl;
			user.u_error = User::myENFILE;
			return NULL;
		}
		else {
			/* ����ip */
			ip->i_number = inumber;
			ip->i_count++;
			/* �������Inode���뻺���� */
			Buf* bp = bufferManager.Bread(FileSystem::inode_offset + inumber / FileSystem::inode_num_per_block);
			/* ���������������ڴ�Inode */
			ip->ICopy(bp, inumber);
			bufferManager.Brelse(bp); // �ͷŻ���
			return ip;
		}
	}
}

void InodeTable::IPut(Inode* ip) {

	if (ip->i_count == 1) {
		if (ip->i_nlink <= 0) { // û��Ŀ¼·��ָ����
			ip->ITrunc(); // �ͷŸ��ļ�ռ�õ������̿�
			ip->i_mode = 0;
			this->m_FileSystem->IFree(ip->i_number); // �ͷŶ�Ӧ�����Inode
		}

		ip->IUpdate(); // �������Inode��Ϣ

		/* �����ڴ�Inode */
		ip->i_flag = Inode::i_flag_init;
		ip->i_number = Inode::i_number_init;
	}

	ip->i_count--; // ��������
}

void InodeTable::UpdateInodeTable() {

	/* �����ڴ�Inode�� */
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