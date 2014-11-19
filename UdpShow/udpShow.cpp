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
		static TUDWord size;
		size = KINECT_FRAME_SIZE;
		udpSocket.reciveData(buff,size);
		depthf.data = buff;
		cv::imshow("depth",depthf);
	}

	return 0;
}

