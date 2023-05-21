#ifndef GLOBAL_H
#define GLOBAL_H

#define ERROR_OPEN_FILE  -1
#define ERROR_PATH_EMPTY -2

#include "Buf.h"
#include "BufferManager.h"
#include "DeviceManager.h"
#include "FileSystem.h"
#include "FileManager.h"
#include "OpenFileManager.h"
#include "User.h"

extern User globalUser;
extern DeviceManager globalDeviceManager;
extern BufferManager globalBufferManager;
extern OpenFileTable globalOpenFileTable;
extern SuperBlock globalSuperBlock;
extern FileSystem globalFileSystem;
extern InodeTable globalINodeTable;
extern FileManager globalFileManager;

#endif