#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "Common.h"

#include <cstdio>

/*
* DeviceManager��
* �����д�����ļ�
* ����C��ʽ��C++��д�ļ���C��ʽ��Щ������
*/
class DeviceManager {
    /*=== Static Consts===*/
public:
    static const char* DISK_FILE_NAME; // ���̾����ļ���

    /*=== Members ===*/
private:
    FILE* fp; // �����ļ�ָ��

    /*=== Functions ===*/
public:
    DeviceManager(); // ���캯��
    ~DeviceManager(); // ��������

    bool IsExist(); // ��龵���ļ��Ƿ����
    void Open(); // �򿪾����ļ�
    void write(const void* buffer, unsigned int size, int offset = -1, unsigned int origin = SEEK_SET); // д����
    void read(void* buffer, unsigned int size, int offset = -1, unsigned int origin = SEEK_SET); // ������
};

#endif