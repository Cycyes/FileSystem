#include "Global.h"

#include "Command.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

vector<string> Command::getCommand(const string& str) {

	/* ��ʼ��cmdList */
	vector<string> cmdList;
	cmdList.clear();

	stringstream cmd(str); // ��str��Ϊ������
	
	string cmdTemp;
	while (cmd >> cmdTemp) {
		cmdList.push_back(cmdTemp);
	}

	return cmdList;
}

void Command::autotest() {
	string filename = "./TestFiles/cmd.txt";
	ifstream infile("./TestFiles/cmd.txt", ios::in | ios::binary);
	if (!infile.is_open()) {
		cerr << "�޷����ļ�" << filename << endl;
		exit(ERROR_OPEN_FILE);
	}

	string buffer;
	while (getline(infile, buffer)) {
		executeCommand(getCommand(buffer));
	}
}

void Command::executeCommand(const vector<string>& cmdList) {

	/* �ж�cmd */
	if (cmdList.size() == 0) {
		return;
	}

	string cmd = cmdList[0]; // ȡ������Ϊָ��

	if (cmd == "format") {
		cout << "��ʽ���������̿ռ䣬����������" << endl;
		globalOpenFileTable.Format();
		globalINodeTable.Format();
		globalBufferManager.Bformat();
		globalFileSystem.Format();
		exit(0);
	}
	else if (cmd == "autotest") {
		autotest();
	}
	else if (cmd == "creat") {
		// globalUser.Mkdir(cmdList[1]);
		// cout << cmdList[1] << " " << cmdList[2] << endl;
		globalUser.Create(cmdList[1], cmdList[2]);
	}
	else if (cmd == "open") {
		globalUser.Open(cmdList[1], cmdList[2]);
		f_handler[cmdList[1]] = globalUser.u_ar0[User::EAX];
	}
	else if (cmd == "read") {
		if (cmdList[2] == "-o") {
			globalUser.Read(to_string(f_handler[cmdList[1]]), cmdList[3], cmdList[4]);
		}
		else {
			globalUser.Read(to_string(f_handler[cmdList[1]]), "", cmdList[2]);
		}
	}
	else if (cmd == "write") {
		globalUser.Write(to_string(f_handler[cmdList[1]]), cmdList[2], cmdList[3]);
	}
	else if (cmd == "seek") {
		globalUser.Seek(to_string(f_handler[cmdList[1]]), cmdList[2], cmdList[3]);
	}
	else if (cmd == "close") {
		globalUser.Close(to_string(f_handler[cmdList[1]]));
	}
	else if (cmd == "mkdir") {
		globalUser.Mkdir(cmdList[1]);
	}
	else if (cmd == "rm") {
		globalUser.Delete(cmdList[1]);
	}
	else if (cmd == "ls") {
		globalUser.Ls();
	}
	else if (cmd == "cd") {
		globalUser.Cd(cmdList[1]);
	}
	else if (cmd == "exit") {
		exit(0);
	}
	else if (cmd == "") {
		return;
	}
	else {
		cout << "δʶ�𵽵ķ���: " << cmd << endl;
	}
}