// Copyright 2013 Yangqing Jia
//
// This is a simple script that allows one to quickly test a network whose
// structure is specified by text format protocol buffers, and whose parameter
// are loaded from a pre-trained network.
// Usage:
//    test_net net_proto pretrained_net_proto iterations [CPU/GPU]

#include <cuda_runtime.h>

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

#include "caffe/caffe.hpp"

using namespace caffe;


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
    int i=0;
    string output,path;
    vector<string> filePathList;
 //open the list file
    std::ifstream listFile(fileName.c_str(), std::ios::in);

    if(listFile.is_open())
    {
    //for (fileid=0;fileid<200;fileid++)
        while(!listFile.eof())
        {
        //read one line at a time
        listFile>>output;

            if(listFile.eof()==1)break;
        filePathList.resize( filePathList.size() + 1 );
            filePathList[i]=output;
            i++;
        
        }
    }
    listFile.close();
    return filePathList;
}

vector<int> audioFileWindowSizeList(string fileName)
{
    int i=0,winSize;
    string output,path;
    vector<int> windowSizeList;
 //open the list file
    std::ifstream listFile(fileName.c_str(), std::ios::in);

    if(listFile.is_open())
    {
    //for (fileid=0;fileid<200;fileid++)
        while(!listFile.eof())
        {
        //read one line at a time
        listFile>>winSize;

            if(listFile.eof()==1)break;
        windowSizeList.resize( windowSizeList.size() + 1 );
            windowSizeList[i]=winSize;
            i++;
        
        }
    }
    listFile.close();
    return windowSizeList;
}

 void writeMFCC(std::vector< vector<float> > mfccCoeff, std::string fileName)
 {
    uint32_t nSamples,samplePeriod=10;
    uint16_t sampSize,parmKind=9;
    float data;
    string mfccFileName;
    char labelChar[2],path[200];
    
   //fileName="outFileTest";
            for(int j=0;j<fileName.size();j++)
            {
                if(j==2 | j==3)labelChar[j-2]=fileName[j];
                else if(j>4) path[j-5]=fileName[j];
            }
    //std::cout<<"path:"<<path<<"\n";
    path[fileName.size()-5]='\0';
    string pathStr(path);
    std::cout<<"pathStr:"<<pathStr<<"\n";

    mfccFileName=pathStr+".post";

    std::ofstream mfccFile(mfccFileName.c_str(), std::ios::out | std::ios::binary);
    //std::ofstream mfccFile("outFile", std::ios::out | std::ios::binary);


    nSamples=mfccCoeff.size()*100;
    sampSize=(mfccCoeff[0].size()/100)*4;  //here 100 is thebatch size

    if(mfccFile.is_open())
    {
    //read the header
      nSamples = swap_endian(nSamples);
      mfccFile.write((char*)(&nSamples), 4);

      samplePeriod =swap_endian(samplePeriod);
      mfccFile.write((char*)(&samplePeriod), 4);

      sampSize =swap_endian16(sampSize);
      mfccFile.write((char*)(&sampSize), 2); //MFCC vector length in BYTES (e.g. 13-d mfcc float -> sampSize=56)
 
      parmKind =swap_endian16(parmKind);
      mfccFile.write((char*)(&parmKind), 2);

    //go to the start of data
    //mfccFile.seekg(12);

   std::cout<<"mfcc Size#:"<<mfccCoeff.size();
   std::cout<<"mfcc line ize#:"<<mfccCoeff[0].size();

//iterate through all the lines
        for(int lineNum=0;lineNum<mfccCoeff.size();++lineNum)
        //for(int lineNum=0;lineNum<5;++lineNum)
        {
        //read the data for each line
        for(int i=0;i<mfccCoeff[0].size();i++)
        //for(int i=0;i<5;i++)
            {
        //convert to little endian
        //save it in a float array
        //read the data(HTK saves files in Big Endian)
        data=mfccCoeff[lineNum][i];
        data=reverseFloat(data);
        mfccFile.write((char*)(&data), 4);
        // pixels[i]=(uint8_t)round(data+100);
            }
        }
    }
    //}
    mfccFile.close();
}

int main(int argc, char** argv) {
  if (argc < 5) {
    LOG(ERROR) << "test_net net_proto pretrained_net_proto audioListFile windowSizeFile [CPU/GPU]";
    return 0;
  }

  cudaSetDevice(0);
  Caffe::set_phase(Caffe::TEST);

  if (argc == 5 && strcmp(argv[5], "GPU") == 0) {
    LOG(ERROR) << "Using GPU";
    Caffe::set_mode(Caffe::GPU);
  } else {
    LOG(ERROR) << "Using CPU";
    Caffe::set_mode(Caffe::CPU);
  }

  NetParameter test_net_param;
  ReadProtoFromTextFile(argv[1], &test_net_param);
  Net<float> caffe_test_net(test_net_param);
  NetParameter trained_net_param;
  ReadProtoFromBinaryFile(argv[2], &trained_net_param);
  caffe_test_net.CopyTrainedLayersFrom(trained_net_param);

  vector<Blob<float>*> dummy_blob_input_vec;

//  const vector<Blob<float>*>& result;

    vector<string> fileList;
    vector<int> windowSizeList;
   //Read the audio file list
    fileList=audioFileList(argv[3]);

   std::cout<<"File#:"<<fileList.size();
    windowSizeList=audioFileWindowSizeList(argv[4]);
  int size=0;

   for(int i=0; i<fileList.size();i++)
//   for(int i=0; i<5;i++)
{
   //For each file, get the name
   //read the number of windows==iter
    //create a vector of the coefficients


    vector < vector<float> > mfccCoeff;
    std::cout<<"windowSize:"<<windowSizeList[i]/10;

    for (int j = 0; j < (windowSizeList[i]/100); ++j) {
//  for (int j = 0; j < 5; ++j) {

    const vector<Blob<float>*>& result =
        caffe_test_net.Forward(dummy_blob_input_vec);
    size = result[1]->count();
    //size = 10;

   std::cout<<"size"<<size;

    vector<float> mfccLine(size);
    mfccCoeff.push_back(mfccLine);

       for(int k=0; k<size;k++){
       //for(int k=0; k<5;k++){
    //LOG(ERROR) << "Batch " << i << ", index: "<<j <<", "<< result[1]->cpu_data()[k];
 //   LOG(ERROR) << "Size " << size;
      mfccCoeff[j][k]=result[1]->cpu_data()[k];
                               }
  //  size = caffe_test_net.num_outputs();
  //  LOG(ERROR) << "Size " << size;
  }
   //write the header
   //write the coefficients
  writeMFCC(mfccCoeff,fileList[i]);
}

  return 0;
}
