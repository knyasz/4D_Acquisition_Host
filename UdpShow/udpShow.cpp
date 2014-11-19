#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "udpSocket.h"

using namespace cv;
using namespace std;
using namespace NUdpSocket;

const TUDWord to(100000);

int main(int argc, char **argv) {
	bool die(false);
	printf("Hello im udpShow \n");
	Mat depthf (Size(640,480),CV_8UC3/*CV_8UC1*/);

	SSocketConfig conf("132.68.58.191","132.68.58.160",50555,50555,
			KINECT_FRAME_SIZE,"IR IMAGE SOCKET");
	CUdpSocket udpSocket;

	udpSocket.configureSocket(conf);
	udpSocket.openSocket();
	TUByte* buff = new TUByte[KINECT_FRAME_SIZE];
	while (!die) {
		TUByte* buffIterator = buff;
		TUByte* buffEnd = buff + KINECT_FRAME_SIZE;
		static TUDWord size = KINECT_FRAME_SIZE;
//		size = KINECT_FRAME_SIZE;
		for(;buffIterator < buffEnd ; buffIterator+size){
			cout<<endl<<"Buffiterator is : "<<static_cast<void*>(buffIterator)<<endl;
			cout<<endl<<"Received already: "<<(buffIterator - buff)<<endl;
			udpSocket.reciveData(buff,size);
		}
		depthf.data = buff;
		cv::imshow("depth",depthf);
	}

	return 0;
}

