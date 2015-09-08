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
#include "Messages.h"
#include "UdpRec_task.h"
#include "UdpTrans_task.h"

using namespace cv;
using namespace std;
using namespace NUdpSocket;
using namespace NSafeContainer;

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

	SSocketConfig conf("10.0.0.1","10.0.0.2",50555,50555,
			KINECT_FRAME_SIZE,"IR IMAGE SOCKET", BUFFER_128M, BUFFER_512k);
	//make reciver task 
        CUdpRecTask recTask;
        
        bool result = recTask.initAndRun(conf);
	
	if(!result)
        {
            return -1;
	}
	
	while (!die) 
        {

        TUDWord frameKey(0);
        if (recTask.getRecivedFrameKey(frameKey) == ESafeQueRetTypes::SUCCESS)
        {
           depthf.data = (recTask.getCellByKey(frameKey))->byteVector;
        }
	
	//RGB.data = buff;

	
	//cv::imshow(winName,RGB);
	cv::imshow(winName,depthf);

	cv::waitKey(5);
		
			
	}

    return 0;
}

