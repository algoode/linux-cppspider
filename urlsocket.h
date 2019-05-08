#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <regex.h>
#include <vector>
#include <list>

#include "url.h"

extern "C" {
#include "wrap.h"
}
using namespace std;

class UrlSocket
{
public: //公共接口
	UrlSocket(Url* url); 
	~UrlSocket();
	int connect(const string,int port = 80);
	int request();  //发送请求
	int response();//接受反馈
	int disconnect();//断开链接操作
	int getData();//get all html and parsehtml
	int parsehtml(string &str);
	int getfd() const;
	Url* getUrl() const;
	const string& getHeadType ()const;
	const string& getBody () const;
	int getStatus() const;
	void extract_url(list<string>& res);
private:  //内部接口
	int setSocketNoB(); //设置socket	int connect();  //链接操作
	char* attach_domain(char *url);//加上域名

private:
	int m_sockfd;  //socket句柄
	char* buf;
	string head_type;
	string body;
	int status;
	Url* m_url;
	static const char* HREF_PATTERN;
};

#endif
