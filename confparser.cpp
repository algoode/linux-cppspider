#include "confparser.h"
#include "stract.h"
#include "dbg.h"
#include <fstream>
ConfigParser* ConfigParser::self;
int ConfigParser::loder(const char* conf_filepath)
{
    ifstream fin("spider.conf");
    if(!fin)
    {
        log_err("open conf file fail!");
        return -1;
    }  
    string line;
    while(getline(fin,line))
    {
        if(line[0]!='#')//除去注释
        {
            trim(line);
            vector<string> temp;
            strsplit(line,"=",temp,1);
            if(temp.size()==2)
            {
                if(temp[0]=="max_job_num")
                job_num=stoi(temp[1]);
                else if(temp[0]=="savedir")
                savedir=temp[1];
                else if(temp[0]=="logfile")
                logfile=temp[1];
                else if(temp[0]=="log_level")
                log_level=stoi(temp[1]);
                else if(temp[0]=="max_depth")
                depth=stoi(temp[1]);
                else if(temp[0]=="module_path")
                module_path=temp[1];
                else if(temp[0]=="seeds")
                seed=temp[1];
                // else if(temp[0]=="accept_types")
                // self->accept_types=temp[1];       
            }
        }
    }

    
}
ConfigParser::ConfigParser()
{
    job_num=10;
    seed="";
    depth=3;
    log_level=0;
    module_path="./";
    savedir="download/";
    logfile="spider.log";
}
ConfigParser* ConfigParser::instance()
{
    if(self==NULL)
    self=new ConfigParser();
    return self;
}
void ConfigParser::release()
{
    delete self;
}

int ConfigParser::getJobNum()
{
    return job_num;
}

int ConfigParser::getDeep()
{
    return depth;
}

int ConfigParser::getLogLevel()
{
    return log_level;
}

string ConfigParser::getmodulePath()
{
    return module_path;
}

string ConfigParser::getUrlSeed()
{
    return seed;
}

string ConfigParser::getLog()
{
    return logfile;
}


string ConfigParser::getsaveDir()
{
    return savedir;
}
