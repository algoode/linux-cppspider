#include <string>
#include "dbg.h"
#include "url.h"
//#include <iostream>
using namespace std;
Url::Url(string url_str,int c_deep):url(url_str),deep(c_deep){}
//解析Url
int Url::parseUrl()
{
	while(url[url.length()-1]=='/')
		 url.pop_back();//去除最后几个/

	int h=url.find(':');
	if(h!=url.npos)
	{
		protocal=url.substr(0,h);//：前为协议
		h=h+3;	//jump '//'
	}
	else
	{
		h=0;
	}
	
	string temp=url.substr(h);
	h=temp.find('/');	//  /之前为域名
	string Filetype="html";

	if(h!=temp.npos)
	{//找到/，之前为域名，之后为路径
		Sitename=temp.substr(0,h);
		Path=temp.substr(h);
		int t=Path.rfind('.');
		Filetype=Path.substr(t+1);
	}
	else
	{
		Sitename=temp;
		Path="";
		Filetype="html";
	}
	ConvertUF(temp);
	Filename=temp+"."+Filetype;
	state=false;
	deep = 0;
    return 0;
}
string Url::getUrl() const
//获得URL
{
	return url;
}
string Url::getProtocal(){
	return protocal;
}
string Url::getSitename(){
	return Sitename;
}
string Url::getPath(){
	return Path;
}
string Url::getFilename(){
	return Filename;
}
bool Url::getState(){
	return state;
}
int Url::getDeep(){
	return deep;
}
void Url::setState(bool in){
	state=in;
}
void Url::ConvertUF(string &a){
	for(int i=0;i<a.length();++i)
		if(a[i]=='/')
			a[i]='_';
}
/*
int main(void){
	Url a=Url("http://epaper.gmw.cn/gmrb/html/2019-04/06/nw.D110000gmrb_20190406_1-08.htm");
	cout<<a.getUrl()<<endl;
	a.parseUrl();
	cout<<a.getProtocal()<<endl;
	cout<<a.getSitename()<<endl;
	cout<<a.getPath()<<endl;
	cout<<a.getFilename()<<endl;
	cout<<a.getDeep()<<endl;
	cout<<a.getFiletype()<<endl;
}*/
