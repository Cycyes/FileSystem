#ifndef COMMAND_H
#define COMMAND_H

#include <vector>
#include <map>
#include <string>

using namespace std;

class Command {
	/*=== Functions ===*/
private:

	void autotest();

public:
	vector<string> getCommand(const string& str); // ���������������
	void executeCommand(const vector<string>& cmdList); // ���ж�Ӧ������

	/*=== Members ===*/
private:

	map<string, int> f_handler; // �ļ�������ӳ��

};
 
#endif