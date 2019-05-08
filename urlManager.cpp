#include "urlManager.h"

UrlManager::UrlManager(){}
int UrlManager::addUrl(string urlstr,int deep)
{
    
    if(findUrl(urlstr)==-1)
    {
        Url* temp=new Url(urlstr,deep);
        UrlMap[urlstr]=temp;
        m_urlqueue.push(temp);
        return 1;
    }
    else 
    return -1;
}

int UrlManager::addUrlList(list<string> urlStrs,int deep)
{
    int n=0;
    while(!urlStrs.empty())
    {
        string temp=urlStrs.front();
        urlStrs.pop_front();
        if(addUrl(temp,deep+1)==1)
        ++n;
    }
    return n;
}

Url* UrlManager::getUrlForQuque ()   //取一个未处理的URL对象指针
{
    Url* temp=m_urlqueue.front();
    m_urlqueue.pop();
    return temp;
}

int UrlManager::setUrlState(Url* url) //设置URL处理状态
{
    url->setState(true);
    return 1;
}
int UrlManager::getNums () const
{
    return UrlMap.size();
}
int UrlManager::getQueueNums() const
{
    return m_urlqueue.size();
}
int UrlManager::findUrl(string urlStr)
{
    if(UrlMap.count(urlStr)==1)
    return 1;
    else 
    return -1;
}