#include "Global.h"
#include "Common.h"

#include <cstring>

/*============================== File ==============================*/
File::File() {
	this->f_count = f_count_init;
	this->f_flag = f_flag_init;
	this->f_offset = f_offset_init;
	this->f_inode = NULL;
}

File::~File() {

}
/*============================== File ==============================*/



/*============================== OpenFiles ==============================*/
OpenFiles::OpenFiles() {

	memset(this->ProcessOpenFileTable, NULL, sizeof(this->ProcessOpenFileTable));
}

OpenFiles::~OpenFiles() {

}

int OpenFiles::AllocFreeSlot() {
	
	/* ����ȫ�ֱ��� */
	User& user = globalUser;

	/* �������̴��ļ��������� */
	for (int i = 0; i < OPEN_FILES_NUM; i++) {
		/* �ж��Ƿ��ǿ����� */
		if (this->ProcessOpenFileTable[i] == NULL) { 
			user.u_ar0[User::EAX] = i;
			return i;
		}
	}

	user.u_ar0[User::EAX] = NOT_FOUND;
	user.u_error = User::myEMFILE;
	return NOT_FOUND;
}

File* OpenFiles::GetF(const int& fd) {

	/* ����ȫ�ֱ��� */
	User& user = globalUser;

	/* �ж�fd�����ⷶΧ */
	if (fd < 0 || fd >= OPEN_FILES_NUM) {
		user.u_error = User::myEBADF;
		return NULL;
	}

	File* fp = this->ProcessOpenFileTable[fd];
	if (fp == NULL) {
		user.u_error = User::myEBADF;
	}
	return fp;
}

void OpenFiles::SetF(const int& fd, File* fp) {

	/* �ж�fd�����ⷶΧ */
	if (fd < 0 || fd >= OPEN_FILES_NUM) {
		return;
	}
	/* ���½��̴��ļ��������� */
	this->ProcessOpenFileTable[fd] = fp;
}
/*============================== OpenFiles ==============================*/



/*============================== IOParameter ==============================*/
IOParameter::IOParameter() {

}

IOParameter::~IOParameter() {

}
/*============================== IOParameter ==============================*/