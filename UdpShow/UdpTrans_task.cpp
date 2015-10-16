#include "UdpTrans_task.h"
#include <chrono>

//using namspaces
using namespace NUdpMessages;
using namespace NUdpSocket;
using namespace NSafeContainer;

//constant that defines the timeout in micro sec
static const unsigned int TIME_OUT = 33300;

///////////////////////////////////////////////////////////////////////////////
// Function Name: initAndRun
// Description:   init the listener and start it
// Output:        None
// In:            config - socket configuration
//				  
// Return:        true/false 
///////////////////////////////////////////////////////////////////////////////
bool CUdpTransTask::initAndRun(SSocketConfig& config)
{
	bool rv(false);

        //m_socket.reset(new NUdpSocket::CUdpSocket);
        m_socket = std::make_shared<NUdpSocket::CUdpSocket>();
        
	if (m_socket->configureSocket(config))
	{
		if (m_socket->openSocket())
		{
			rv = true;
			m_thread.reset(new std::thread([this](){this->mainFunc();}));
		}
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: initAndRun
// Description:   init the listener and start it
// Output:        None
// In:            otherSocket - just give me the other socket
//				  
// Return:        true on ok started / false on problem detected 
///////////////////////////////////////////////////////////////////////////////
bool CUdpTransTask::initAndRun(const std::shared_ptr<NUdpSocket::CUdpSocket>& otherSocket)
{
    bool rv(true);
    
    m_socket = otherSocket;
    m_thread.reset(new std::thread([this](){this->mainFunc();}));
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: mainFunc
// Description:   runs untill alive and performs the threds duties
// Output:        None
// In:            none
// Return:        none 
///////////////////////////////////////////////////////////////////////////////
void CUdpTransTask::mainFunc()
{
	m_alive = true;
	
	m_safeCar.create(65); //create 65 cells (we want to recive 30 frames/s so we will give duble that size)

	while (m_alive)
	{
		EOpCodesSend opCod;

		if (m_queue.pop(opCod, TIME_OUT) == ESafeQueRetTypes::SUCCESS)
		{
			m_funArr[toIndex(opCod)]();
		}
		else
		{
			//no data yet sleep for a while
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: sendFrame
// Description:   send frame
// Output:        None
// In:            none
// Return:        true/ false
///////////////////////////////////////////////////////////////////////////////
bool CUdpTransTask::sendFrameRGB()
{
	bool rv(false);
	TUDWord key(0xffffffff);

	if (m_keyQue.pop(key, TIME_OUT) == NSafeContainer::ESafeQueRetTypes::SUCCESS)
	{
		SFrameRGB* frameRGB = m_safeCar.getCellByKey(key);
		frameRGB->opCode = OP_FRAME_RGB_SND;
		frameRGB->size = NUdpSocket::KINECT_FRAME_RGB_SIZE;
		frameRGB->timeStamp = timeNow();

		if (frameRGB != nullptr)
		{
			//transmit the header
			if (m_socket->sendData(reinterpret_cast<TUByte*>(frameRGB), sizeof(SHeader)))
			{
				rv = sendFrame(frameRGB->byteVector, frameRGB->size);
			}
		}
	}

	//this cell is not needed anymore no metter what
	m_safeCar.releaseCell(key);

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: sendFrame
// Description:   send frame
// Output:        None
// In:            none
// Return:        true/ false
///////////////////////////////////////////////////////////////////////////////
bool CUdpTransTask::sendFrameDEP()
{
	bool rv(false);
	TUDWord key(0xffffffff);

	if (m_keyQue.pop(key, TIME_OUT) == NSafeContainer::ESafeQueRetTypes::SUCCESS)
	{
		SFrameDep* frameDEP = reinterpret_cast<SFrameDep*>(m_safeCar.getCellByKey(key));
		frameDEP->opCode = OP_FRAME_DEP_SND;
		frameDEP->size = KINECT_FRAME_GRAY_SIZE;
		frameDEP->timeStamp = timeNow();

		if (frameDEP != nullptr)
		{
			//transmit the header
			if (m_socket->sendData(reinterpret_cast<TUByte*>(frameDEP), sizeof(SHeader)))
			{
				rv = sendFrame(frameDEP->byteVector, frameDEP->size);
			}
		}
	}

	//this cell is not needed anymore no metter what
	m_safeCar.releaseCell(key);

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: sendFrame
// Description:   send frame
// Output:        None
// In:            sz - the toal size to send
// Return:        true/ false
///////////////////////////////////////////////////////////////////////////////
bool CUdpTransTask::sendFrame(NUdpSocket::TUByte* vector, NUdpSocket::TUDWord size)
{
	bool rv(true);
	TUDWord byetsWritten(0);
	
	TUDWord chunkSize(NUdpMessages::CHUNK_SIZE);

	while ((byetsWritten <= size) && (rv))
	{
		if (size - byetsWritten < chunkSize)
		{
			chunkSize = size - byetsWritten;
		}

		rv = m_socket->sendData(vector, chunkSize);
		byetsWritten += chunkSize;
		vector += chunkSize;
	}

	return rv;
}



///////////////////////////////////////////////////////////////////////////////
// Function Name: sendAck
// Description:   send the ack msg
// Output:        None
// In:            none
// Return:        true if successfully sent
///////////////////////////////////////////////////////////////////////////////
bool CUdpTransTask::sendAck()
{
	bool rv(false);
	TUDWord sz;
	SAck ack;

	ack.opCode = OP_ACK_SND;
	ack.size = sizeof(SAck) - sizeof(SHeader);
	ack.timeStamp = timeNow();
	
	TUByte* buff(asBuffer(ack, sz));
	ack.size = sz;


	rv = m_socket->sendData(buff, sz);

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: sendStart
// Description:   send the start message
// Output:        None
// In:            none
// Return:        true / false
///////////////////////////////////////////////////////////////////////////////
bool CUdpTransTask::sendStart()
{
	bool rv(false);
	TUDWord sz(0);
	SStart str;

	str.opCode = OP_START_SND;
	str.size = sizeof(SStart) - sizeof(SHeader);
	str.timeStamp = timeNow();

	TUByte* buff(asBuffer(str, sz));
	str.size = sz;

	rv = m_socket->sendData(buff, sz);

	return rv;
}