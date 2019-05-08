#ifndef URL_H
#define URL_H
#include <string>

using namespace std;
#define MAX_LINK_LEN 128

#define TYPE_HTML  0
#define TYPE_IMAGE 1

class Url
{
	public:
	Url(string url_str,int deep);  //构造函数，
	int parseUrl();  //解析url，并填充内部URL结构
	string getUrl() const;  //获得URL
	string getProtocal();  
	string getSitename();  
	string getPath();  
	string getFilename();
	string getFiletype();
	bool getState();
	int getDeep();
	void setState(bool in);

	private:
	string url;
	string protocal;
	string Sitename;
	string Path;
	string Filename;
	bool state;
	int deep;
	void ConvertUF(string &a);
};

#endif
