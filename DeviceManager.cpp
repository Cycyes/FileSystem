#define _CRT_SECURE_NO_WARNINGS

#include "DeviceManager.h"

#include <Stdlib.h>

using namespace std;

const char* DeviceManager::DISK_FILE_NAME = "FileSystem.img";

DeviceManager::DeviceManager() {
    fp = fopen(DISK_FILE_NAME, "rb+");
}

DeviceManager::~DeviceManager() {
    if (fp) {
        fclose(fp);
    }
}

bool DeviceManager::IsExist() {
    return fp != NULL;
}

void DeviceManager::Open() {
    fp = fopen(DISK_FILE_NAME, "wb+");
    if (fp == NULL) {
        printf("�򿪻��½��ļ�%sʧ�ܣ�", DISK_FILE_NAME);
        exit(ERROR_OPEN_FILE);
    }
}

void DeviceManager::write(const void* buffer, unsigned int size, int offset, unsigned int origin) {
    if (offset >= 0) {
        fseek(fp, offset, origin);
    }
    fwrite(buffer, size, 1, fp);
}

void DeviceManager::read(void* buffer, unsigned int size, int offset, unsigned int origin) {
    if (offset >= 0) {
        fseek(fp, offset, origin);
    }
    fread(buffer, size, 1, fp);
}