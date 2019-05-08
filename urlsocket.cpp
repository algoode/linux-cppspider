
#include "urlsocket.h"
#include "stract.h"
#include "dbg.h"
#include <iostream>
using namespace std;
#define HTML_MAXLEN   100*1024


UrlSocket::UrlSocket(Url* url):m_url(url)
{
}
UrlSocket::~UrlSocket()
{
 close(m_sockfd);
}
const char* UrlSocket::HREF_PATTERN = "href=\"\\s*\\([^ >\"]*\\)\\s*\"";//定义正则匹配规则
//  \\ 转义成\ '\s'表示空格,故\\s为空格
//本为 href="\s*([^ >\"]*)\s*",小括号内为子正则表达式,这里转义字符要用两个

//发送请求
int UrlSocket::connect(const string ip,int port)
{
    //socket set,connect,request
    struct sockaddr_in servaddr;
    m_sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    setSocketNoB();
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    log_info("%s:ip done!",m_url->getUrl().c_str());
    if(Connect(m_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))==-1)
    {
   //     errno=0;
        log_err("\nsocket connect fail!");
        return -1;
    }
    return 1;
}

//请求
int UrlSocket::request()
{
	char re[1024]={0};
	sprintf(re, "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: keep-alive\r\n"
            "Cache-Control: max-age=0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36\r\n"
            "Accept-Language: zh-CN,zh;q=0.8\r\n\r\n",m_url->getPath().c_str(), m_url->getSitename().c_str());
    
	int n=Writen(m_sockfd,re,strlen(re));
    if(n==-1)
    {
        log_err("\n%s:request fail!",m_url->getUrl().c_str());
        return -1;
    }
    log_info("%s:request done!",m_url->getUrl().c_str());
    return n;
}
//接受反馈
int UrlSocket::response()
{
	buf = (char *)malloc(HTML_MAXLEN);
	int n=Read(m_sockfd,buf,HTML_MAXLEN);
	return n;
}
//设置UrlSocket
int UrlSocket::setSocketNoB()
{
    int flag;
    if ((flag = fcntl(m_sockfd, F_GETFL, 0)) < 0) {
        log_err("\n\n%s :fcntl getfl fail",m_url->getUrl().c_str());
    }
    flag |= O_NONBLOCK;
    if ((flag = fcntl(m_sockfd, F_SETFL, flag)) < 0) {
       
        log_err("\n\n%s :fcntl setfl fail",m_url->getUrl().c_str());
    }
    return 1;
}


int UrlSocket::getfd() const
{
    return m_sockfd;
}
//得到返回的数据，并解析成head和body
int UrlSocket::getData()
{
    log_info("%s:begin accept response!",m_url->getUrl().c_str());
    int n;
    string res="";
    int length=0;
    do
    {
    n=response();
    res+=string(buf);
    free(buf); 
    buf=NULL;
    length+=n;
    }
    while(n!=0 && n>0);
    log_info("%s:get response done!\n",m_url->getUrl().c_str());
    if(parsehtml(res)==-1)
    {
        log_err("\n%s:not get html success!",m_url->getUrl().c_str());
        return -1;
    }
    log_info("%s:get head status and body done!",m_url->getUrl().c_str());
    return length;
}
//解析成head和body

int UrlSocket::parsehtml(string &str)
{
    if(str=="")
    return -1;
    string::size_type pos;
    pos=str.find("\r\n\r\n");
    // get status and type
    string head=str.substr(0,pos);
    vector <string> headRow;
    strsplit(head,"\r\n",headRow,-1);
    vector <string> temp;
    strsplit(headRow[0]," ",temp,3);
    status = stoi(temp[1]);
    if(status!=200)
    {
        log_err("\n%s:get data fail",m_url->getUrl().c_str());
        return -1;
    }
    else
    {
        for(int i=1;i<headRow.size();++i)
        {
            temp.clear();
            strsplit(headRow[i],":",temp,1);
            if(temp.size()==2)
            {
                trim(temp[0]);
                if(strcasecmp(temp[0].c_str(),"Content-Type")== 0)
                {
                    trim(temp[1]);
                    head_type=temp[1];
                }
            }
        }
    }
    //get body
    body=str.substr(pos+4);
    return 1;
}
const string& UrlSocket::getBody() const
{
    return body;
}
const string& UrlSocket::getHeadType() const
{
    return head_type;
}
int UrlSocket::getStatus() const
{
    return status;
}
Url* UrlSocket::getUrl() const
{
    return m_url;
}
//提取url，res为传入传出参数
void UrlSocket::extract_url(list<string>& res)
{
    const char* p=body.c_str();
    regex_t re;

    if (regcomp(&re, HREF_PATTERN, 0) != 0) {/* compile error */
       log_err( "%s:compile regex error",m_url->getUrl().c_str());
    }
    const size_t nmatch = 2;
    regmatch_t matchptr[nmatch];
    log_info("%s:begin regex",m_url->getUrl().c_str());
    while(regexec(&re,p,nmatch, matchptr, 0)!=REG_NOMATCH)
    {
      int len=(matchptr[1].rm_eo - matchptr[1].rm_so);
      p = p + matchptr[1].rm_so;
      char *tmp = (char *)calloc(len+1, 1);
      strncpy(tmp, p, len);
      tmp[len] = '\0';
      char* t=attach_domain(tmp);//接受返回的正常的链接
      if(t!=NULL)
      {
          log_info("%s:found a url : %s",m_url->getUrl().c_str(),t);
          res.push_back(t);
      }
      free(t);
      p = p + len + (matchptr[0].rm_eo - matchptr[1].rm_eo);//更新下要匹配的字符串，可能后面还有其他空格之类的
    }

    regfree(&re);
}

//加上域名
char* UrlSocket::attach_domain(char *url)
{
    if (url == NULL)
        return NULL;

    if (strncmp(url, "http", 4) == 0) {//以http开头，表示已经有完整的域名了
        return url;
		
    } else if(strncmp(url, "www", 3) == 0)
    {
        return url;
    }
    else if(strncmp(url, "//www", 5) == 0)
    {
        int j=0;
        int len = strlen(url);
        char *tmp1 = (char *)malloc(len+1);
        for(j=2;j < len;j++)
        {
            tmp1[j-2] = url[j];
        }
        free(url);
        //log_err("\n new url：%s",tmp1);
        return tmp1;
    }
	else if (*url == '/') {
		//不是完整的url，补充域名，这里可能会出现一些错误的情况，可能多一个'/'，或者原来是以www开头
        int i;
        string pp=m_url->getSitename();
        const char* sitename=pp.c_str();
        log_warn("%s",sitename);
        int ulen = strlen(url);
        int dlen = strlen(sitename);
        char *tmp = (char *)malloc(ulen+dlen+1);
        for (i = 0; i < dlen; i++)
            tmp[i] = sitename[i];
        for (i = 0; i < ulen; i++)
            tmp[i+dlen] = url[i];
        tmp[ulen+dlen] = '\0';
        free(url);
        return tmp;

    } else {
        //do nothing
        free(url);
        return NULL;
    }
}

/*int main(int argc, char *argv[])
{
	UrlSocket a= UrlSocket("127.0.0.1",8000);
    	a.request();
	a.getData();
    return 0;
}
*/

