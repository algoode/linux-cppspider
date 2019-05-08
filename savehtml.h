#include<string>
using namespace std;
class SaveHtml
{
public:
	int setFilePathName(const string &t);
	int saveData(const string &content);
private:
	//int save();     //后面保存为更多文件时，设置为纯虚函数
	string name;
};