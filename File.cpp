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
	
	/* 引用全局变量 */
	User& user = globalUser;

	/* 遍历进程打开文件描述符表 */
	for (int i = 0; i < OPEN_FILES_NUM; i++) {
		/* 判断是否是空闲项 */
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

	/* 引用全局变量 */
	User& user = globalUser;

	/* 判断fd的特殊范围 */
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

	/* 判断fd的特殊范围 */
	if (fd < 0 || fd >= OPEN_FILES_NUM) {
		return;
	}
	/* 更新进程打开文件描述符表 */
	this->ProcessOpenFileTable[fd] = fp;
}
/*============================== OpenFiles ==============================*/



/*============================== IOParameter ==============================*/
IOParameter::IOParameter() {

}

IOParameter::~IOParameter() {

}
/*============================== IOParameter ==============================*/