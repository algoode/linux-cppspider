#ifndef URLM_H
#define URLM_H
#include<list>
#include<map>
#include<queue>
#include<string>
#include"url.h"
using namespace std;

class UrlManager
{
public: //外部接口
	int addUrl(string urlstr,int deep); //添加一个全新的URL 加入到list and quque中，同时生存索引
	int addUrlList(list<string> urlStrs,int deep); //批量添加
	Url* getUrlForQuque () ;  //取一个未处理的URL对象指针
	int setUrlState(Url* url); //设置URL处理状态
	UrlManager();
	int getNums () const;
	int getQueueNums() const;
	//setUrlValue();

private: //内部接口
	int findUrl(string urlStr);
		
private:
	map<string, Url*> UrlMap;  //所有URL的索引
	queue<Url*> m_urlqueue;   //维护等待抓取的URL
};
//

#endif
