#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <list>
using namespace std;
class ConfigParser
{
public:
    //ConfigParser();
    int loder(const char* conf_filepath);
    // for single
    static ConfigParser* instance();
    static void release();

    //opratons
    int getJobNum();
    string getUrlSeed();
    int getDeep();
    int getLogLevel();
    string getmodulePath();
    string getsaveDir();
    string getLog();
    list<string> getModules();
    list<string> getFileTypes();
private: //for single
    ConfigParser();
	ConfigParser(const ConfigParser&);
	ConfigParser& operator=(const ConfigParser&);
    static ConfigParser* self;

private:
    int job_num;
    string seed;
    int depth;
    int log_level;
    string module_path;
    string savedir;
    string logfile;
    list<string> modules;
    list<string> file_type;
};
#endif
