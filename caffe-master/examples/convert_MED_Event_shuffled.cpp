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
#include <boost/random/mersenne_twister.hpp>
#include <boost/shared_ptr.hpp>

using boost::shared_ptr;
using namespace std;
typedef boost::mt19937::result_type rv_t; //for boost rand number gen

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

//rand unsigned key for leveldb (to avoid having many adjacent leveldb items that are the same category) 
uint64_t my_rand_key(boost::mt19937& gen){
    assert( sizeof( rv_t ) == 4 );
    uint64_t val = (uint64_t(gen()) << 32) + gen();
    return val;
}

bool checkIfKeyExists(const string& key, leveldb::DB* db)
{
    leveldb::Iterator* it =  db->NewIterator(leveldb::ReadOptions()); 
    it->Seek(key);
    return ( it->Valid() && (it->key() == (leveldb::Slice)key) ); //check if key exists in db already
}

//void test_checkIfKeyExists(leveldb::DB* db, shared_ptr<leveldb::Iterator> it)
void test_checkIfKeyExists(leveldb::DB* db)
{
    string value = "ohai";
    string key = "12345";
    db->Put(leveldb::WriteOptions(), key, value); 

    bool exist = checkIfKeyExists(key, db);
    if(exist == true)
        printf("    successfully identified a key that exists \n");
    else
        printf("    FAILED to find a key that exists \n");
}

int main(int argc, char** argv)
{
    boost::mt19937 gen;
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
    uint32_t labelInt;

    char labelC;
    char key[10];
    //char key[8]; //uint64_t
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

    //test_checkIfKeyExists(db);
    
    //open the list file
    std::ifstream trainFile(argv[1], std::ios::in);
    dbCount=0;
    if(trainFile.is_open()){
        while(!trainFile.eof())
        {
            //read one line at a time
            trainFile>>output;

            if(trainFile.eof()==1)break;
            //find out the position of = and [ 
            for(j=1;j<strlen(output);j++)
            {
                if(j<4) labelChar[j-1]=output[j];
                else if (j==4) {}
                else path[j-5]=output[j];
            } //end position of = and [ in a file name

            path[strlen(output)-5]='.';
            path[strlen(output)-4]='h';
            path[strlen(output)-3]='t';
            path[strlen(output)-2]='k';

            //convert the start and end points to int 
            label=atoi(labelChar);
            labelInt=label;

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
                mfccFile.seekg(12);

                numFloats=(nSamples)*(sampSize/4);
                //rows=(int)floor(nSamples/10);
                rows=30;
                cols=sampSize/4;
                numFloatsWindow=rows*(sampSize/4);
                dataArray=new float[numFloatsWindow];
                std::cout<<"numSamples"<<nSamples<<"\n";

                char* pixels=new char[numFloatsWindow];
                //initialize the data structure
                datum.set_channels(1);
                datum.set_height(rows);//time axis
                datum.set_width(sampSize/4); //mfcc axis
                numFrames=(uint32_t)floor((nSamples)/rows);

                //read the data and write to db for each frame
                for(frameid=0;frameid<numFrames;++frameid)
                {
                    //in a single frame
                    for(i=0;i<rows*(sampSize/4);i++){
                        //read the data(HTK saves files in Big Endian)
                        mfccFile.read((char*)(&data), 4);
                        //convert to little endian
                        data=reverseFloat(data);
                        //save it in a float array
                        pixels[i]=(char)round(data+100);
                    }

                    //write to the level db for each frame
                    datum.set_data(pixels, numFloatsWindow);
                    datum.set_label((char)labelInt);
                    datum.SerializeToString(&value);
                    //sprintf(key, "%08d", dbCount);
                    uint64_t rand_key = my_rand_key(gen);
                    //sprintf(key, "%08lu", rand_key);
                    //cout << "key = " << rand_key << endl;
                    string rand_key_str = string( (char*)&rand_key, (char*)&rand_key+8 );
                    //db->Put(leveldb::WriteOptions(), std::string(key), value);
                    while( checkIfKeyExists(rand_key_str, db) )
                    {
                        rand_key = my_rand_key(gen); //fresh key if current key is already in use
                        rand_key_str = string( (char*)&rand_key, (char*)&rand_key+8 ); 
                    }
                    db->Put(leveldb::WriteOptions(), rand_key_str, value); //TODO: check success/fail
                    ++dbCount;
                }
            }//end of mfcc file is open

            mfccFile.close();
        } //end of files read from trainList
    }  //end of trainFile is open
    delete db;
    trainFile.close();
} //end of main
