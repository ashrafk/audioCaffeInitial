/*
   Khalid Ashraf 2014
   Program to obtain the file path and MFCC start and 
   end location from the text file containing file names.

//next extension: do it for multiple files
 */
#include <google/protobuf/text_format.h>
#include <glog/logging.h>
#include <leveldb/db.h>
#include "caffe/proto/caffe.pb.h"

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <math.h>
using namespace std;

//class that defines the data structure returned by the 
class timeStamp{
    public:
        float startTime;
        float endTime;
        int phoneName;
        int numLines; 
};
//class mfccFeat{
//    vector<float> mfcc; 
//};
vector<string> audioFileList(string fileName);

uint32_t swap_endian( uint32_t val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | (val >> 16);
}
uint16_t swap_endian16( uint16_t val )
{
    return (val << 8) | (val >> 8);
}

float reverseFloat( const float inFloat)
{
        float retVal;
        char *ftoC=(char*) & inFloat;
        char *retF=(char*) & retVal;

        retF[0]=ftoC[3];
        retF[1]=ftoC[2];
        retF[2]=ftoC[1];
        retF[3]=ftoC[0];
    return retVal;
}




vector<string> audioFileList(string fileName)
{
       // printf("test3") ;
    int i=0;
    string output;
    vector<string> filePathList;
 //open the list file
    //std::ifstream listFile("trainList", std::ios::in);
    std::ifstream listFile(fileName.c_str(), std::ios::in);

    if(listFile.is_open())
    {
    //for (fileid=0;fileid<200;fileid++)
        while(!listFile.eof())
        {
        //read one line at a time
        listFile>>output;

        //printf("test3") ;

            if(listFile.eof()==1)break;
        filePathList.resize( filePathList.size() + 1 );
            output.resize(output.size()-3);
            filePathList[i]=output;
            i++;
        
        }
    }
    listFile.close();
    return filePathList;
}


/*
usage: ./executable fileList leveldbName
*/

int main (int argc, char** argv)
{
    int i=0;
    vector<string> filePathList;
     
    int windowSize=23;
/*    char* dbName=new char[200];
    dbName=argv[2];
    char* listFile=new char[200];
    listFile=argv[1];
*/
    vector<string> fileList;
    string output;
        printf("test1\n") ;
    //writeToLeveldb(argv[2],argv[1],windowSize);
    fileList=audioFileList("testList");
/*
    std::ifstream listFile("testList", std::ios::in);
    if(listFile.is_open())
    {
    for (int fileid=0;fileid<10;fileid++)
    //    while(!listFile.eof())
        {
            listFile>>output;
       cout<<output<<"\n";


            if(listFile.eof()==1)break;
            filePathList.resize( filePathList.size() + 1 );
            output.resize(output.size()-3);
       cout<<output<<"\n";
            filePathList[i]=output;
            i++;


        }
    }
    listFile.close();
*/

return 0;
}


