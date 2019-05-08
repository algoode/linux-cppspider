#include "dbg.h"
#include "DNSManager.h"
#include "urlsocket.h"
#include "savehtml.h"
#include "urlManager.h"
#include "confparser.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <sys/epoll.h>
#include <pthread.h>

using namespace std;
void * phdo(void * arg);
int attach_epoll_task();
int create_thread(void *(*start_routine) (void *), void *arg, pthread_t * thread, pthread_attr_t * pAttr);
void begin_thread();
void end_thread();
int g_epfd;
UrlManager u;
DNSManager dns;
SaveHtml save;
ConfigParser* con;
/* 当前线程数 */
int g_cur_thread_num = 0;

/* 给当前线程数g_cur_thread_num的值上锁 */
pthread_mutex_t gctn_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dns_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t que_lock = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char *str[])
{
  struct epoll_event events[10];
  //读取配置文件
  con=ConfigParser::instance();
  con->loder("spider.conf");
  const string dir=con->getsaveDir();
  if(access(dir.c_str(),F_OK)==-1)//已存在
  if(mkdir(dir.c_str(),0777)==-1)
  log_err("mkdir fail!");
  //得到爬取种子（URL）
  u=UrlManager();

  cout<<con->getUrlSeed()<<endl;;
  if(u.addUrl(con->getUrlSeed(),1)==-1)
  log_err("add seed fail!");
  int onum=0;
  g_epfd = epoll_create(con->getJobNum());
  while(onum++<con->getJobNum())//将现有的seed加入到epoll中
  {
    if(attach_epoll_task()<0)
    break;
  }
  /* epoll wait 等待抓取任务*/
    int n, i;
    while(1) {
        n = epoll_wait(g_epfd, events, 10, 2000);//epoll将会把发生的事件赋值到events数组中,超时时间单位毫秒
        printf("anwser epoll:%d   cur_thread:%d  rest Queuenums:%d\n",n,g_cur_thread_num,u.getQueueNums());//打印活跃事件数
		//容错
        if (n == -1)
            printf("epoll errno:%s\n",strerror(errno));
        fflush(stdout);
		//取出一个url，然后处理
        if (n <= 0) {
            if (g_cur_thread_num <= 0 && u.getQueueNums()==0) {
                sleep(1);
                if (g_cur_thread_num <= 0 && u.getQueueNums()==0)
                    break;//跳出条件
            }
        }
		//event和epoll结合
        for (i = 0; i < n; i++) {//n为活跃的事件数
            UrlSocket* sock = (UrlSocket *)(events[i].data.ptr);
			//发生错误，挂起，不是epollin事件
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                log_warn("epoll fail, close socket %d",sock->getfd());
                close(sock->getfd());
                continue;
            }
            //没错，则
            epoll_ctl(g_epfd, EPOLL_CTL_DEL, sock->getfd(), &events[i]); /* del event */

            fflush(stdout);
			//清除读写缓冲区，需要立即把输出缓冲区的数据进行物理写入时
            //create_thread是自己封装的，在线程的头文件中，创建线程处理
            //recv_response为回调函数
            //产生epollin事件就调用接收函数接收，产生一个线程处理任务
            create_thread(phdo, sock, NULL, NULL);
        }
    }
 
    log_info("Task done!");//结束
    close(g_epfd);
    con->release();
    pthread_mutex_destroy(&gctn_lock);
    pthread_mutex_destroy(&dns_lock);
    pthread_mutex_destroy(&que_lock);
  
}
int n=0;
int attach_epoll_task()
{
    
    struct epoll_event ev;
    if(u.getQueueNums()==0)//判断是否没有url
    {
      log_warn("Queue is empty!");
      return -1;
    }
    pthread_mutex_lock(&que_lock);
    Url* urltemp=u.getUrlForQuque();
    log_info("epoll:add epoll done :%s,all epoll num:%d",urltemp->getUrl().c_str(),++n);
    pthread_mutex_unlock(&que_lock);

    UrlSocket* sock=new UrlSocket(urltemp);
    log_info("%s:crawl begin",urltemp->getUrl().c_str());
    urltemp->parseUrl();
    pthread_mutex_lock(&dns_lock);
    string ip = dns.getIP(urltemp->getSitename());
    pthread_mutex_unlock(&dns_lock);
    /* connect socket and get sockfd */
    if(sock->connect(ip)==-1)
    {
      log_err("\n%s:connect fail",urltemp->getUrl().c_str());
      delete sock;
      return -1;
    }
    if(sock->request()==-1)
    {
      log_err("\n%s:request fail",urltemp->getUrl().c_str());
      delete sock;
      return -1;
      
    }
    
    ev.data.ptr = sock;
    ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(g_epfd, EPOLL_CTL_ADD, sock->getfd(), &ev) == 0) {/* add event */
        log_info("Attach an epoll event success!\n url:%s",sock->getUrl()->getUrl().c_str());
    } else {
        log_err("Attach an epoll event fail!");
        return -1;
    }
    g_cur_thread_num++; 	//当前正在执行抓取的任务数量
    return 0;
}


