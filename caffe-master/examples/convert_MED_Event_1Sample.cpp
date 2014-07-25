/*
Khalid Ashraf 2014
Program to obtain the file path and MFCC start and 
end location from the text file containing file names.

arg1: List file
arg2: leveldb file

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
#include <math.h>
#include <stdio.h>

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


int main(int argc, char** argv)

{
int i,j,rows,cols,fileid,label;

uint32_t length,numFrames,numFloatsWindow;
char output[200],path[200],labelChar[3];
  uint32_t nSamples;
  uint32_t samplePeriod;
  uint16_t sampSize;
  uint32_t numFloats,frameid;
  uint16_t parmKind;
  uint32_t dbCount;
  float data;
  char* sampS=new char[4];
  float* dataArray;
  uint32_t labelInt,start,end;

  char labelC;
  char key[10];
  std::string value;
  caffe::Datum datum;
  char* db_filename=new char[200];
       //db_filename="aladdinWin-test-leveldb";
       db_filename=argv[2];
  // Open leveldb
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  options.error_if_exists = true;
  leveldb::Status status = leveldb::DB::Open(
      options, db_filename, &db);
  CHECK(status.ok()) << "Failed to open leveldb " << db_filename
      << ". Is it already existing?";

	//open the list file
std::ifstream trainFile(argv[1], std::ios::in);
dbCount=0;
if(trainFile.is_open()){
//for (fileid=0;fileid<200;fileid++)
while(!trainFile.eof())
{
//read one line at a time
trainFile>>output;
//std::cout<<output;
//find out the position of = and [ 
for(j=1;j<strlen(output);j++)
{
if(j<4) labelChar[j-1]=output[j];
else if (j==4) {}
else path[j-5]=output[j];
} //end position of = and [ in a file name

//path[strlen(output)-5]='\0';
path[strlen(output)-5]='.';
path[strlen(output)-4]='h';
path[strlen(output)-3]='t';
path[strlen(output)-2]='k';

//convert the start and end points to int 
label=atoi(labelChar);
//labelInt=(uint8_t)label;
labelInt=label;
//start=atoi(first);
//end=atoi(last);

std::cout<<"\n";
//equal=1;left=2;
length=strlen(output);
{std::cout<<"path="<<path<<std::endl;}
{std::cout<<"label:"<<label<<std::endl;}
{std::cout<<"labelChar:"<<labelInt<<std::endl;}

//Given-> mfcc file name, start line, end line
//Insert entry into the dB 
	//close the list file
	//open the file
std::ifstream mfccFile(path, std::ios::in | std::ios::binary);

if(mfccFile.is_open()){
//read the header
  mfccFile.read((char*)(&nSamples), 4);
  nSamples = swap_endian(nSamples);
  mfccFile.read((char*)(&samplePeriod), 4);
  samplePeriod =swap_endian(samplePeriod);
  mfccFile.read((char*)(&sampSize), 2);
  sampSize =swap_endian16(sampSize);
  mfccFile.read((char*)(&parmKind), 2);
  parmKind =swap_endian16(parmKind);

//go to the start location
start=(uint32_t)(floor)(nSamples/4);
mfccFile.seekg(12+start);
//mfccFile.read((char*)(&data), 4);
//data=reverseFloat(data);
//std::cout<<"Test First Float"<<data<<"\n";

numFloats=(nSamples)*(sampSize/4);
//rows=(int)floor(nSamples/10);
rows=1000;
cols=sampSize/4;
numFloatsWindow=rows*(sampSize/4);
 dataArray=new float[numFloatsWindow];
std::cout<<"numSamples"<<nSamples<<"\n";

  char* pixels=new char[numFloatsWindow];
//initialize the data structure
  datum.set_channels(1);
  datum.set_height(rows);//time axis
  datum.set_width(sampSize/4); //mfcc axis
//  numFrames=(uint32_t)floor((nSamples)/rows);

//read the data and write to db for each frame
//for(frameid=0;frameid<numFrames;++frameid)
{
//in a single frame
for(i=0;i<rows*(sampSize/4);i++){
  //read the data(HTK saves files in Big Endian)
  mfccFile.read((char*)(&data), 4);
  //convert to little endian
  data=reverseFloat(data);
  //save it in a float array
  pixels[i]=(uint8_t)round(data+100);
}


//write to the level db for each frame
    datum.set_data(pixels, numFloatsWindow);
    datum.set_label((char)labelInt);
    datum.SerializeToString(&value);
    sprintf(key, "%08d", dbCount);
    db->Put(leveldb::WriteOptions(), std::string(key), value);
  ++dbCount;
}
}//end of mfcc file is open

mfccFile.close();
} //end of files read from trainList
}  //end of trainFile is open
  delete db;
trainFile.close();
} //end of main
