#include "savehtml.h"
#include <fstream>
#include "dbg.h"
using namespace std; 
int SaveHtml::setFilePathName(const string &t)
{
    name=t;
    return 1;
}
int SaveHtml::saveData(const string &content)
{
    ofstream fout( name );  
    if (!fout)
    {
        log_err("\n %s:open fail",name.c_str());
    }
    else
    {
        fout <<content;
        fout.close();
    }
}