int create_thread(void *(*start_func)(void *), void * arg, pthread_t *pid, pthread_attr_t * pattr)
{
    pthread_attr_t attr;
    pthread_t pt;

    if (pattr == NULL) {
        pattr = &attr;
        pthread_attr_init(pattr);
        pthread_attr_setstacksize(pattr, 1024*1024);//设置线程堆栈的大小
        pthread_attr_setdetachstate(pattr, PTHREAD_CREATE_DETACHED);//设置线程分离
    }

    if (pid == NULL)
        pid = &pt;

    int rv = pthread_create(pid, pattr, start_func, arg);//创建线程
    pthread_attr_destroy(pattr);
    return rv;
}

void begin_thread()
{
    log_info("Begin Thread %lu", pthread_self());//获得线程自身的ID
}

//结束一个任务，只是结束任务， 因为设置了线程分离，所以回调函数执行结束后便会结束该线程
//但是如果任务数允许，则会创建新的任务
void end_thread()
{
    pthread_mutex_lock(&gctn_lock);	
    int left = ConfigParser::instance()->getJobNum() - (--g_cur_thread_num);//刷新剩下的任务数
    log_info("rest tasknums:%d",left);
    if (left == 1) {
        /* can start one thread */
		//创建一些新的任务，主函数那里只是处理原来的那些url
        attach_epoll_task();//该函数执行了发送请求，注册epoll事件等操作
    } else if (left > 1) {
        /* can start two thread */
        attach_epoll_task();
        attach_epoll_task();
    } else {
		//要先等待其他的事件退出，才能开启新的任务
        /* have reached g_conf->max_job_num , do nothing */
    }
	//打印当前线程的id以及剩下的任务数
    log_info("End Thread %lu, cur_thread_num=%d", pthread_self(), g_cur_thread_num);
    pthread_mutex_unlock(&gctn_lock);	
}

void * phdo(void * arg)
{
    begin_thread();//这个函数只是打印线程自身的id
    UrlSocket* sock=(UrlSocket*)arg;
    log_info("%s:begin get and extract data",sock->getUrl()->getUrl().c_str());
    int ans=sock->getData();
    if(ans<0)
    {//get fail
      log_err("\n%s:fail to get data!",sock->getUrl()->getUrl().c_str());
      delete sock;  //获得失败后，删除且补充
      end_thread();
      return NULL;
    }
    list<string> moreurl;
    log_info("%s deep:%d max deep:%d",sock->getUrl()->getUrl().c_str(),sock->getUrl()->getDeep(),ConfigParser::instance()->getDeep());
    if(sock->getUrl()->getDeep()<ConfigParser::instance()->getDeep())//达到最大深度则不抽取url
    {
      sock->extract_url(moreurl);
      pthread_mutex_lock(&que_lock);
      u.addUrlList(moreurl,sock->getUrl()->getDeep());
      pthread_mutex_unlock(&que_lock);
    }
    save.setFilePathName(con->getsaveDir()+"/"+sock->getUrl()->getFilename());
    save.saveData(sock->getBody());
    delete sock;
    end_thread();
    return NULL;
}