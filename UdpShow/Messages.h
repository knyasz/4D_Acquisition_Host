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

	struct SAck : public SHeader
	{
		NUdpSocket::TReal64 timeStamp;
	};

	struct SStart : public SHeader
	{
		NUdpSocket::TReal64 timeStamp;
	};

	struct SFrameDep : public SHeader
	{
		NUdpSocket::TReal64 timeStamp;
		NUdpSocket::TUByte byteVector[NUdpSocket::KINECT_FRAME_GRAY_SIZE];
	};

	struct SFrameRGB : public SHeader
	{
		NUdpSocket::TReal64 timeStamp;
		NUdpSocket::TUByte byteVector[NUdpSocket::KINECT_FRAME_RGB_SIZE];
	};

	struct SFrameChunk : public SHeader
	{
		NUdpSocket::TUByte byteVector[CHUNK_SIZE];
	};
}
#pragma pack(pop)

	static NUdpSocket::TReal64 timeNow()
	{
		auto time = std::chrono::system_clock::now();

		NUdpSocket::TReal64 realTimeMilli =  std::chrono::duration_cast<std::chrono::milliseconds>
								 (time.time_since_epoch()).count();
		return realTimeMilli/1000.;

	}

#endif
