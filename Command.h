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
	vector<string> getCommand(const string& str); // 处理输入的命令行
	void executeCommand(const vector<string>& cmdList); // 进行对应的运算

	/*=== Members ===*/
private:

	map<string, int> f_handler; // 文件名与句柄映射

};
 
#endif