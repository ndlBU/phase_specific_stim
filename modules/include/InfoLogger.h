#ifndef INFOLOGGER_H
#define INFOLOGGER_H


#include <vector>
#include <string>
#include <iostream>
#include <fstream>


using namespace std;


class InfoLogger 
{ 
public:
    InfoLogger(void);
    virtual ~InfoLogger(void);

    virtual void insertdata(string);
    virtual void writebuffer(string, string, int);
    virtual void resetbuffer();
    virtual void setRecNum(int);
    virtual void newcell(int);
    void setDSRate(int ds);

    void dumpData();

private:

    void create_fns(string);
    string create_path(void);
	
    vector< string > buffer; 

    int cellnum;
    int recnum;

    string datafn;
    string logfn;
    string timeofday; 

    string path;
    
    int downsample;
};




#endif 
