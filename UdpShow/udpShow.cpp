#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "udpSocket.h"
#include <unistd.h>
#include <string>

using namespace cv;
using namespace std;
using namespace NUdpSocket;

const TUDWord to(100000);

int main(int argc, char **argv) {
	bool die(false);
	printf("Hello im udpShow \n");
	Mat depthf(Size(640,480),CV_8UC1);
	Mat RGB(Size(640,480),CV_8UC3);
	Mat jpgRead(Size(640,480),CV_8UC1);
	//FILE* rcvdData;
	std::string winName("depth");
//	std::string winName("RGB");
	std::string jpgLoad("jpg");
	namedWindow(winName,CV_WINDOW_AUTOSIZE);
	//namedWindow(jpgLoad,CV_WINDOW_AUTOSIZE);

	SSocketConfig conf("10.0.0.1","10.0.0.2",50555,50555,
			KINECT_FRAME_SIZE,"IR IMAGE SOCKET");
	CUdpSocket udpSocket;
	bool result = true;
	result = udpSocket.configureSocket(conf);
	//char fileName[] = {"birds.jpg"};
	//FILE* file(fopen(fileName, "rb"));
	//result = udpSocket.initFromFile(fileName,"Kinect1");
	if(!result){
		return -1;
	}
	result = udpSocket.openSocket();
	if (!result){
		return -1;
	}
		TUDWord flushSize = 60000;
		TUByte* flush = new TUByte[flushSize];
		TUDWord count(0);
	while(udpSocket.reciveData(flush,flushSize,10000) && count < 300){ count++; }
	uint bufferSize = KINECT_FRAME_SIZE*3;
//	uint bufferSize = KINECT_FRAME_SIZE;

	TUByte* buff = new TUByte[bufferSize];
	TUByte* buffEnd = buff + bufferSize;
	long alreadyReceived = 0;
	while (!die) {

		TUDWord size = KINECT_FRAME_SIZE;
//		size = KINECT_FRAME_SIZE;
		for(TUByte* buffIterator = buff;buffIterator < buffEnd ;buffIterator+=size){
			alreadyReceived = buffIterator - buff;
			//cout<<endl<<"Buffiterator is : "<<static_cast<void*>(buff)<<endl;
			//cout<<endl<<"Received already: "<<(alreadyReceived)<<endl;
			udpSocket.reciveData(buffIterator,size);
		}
//		TUDWord lastReceiveSize = KINECT_FRAME_SIZE - alreadyReceived + 100;
//		cout<<endl<<"Received before last "<<(lastReceiveSize)<<endl;
//		udpSocket.reciveData(buff,lastReceiveSize);
//		alreadyReceived+=lastReceiveSize;
//		cout<<endl<<"Received already: "<<(alreadyReceived)<<endl;
		//cout<<endl<<"I'm done"<<endl;



		depthf.data = (buff);
//		RGB.data = buff;

		//rcvdData = fopen("/home/alexandalex/kinect/UdpShow/rcvdData.bin","wb");
		//perror("err");
		//int stat = fwrite(buff,sizeof(char),KINECT_FRAME_SIZE,rcvdData);
		//perror("err");
		//fclose(rcvdData);
		//perror("err");
		//cv::imwrite("rcvdGray.jpg",depthf);
		//jpgRead = cv::imread("birds.jpg",CV_LOAD_IMAGE_GRAYSCALE);
		//Mat test;
		//test = cv::imread("birds.jpg",CV_LOAD_IMAGE_GRAYSCALE);

//		cv::imshow(winName,RGB);
		cv::imshow(winName,depthf);

		cv::waitKey(5);
		//if (!test.empty())
		//{
			//cv::imshow(jpgLoad,jpgRead);
		//	cout << "Successfully loaded the file" << endl;
		//	cv::imshow(jpgLoad,test);
		//	k = cv::waitKey(10);
		//}
		//else
		//{
		//	cout <<"Img Wasnt LOaded" << endl;
		//}
		
		
	}

	return 0;
}

