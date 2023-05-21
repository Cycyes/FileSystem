#include "Command.h"
#include "Buf.h"
#include "BufferManager.h"
#include "DeviceManager.h"
#include "File.h"
#include "FileSystem.h"
#include "FileManager.h"
#include "INode.h"
#include "OpenFileManager.h"
#include "User.h"

#include <iostream>

DeviceManager globalDeviceManager;
BufferManager globalBufferManager;
OpenFileTable globalOpenFileTable;
SuperBlock globalSuperBlock;
FileSystem globalFileSystem;
InodeTable globalINodeTable;
FileManager globalFileManager;
User globalUser;

string line;

int main() {
	Command command;
	while (1) {
		cout << "[u2053186@localhost " + globalUser.u_curdir + "]$ ";
		getline(cin, line);

		vector<string> cmdList = command.getCommand(line); // ��ʱ��һ����Ϊ������

		command.executeCommand(cmdList);

	}

	return 0;
}