/**
 * Stand alone data recorder  
 */
 
 
#include "RealTimeLogger.h"

using namespace std;


RealTimeLogger::RealTimeLogger(int bs, int cols, string p) 
{
   cellnum=1;
   recnum=1;
   path = p;
   bufsize = bs;
   
   // number of columns in the data set
   numcols = cols;
   
   // do not downsample by default
   downsample = 1;
   bufpos = 0;

   allocateBuffer(bufsize);
}

RealTimeLogger::~RealTimeLogger(void)
{
     delete(buffer);
}

void RealTimeLogger::allocateBuffer(int bs) {
    buffer = new double [bs];
    bufsize = bs;
    cerr << "Buffer created with " << bufsize << " entries" << endl;
}


void RealTimeLogger::newcell(int num)
{
  if (cellnum != num) {
    cellnum = num;
    recnum = 1;
  }
}


void RealTimeLogger::setRecNum(int rn) {
    recnum=rn;
}


void RealTimeLogger::deleteBuffer() {
     delete(buffer);
}


void RealTimeLogger::setBufferLen(int bs) {
     allocateBuffer(bs);
}



void RealTimeLogger::setPrint(int p) {
    print = p;
}


void RealTimeLogger::touchdata() {
    int fact = 100;
    for (int i = 0; i < bufsize; i+=fact) {
        buffer[bufpos]=0;
    }
}



void RealTimeLogger::insertdata(double newdata) 
{
    if (bufpos < bufsize) {
        buffer[bufpos] = newdata;
    } else {
        cerr << "WARNING: bufpos = " << bufpos << " when bufsize = " << bufsize << ".  Not inserting.  " << endl;
    }
    bufpos++;
}


void RealTimeLogger::resetbuffer()
{
    bufpos = 0;
}

int RealTimeLogger::getNumCols() {
    return numcols;
}

void RealTimeLogger::setDSRate(int ds) {
    downsample = ds;
}


void RealTimeLogger::writebuffer(string prefix, string fileinfo)
{

    create_fns(prefix);

    // open the file as binary
    ofstream data_file ((path + datafn).c_str(), ios::binary | ios::out);
    //ofstream log_file  (logfn.c_str(), ios::out);


    if (data_file.is_open()) {


        for (int i = 0; i <= bufsize-numcols; i+=downsample*numcols) {
            for (int j=0; j < numcols; j++) {
                if ((i+j) < bufsize) {
                    data_file.write(reinterpret_cast<char*>(&buffer[i+j]),sizeof(double));
                } else {
                    cerr << "Tilman was right" << endl;
                }
            }
        }
    } else {
        cout << "Unable to open file " << (path + datafn).c_str() << endl;
    }

    /*    if (data_file.is_open()) {

    // write buffer to file
    for (int i = 0; i < bufsize; i+=downsample) {
    data_file.write(reinterpret_cast<char*>(&buffer[i]),sizeof(double));
    }
    } else {
        cout << "Unable to open file " << (path + datafn).c_str() << endl;
    }*/

    data_file.close();
    if (print) cout << "File " << datafn << " written successfully." << endl;
    recnum++;

    update_log(fileinfo);
}


void RealTimeLogger::update_log(string fileinfo)
{
    ofstream log_file ((path + logfn).c_str(), ios::out | ios::app);
    if (log_file.is_open()) {
        log_file << timeofday<< " " << datafn << ": " << fileinfo << endl;
    }
    log_file.close();
    if (print) cout << "Log File " << logfn << " written successfully." << endl;

}


/**
 * Test function to output the vector
 */
void RealTimeLogger::dumpData() {

    for (int j = 0; j < bufsize; j++) {
        printf("element %d: %f\n", j, buffer[j]);
    }
}


void RealTimeLogger::create_fns(string prefix)
{
    char tstring[80];
    time_t sec;
    time (&sec);
    strftime(tstring, 80,"%b_%d_%y", localtime(&sec));

    // convert doubles to ints for printing, change cellnum to ANSI char
    char trial[80];
    sprintf(trial, "_%c%d.dat", ((int)cellnum)+64, (int)recnum);

    char t_o_d[80];
    strftime(t_o_d, 80, "%T", localtime(&sec));
    timeofday = string(t_o_d);

    datafn = prefix + "_" + string(tstring) + string(trial);
    logfn = string(tstring) + ".log";
}

