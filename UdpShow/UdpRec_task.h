#ifndef _UDP_RECEIVE_TASK_
#define _UDP_RECEIVE_TASK_

#include "udpSocket.h"
#include "Messages.h"
#include "SafeQue.hpp"
#include "safeCarusel.h"
#include <functional>
#include <array>
#include <memory>
#include <thread>
#include <mutex>

//////////////////////////////////////////////////////
/// CUdpTransTask : class that will act as the sender 
///				   of data 
/////////////////////////////////////////////////////
class CUdpRecTask
{
public:
	///////////////////////////////////////////////////////////////////////////////
	// ctor
	///////////////////////////////////////////////////////////////////////////////
	CUdpRecTask() : m_newFrame(true) 
	{
		bindFunctors();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: initAndRun
	// Description:   init the listener and start it
	// Output:        None
	// In:            config - socket configuration
	//				  
	// Return:        true on ok started / false on problem detected 
	///////////////////////////////////////////////////////////////////////////////
	bool initAndRun(NUdpSocket::SSocketConfig& config);


	///////////////////////////////////////////////////////////////////////////////
	// Function Name: kill
	// Description:   stop the thread
	// Output:        None
	// In:            none
	// Return:        none 
	///////////////////////////////////////////////////////////////////////////////
	inline void kill()
	{
		m_alive = false;
		(m_thread.get())->join();
	}
        
        ///////////////////////////////////////////////////////////////////////////////
	// Function Name: getRecivedFrameKey
	// Description:   pends on the key que untill recive frame is complite
	// Output:        None
	// In:            none
	// Return:        none 
	///////////////////////////////////////////////////////////////////////////////
	NSafeContainer::ESafeQueRetTypes getRecivedFrameKey(NUdpSocket::TUDWord& key)
	{
           return m_keyQue.pop(key,40);  //give it 60 miliseconds then bail (1 sec = 1000 mili  with 30 frames we need 34 mseconds to get a cell)
	}
        
        

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: releaseCell
	// Description:   release the cell
	// Output:        None
	// In:            Key - key to the cell to release
	// Return:        true/false
	///////////////////////////////////////////////////////////////////////////////
	bool releaseCell(const NUdpSocket::TUDWord Key)
	{
		return m_safeCar.releaseCell(Key);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: getCellByKey
	// Description:   retrive existing cell by its key
	// Output:        None
	// In:            None
	// Return:        Pointer to the cell
	///////////////////////////////////////////////////////////////////////////////
	NUdpMessages::SFrameRGB* getCellByKey(const NUdpSocket::TUDWord Key)
	{
		return m_safeCar.getCellByKey(Key);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: getLastAckTT
	// Description:   get ack time stampTUDWord flushSize = 60000;
	// Output:        None
	// In:            none
	// Return:        true/ false
	///////////////////////////////////////////////////////////////////////////////
	NUdpSocket::TReal64 getLastAckTT()
	{
		std::lock_guard<std::mutex> gaurd(m_mutex);

		return m_ack.timeStamp;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: getLastStartTT
	// Description:   get ack time stamp
	// Output:        None
	// In:            none
	// Return:        true/ false
	///////////////////////////////////////////////////////////////////////////////
	NUdpSocket::TReal64 getLastStartTT()
	{
		std::lock_guard<std::mutex> gaurd(m_mutex);

		return m_start.timeStamp;
	}
        
        

private:
	///////////////////////////////////////////////////////////////////////////////
	// Function Name: mainFunc
	// Description:   runs untill alive and performs the threds duties
	// Output:        NoneTUDWord flushSize = 60000;
	// In:            none
	// Return:        none 
	///////////////////////////////////////////////////////////////////////////////
	void mainFunc();

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: sendAck
	// Description:   send the ack msg
	// Output:        None
	// In:            none
	// Return:        true if successfully sent
	///////////////////////////////////////////////////////////////////////////////
	bool recAck(NUdpMessages::SHeader* buff);

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: recStart
	// Description:   rec the start msg
	// Output:        None
	// In:            none
	// Return:        true if successfully sent
	///////////////////////////////////////////////////////////////////////////////
	bool recStart(NUdpMessages::SHeader* buff);

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: recFrame
	// Description:   recive frame frame
	// Output:        None
	// In:            none
	// Return:        true/ false
	///////////////////////////////////////////////////////////////////////////////
	bool recFrame(NUdpMessages::SHeader* buff);
        
        ///////////////////////////////////////////////////////////////////////////////
	// Function Name: sendFrame
	// Description:   send frame
	// Output:        None
	// In:            none
	// Return:        true/ false
	///////////////////////////////////////////////////////////////////////////////
        inline void cleanSocket()
        {
          NUdpSocket::TUDWord flushSize = 60000;
          std::unique_ptr<NUdpSocket::TUByte[]> flushBuff(new NUdpSocket::TUByte[flushSize]);
          NUdpSocket::TUDWord count(0);
          while(
           m_socket.reciveData(reinterpret_cast<NUdpSocket::TUByte*>(flushBuff.get()),
                  flushSize,10000) && count < 300){ count++; } 
        }

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: bindFunctors
	// Description:   bind the functors array
	// Output:        None
	// In:            None 
	// Return:        None 
	///////////////////////////////////////////////////////////////////////////////
	void bindFunctors()
	{
		m_funArr[toIndex(NUdpMessages::OP_ACK_REC)]  = 
			[this](NUdpMessages::SHeader* buff) -> bool{ return this->recAck(buff); };
		m_funArr[toIndex(NUdpMessages::OP_FRAME_REC)] = 
			[this](NUdpMessages::SHeader* buff) -> bool { return this->recFrame(buff); };
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: toIndex
	// Description:   convert Enum To Index
	// Output:        None
	// In:            eIndex - enum Ind
	// Return:        number Ind 
	///////////////////////////////////////////////////////////////////////////////
	template <typename TEIndex>
	unsigned int toIndex(TEIndex ind)
	{
		return static_cast<unsigned int>(ind);
	}

	//private data
	//this member is the inner task of this class
	std::unique_ptr<std::thread> m_thread;

	//the socket to use
	NUdpSocket::CUdpSocket m_socket;

	//the thread que
	NSafeContainer::CSafeQue<NUdpMessages::EOpCodesRec> m_queue;

	//the thread que
	NSafeContainer::CSafeQue<NUdpSocket::TUDWord> m_keyQue;

	//safeCarusel to be used here 
	NSafeContainer::CSafeCarousel<NUdpMessages::SFrameRGB> m_safeCar;

	//array that will hold the prepare functions
	std::array <std::function<bool(NUdpMessages::SHeader*)>, NUdpMessages::OP_REC_COUNT> m_funArr;

	//kill this thread
	bool m_alive;
	
	//new frame
	bool m_newFrame;
	
	//working key 
	NUdpSocket::TUDWord m_workingKey;

	//last recived ack
	NUdpMessages::SAck m_ack;

	//last start
	NUdpMessages::SStart m_start;

	//mutex for this thread
	std::mutex m_mutex;

	//how many bytes were written to the buffer.
	NUdpSocket::TUDWord m_bytesWritten;
};

#endif