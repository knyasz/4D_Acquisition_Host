#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <chrono>
#include "udpSocket.h"

#ifndef _UDPMsg_H
#define	_UDPMsg_H
#pragma pack(push,1)


namespace NUdpMessages
{
	static const NUdpSocket::TUDWord CHUNK_SIZE(64000);
	static const NUdpSocket::TUWord SYNC(0xA5A5);
	enum EOpCodesSend
	{
		OP_ACK_SND = 0,
		OP_START_SND,
		OP_FRAME_DEP_SND,
		OP_FRAME_RGB_SND,

		OP_SND_COUNT
	};

	enum EOpCodesRec
	{
		OP_ACK_REC = 0,
		OP_FRAME_REC,
		OP_START_REC,

		OP_REC_COUNT
	};

	struct SHeader
	{
		NUdpSocket::TUWord sync;
		NUdpSocket::TUDWord size;
		NUdpSocket::TUDWord opCode;
		NUdpSocket::TUDWord checksum;
                NUdpSocket::TUWord  padding;

		SHeader() :
			sync(0xA5A5),
                        padding(0)
		{};
	};
        
        enum EKinect : NUdpSocket::TUDWord
        {
            KINECT_0 = 0,
            KINECT_1,
            KINECT_2,
            KINECT_3,
            
            /////////////////////////// Future kinect ////////////
            KINECT_4,
            KINECT_5,
            KINECT_6,
            KINECT_7,
            KINECT_8,
            KINECT_9,
            KINECT_10,
            KINECT_11,
            KINECT_12,
            KINECT_13,
            KINECT_14,
            KINECT_15,
            KINECT_16,
            KINECT_17,
            KINECT_18
            /////////////////////////////////////////////////////////        
        };
        
        struct SLine0 : public SHeader
        {
            NUdpSocket::TReal64 timeStamp;
            SLine0():
                timeStamp(0.)
            {};               
        };

	struct SAck : public SHeader
	{
            NUdpSocket::TReal64 timeStamp;
            
            SAck():
                timeStamp(0.)
            {};    
	};

	struct SStart : public SHeader
	{
            NUdpSocket::TReal64 timeStamp;
            
            SStart():
                timeStamp(0.)
            {};    
	};

	struct SFrameDep : public SLine0
	{
            NUdpSocket::TUByte byteVector[NUdpSocket::KINECT_FRAME_GRAY_SIZE];
	};

	struct SFrameRGB : public SLine0
	{
		NUdpSocket::TUByte byteVector[NUdpSocket::KINECT_FRAME_RGB_SIZE];
	};

	struct SFrameChunk : public SHeader
	{
		NUdpSocket::TUByte byteVector[CHUNK_SIZE];
	};

#pragma pack(pop)

	static NUdpSocket::TReal64 timeNow()
	{
		auto time = std::chrono::system_clock::now();

		NUdpSocket::TReal64 realTimeMilli =  std::chrono::duration_cast<std::chrono::milliseconds>
								 (time.time_since_epoch()).count();
		return realTimeMilli/1000.;

	}
        
        static bool isWithinTimeDelta(NUdpSocket::TReal64 time2Check)
        {
            static const NUdpSocket::TReal64 DELTA(0.003); //sec
            bool rv(false);
            NUdpSocket::TReal64 ttNow(timeNow());
            
            //only if the timestamp is larger then zero
            if (time2Check > 0)
            {
                rv = true;
                
                //if ttNow > time2Check its all good 
                if (ttNow < time2Check)
                {
                    //if the time2check is bigger then time of the host check if its not 
                    //too far away in the future due to ntp driffts. if its not larger then the delta
                    //allow the delta
                    if ((time2Check - ttNow) > DELTA)
                    {
                        rv = false;
                    }
                }
            }
            
            
            return rv;
        }
}

#endif
