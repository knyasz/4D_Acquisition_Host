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
#include <mutex>

using namespace cv;
using namespace std;
using namespace NUdpSocket;
using namespace NSafeContainer;

const TUDWord to(100000);

std::mutex g_mutex;
NUdpMessages::SFrameDep* g_depth;
NUdpMessages::SFrameRGB* g_rgb;
bool g_newFrame(false);
TUDWord g_key;

//make reciver task 
CUdpRecTask g_recTask;

void shower()
{
    Mat depthf(Size(640,480),CV_8UC1);
    std::string winName("depth");
    namedWindow(winName,CV_WINDOW_AUTOSIZE);
    
    TUDWord oldKey(70);
    
    while(1)
    {
        //cs
        {
            std::lock_guard<std::mutex> guard(g_mutex);
            if (g_newFrame)
            {
                g_recTask.releaseCell(oldKey);
                oldKey = g_key;
                depthf.data = g_depth->byteVector;
                g_newFrame = false;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        cv::imshow(winName,depthf);

        cv::waitKey(5);
    }
    
}


int main(int argc, char **argv) {
	bool die(false);
	printf("Hello im udpShow \n");
        
	Mat RGB(Size(640,480),CV_8UC3);
	//Mat jpgRead(Size(640,480),CV_8UC1);
        //Mat depthf(Size(640,480),CV_8UC1);
	//FILE* rcvdData;
	//std::string winName("depth");
//	std::string winName("RGB");
	//std::string jpgLoad("jpg");
	//namedWindow(winName,CV_WINDOW_AUTOSIZE);
	//cv::imshow(winName,depthf);

        SSocketConfig conf("10.0.0.1","10.0.0.2",50555,50555,
			KINECT_FRAME_SIZE,"IR IMAGE SOCKET", BUFFER_128M, BUFFER_512k);
	
        
        bool result(g_recTask.initAndRun(conf));
        
        if (!result)
        {
            return -1;
        }
	
	
	
        std::thread showerThread(shower);
        
	while (!die) 
        {

            TUDWord frameKey(70); //keys can only get to 64
            if (g_recTask.getRecivedFrameKey(frameKey) == ESafeQueRetTypes::SUCCESS)
            {
               //depthf.data = (r(ecTask.getCellByKey(frameKey))->byteVector;
                std::lock_guard<std::mutex> guard(g_mutex);
                g_depth = reinterpret_cast<NUdpMessages::SFrameDep*>(g_recTask.getCellByKey(frameKey));
                g_newFrame = true;
                g_key = frameKey;
            }
			
	}
        
        g_recTask.kill();
        showerThread.join();

    return 0;
}

