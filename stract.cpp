#include "stract.h"
using namespace std;

void trim(string &a)
{
	if(!a.empty())
	{
		a.erase(0,a.find_first_not_of(" "));
		a.erase(a.find_last_not_of(" ") + 1);
	}
}

void  strsplit(const string& a,const string delim,vector<string>& v,int limit){

	if(limit==0)
		return ;
	string::size_type pos1, pos2;
	int len=delim.length();
	pos2 = a.find(delim);
	pos1 = 0;
	while(limit!=0&&string::npos != pos2)	//<0 -- all
	{
		--limit;
		v.push_back(a.substr(pos1, pos2-pos1));
		pos1 = pos2 + len;
		pos2 = a.find(delim, pos1);
	}
	if(pos1 != a.length())
		v.push_back(a.substr(pos1));

}	