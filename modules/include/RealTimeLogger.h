#ifndef REALTIMELOGGER_H
#define REALTIMELOGGER_H


#include <vector>
#include <string>
#include <iostream>
#include <fstream>


using namespace std;


class RealTimeLogger 
{ 
public:
    RealTimeLogger(int, int, string);
    virtual ~RealTimeLogger(void);

    virtual void touchdata();
    virtual void insertdata(double);
    virtual void writebuffer(string, string);
    virtual void newcell(int);
    virtual void resetbuffer();
    virtual void setRecNum(int);
    virtual void deleteBuffer();
    virtual void setBufferLen(int);

    
    virtual int getNumCols();
    virtual void setDSRate(int ds);

    virtual void dumpData();
    virtual void setPrint(int);
    virtual void allocateBuffer(int);

private:

    void create_fns(string);
    void update_log(string);
    string create_path(void);
	
    double *buffer;
    int bufpos;

    int cellnum;
    int recnum;
    string datafn;
    string logfn;
    string timeofday; 

    string path;
    int bufsize;
    
    int downsample;
    int numcols;
    int print;
};




#endif 
