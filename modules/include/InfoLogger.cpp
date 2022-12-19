/**
 * Logs information about each run including the net file and a comment.
 */
 
 
#include "InfoLogger.h"

InfoLogger::InfoLogger(void) 
{
   cellnum=1;
   recnum=1;
   path = "/usr/src/rtxi/DATA/";
   
   // do not downsample by default
   downsample = 1;
}


InfoLogger::~InfoLogger(void)
{
//   buffer.~vector< vector<double> >();
}


void InfoLogger::newcell(int num)
{
  if (cellnum != num) {
    cellnum = num;
    recnum = 1;
  }
}
void InfoLogger::setRecNum(int rn) {
    recnum=rn;
}

void InfoLogger::insertdata(string newdata) 
{
   buffer.push_back(newdata);
}


void InfoLogger::resetbuffer()
{
   buffer.clear();
}

void InfoLogger::setDSRate(int ds) {
    downsample = ds;
}


void InfoLogger::writebuffer(string prefix, string comment, int nc)
{

    create_fns(prefix);

    // open the info file as a text file
    ofstream info_file ((path + datafn).c_str(), ios::out);

    if (info_file.is_open()) {
        
        // write number of columns to file
        info_file << "numcols:" << nc << endl << endl;
        
        // write comment string to file first
        info_file << comment << endl << endl;

        // output net file
        for (int i = 0; i < (int)buffer.size(); i++) {
            // output a data point
            //data_file.write(reinterpret_cast<char*>(&buffer[i][j]),sizeof(double));
            //cout << "i: " << i << " j: " << j << endl;
            // we are only writing string so just use C++ file I/O
            info_file << buffer[i] << endl;
        }
        recnum++;
    } else {
        cout << "Unable to open file " << datafn << endl;
    }

    info_file.close();
    cout << "File " << datafn << " written successfully." << endl;
}


/**
 * Test function to output the vector
 */
void InfoLogger::dumpData() {
    
    for (int i = 0; i < (int)buffer.size(); i++) {
        cout << "element " << i << ": " << buffer[i] << endl;
    }
}



    
void InfoLogger::create_fns(string prefix)
{
    char tstring[80];
    time_t sec;
    time (&sec);
    strftime(tstring, 80,"%b_%d_%y", localtime(&sec));

    // convert doubles to ints for printing, change cellnum to ANSI char
    char trial[80];
    sprintf(trial, "_%c%d.info", ((int)cellnum)+64, (int)recnum);
    
    char t_o_d[80];
    strftime(t_o_d, 80, "%T", localtime(&sec));
    timeofday = string(t_o_d);
    
    datafn = prefix + "_" + string(tstring) + string(trial);
    logfn = string(tstring) + ".log";
}

