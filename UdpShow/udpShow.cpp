#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "udpSocket.h"
#include <unistd.h>
#include "Messages.h"
#include "UdpRec_task.h"
#include "UdpTrans_task.h"
#include <mutex>
#include <map>
#include <utility>
#include "udpShow.h"

using namespace cv;
using namespace std;
using namespace NUdpSocket;
using namespace NSafeContainer;

const TUDWord to(100000);

std::mutex g_mutex;
NUdpMessages::SFrameDep* g_depth;
NUdpMessages::SFrameRGB* g_rgb;
bool g_newFrameDEP(false);
bool g_newFrameRGB(false);
TUDWord g_keyDEP;
TUDWord g_keyRGB;


//make reciver task 
CUdpRecTask g_recTask;

//make sendrTask
CUdpTransTask g_transTask;


int main(int argc, char **argv) {
	bool die(false);
	printf("Hello im udpShow \n");
        
	//Mat RGB(Size(640static TUDWord fameCount(0);,480),CV_8UC3);
	//Mat jpgRead(Size(640,480),CV_8UC1);
        //Mat depthf(Size(640,480),CV_8UC1);
	//FILE* rcvdData;
	//std::string winName("depth");
//	std::string winName("RGB");
	//std::string jpgLoad("jpg");
	//namedWindow(winName,CV_WINDOW_AUTOSIZE);
	//cv::imshow(winName,depthf);

        SSocketConfig conf("10.0.0.1","10.0.0.2",50555,50555,
			KINECT_FRAME_GRAY_SIZE,"IR IMAGE SOCKET", BUFFER_128M, BUFFER_512k);
	

        
        bool result(g_recTask.initAndRun(conf));
        result = result &&  g_transTask.initAndRun(g_recTask.getSocket());
        
        if (!result)
        {
            killEveryone(g_recTask,g_transTask);
            return -1;     
        }
	
	
        //send start to all the jetsons (not active now))
        if (!sendStartAndWaitForAck(&g_recTask,&g_transTask))
        {
            killEveryone(g_recTask, g_transTask);
            return -1;
        }
	
        std::thread showerThread(showerFunc);
        
	while (!die) 
        {

            TUDWord frameKey(70); //keys can only get to 64
            if (g_recTask.getRecivedFrameKey(frameKey) == ESafeQueRetTypes::SUCCESS)
            {
               //depthf.data = (r(ecTask.getCellByKey(frameKey))->byteVector;
                std::lock_guard<std::mutex> guard(g_mutex);
                g_rgb = g_recTask.getCellByKey(frameKey);
               
                if (g_rgb->size == KINECT_FRAME_GRAY_SIZE)
                {
                    g_depth = reinterpret_cast<NUdpMessages::SFrameDep*>(g_rgb);
                    g_newFrameDEP = true;
                    g_keyDEP = frameKey;
                }
                else if (g_depth->size == KINECT_FRAME_RGB_SIZE)
                {
                    g_newFrameRGB = true;
                    g_keyRGB = frameKey;
                }
                  
                /*depthf.data = (reinterpret_cast<NUdpMessages::SFrameDep*>(g_recTask.getCellByKey(frameKey)))->byteVector;
                cv::imshow(winName,depthf);

                cv::waitKey(1);
                g_recTask.releaseCell(frameKey);*/
                
            }
			
	}
        
        killEveryone(g_recTask, g_transTask, showerThread);

    return 0;
}


void countAndShowFrameCount(std::string name)
{
    static std::map<std::string,TUDWord> counter;
    static bool isInit(false);
    
    if (!isInit)
    {
        counter["Depth"] = 0;
        counter["RGB"] = 0;
        isInit = true;
    }
    
    static TReal64 tt(timeNow());
    
    ++counter[name];
    
    TReal64 curTT(timeNow());
    
    if (curTT - tt >= 1)
    {
        printf("Successfully transmitted [%d] Depth frames\n", counter["Depth"]);
        printf("Successfully transmitted [%d] Rgb frames\n", counter["RGB"]);
        tt = curTT;
        counter["Depth"] = 0;
        counter["RGB"] = 0;   
    }
}

void showerFunc()
{
    Mat depthf(Size(640,480),CV_8UC1);
    Mat rgbf(Size(640,480),CV_8UC3);
    std::string winNameDep("depth");
    std::string winNameRgb("rgb");
    namedWindow(winNameDep,CV_WINDOW_AUTOSIZE);
    namedWindow(winNameRgb,CV_WINDOW_AUTOSIZE);
    
    
    TUDWord oldKeyDep(70);
    TUDWord oldKeyRgb(70);
    
    while(1)
    {
        //cs
        {
            std::lock_guard<std::mutex> guard(g_mutex);
            if (g_newFrameDEP)
            {
                printf("release cell dep [%d] \n", oldKeyDep);
                g_recTask.releaseCell(oldKeyDep);
                oldKeyDep = g_keyDEP;
                printf("got new key for cell dep [%d] \n", oldKeyDep);
                depthf.data = g_depth->byteVector;
                g_newFrameDEP = false;
                //countAndShowFrameCount(std::move("Depth"));
            }
            
            if (g_newFrameRGB)
            {
                printf("release cell rgb [%d] \n", oldKeyRgb);
                g_recTask.releaseCell(oldKeyRgb);
                oldKeyRgb = g_keyRGB;
                printf("got new key for cell rgb [%d] \n", oldKeyRgb);
                rgbf.data = g_rgb->byteVector;
                g_newFrameRGB = false;
                //countAndShowFrameCount(std::move("RGB"));
            }
               
        }

        cv::imshow(winNameDep,depthf);
        cv::imshow(winNameRgb,rgbf);

        cv::waitKey(1);
    }
    
}


/*void showerRGB()
{
    Mat rgbf(Size(640,480),CV_8UC3);
    std::string winName("RGB");
    namedWindow(winName,CV_WINDOW_AUTOSIZE);
    
    TUDWord oldKey(70);
    
    while(1)
    {
        //cs
        {
            //std::lock_guard<std::mutex> guard(g_mutex);
            if (g_newFrameRGB)
            {
                g_recTask.releaseCell(oldKey);
                oldKey = g_keyRGB;
                rgbf.data = g_rgb->byteVector;
                g_newFrameRGB = false;
            }
        }

        cv::imshow(winName,rgbf);

        cv::waitKey(1);
    }
}*/

bool sendStartAndWaitForAck(CUdpRecTask* recTask, CUdpTransTask* tranTask)
{
    bool rv(false);
    
    //send start message to all the jetsons and wait for a response if not try for 3 times then die
     if (tranTask->sendMsg(NUdpMessages::OP_START_SND) == ESafeQueRetTypes::SUCCESS)
     {
            
        TUDWord failCounter(0);
        TReal64 tt(timeNow());
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); //give time to replay

        while ((failCounter < 60) && (!rv)) //try to connect for a minute then bail
        {
           if (recTask->getLastAckTT() >=  tt)
           {
               rv = true;
           }
           else
           {
               failCounter++;
               tranTask->sendMsg(NUdpMessages::OP_START_SND);
               tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
               std::this_thread::sleep_for(std::chrono::milliseconds(500));
           }
        }            
      }
      
    
    return rv;
}