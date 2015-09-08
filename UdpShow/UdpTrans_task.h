#ifndef _UDP_TRANS_TASK_
#define _UDP_TRANS_TASK_

#include "udpSocket.h"
#include "Messages.h"
#include "SafeQue.hpp"
#include "safeCarusel.h"
#include <functional>
#include <array>
#include <memory>
#include <thread>

//////////////////////////////////////////////////////
/// CUdpTransTask : class that will act as the sender 
///				   of data 
/////////////////////////////////////////////////////
class CUdpTransTask
{
public:

	///////////////////////////////////////////////////////////////////////////////
	//ctor
	//////////////////////////////////////////////////////////////////////////////
	CUdpTransTask()
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
	// Function Name: initAndRun
	// Description:   init the listener and start it
	// Output:        None
	// In:            opcode - the operation opcode
	//				  
	// Return:        none 
	///////////////////////////////////////////////////////////////////////////////
	NSafeContainer::ESafeQueRetTypes sendMsg(const NUdpMessages::EOpCodesSend& opCode)
	{
		return m_queue.push(opCode);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: initAndRun
	// Description:   init the listener and start it
	// Output:        None
	// In:            opCode - operation opcode
	//				  key - carusel key
	// Return:        none 
	///////////////////////////////////////////////////////////////////////////////
	NSafeContainer::ESafeQueRetTypes sendMsg(const NUdpMessages::EOpCodesSend& opCode, const NUdpSocket::TUDWord key)
	{
		NSafeContainer::ESafeQueRetTypes rv(NSafeContainer::ESafeQueRetTypes::SUCCESS);

		if (m_queue.push(opCode) == NSafeContainer::ESafeQueRetTypes::SUCCESS)
		{
			rv = m_keyQue.push(key);
		}

		return rv;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: kill
	// Description:   stop the thread
	// Output:        None
	// In:            none
	// Return:        none 
	///////////////////////////////////////////////////////////////////////////////
	void kill()
	{
		m_alive = false;
		(m_thread.get())->join();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: getCellByKey
	// Description:   retrive existing cell by its key
	// Output:        None
	// In:            None
	// Return:        Pointer to the cell
	///////////////////////////////////////////////////////////////////////////////
	NUdpMessages::SFrameRGB* giveCell(NUdpSocket::TUDWord& Key)
	{
		return m_safeCar.giveCell(Key);
	}

private:
	///////////////////////////////////////////////////////////////////////////////
	// Function Name: mainFunc
	// Description:   runs untill alive and performs the threds duties
	// Output:        None
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
	bool sendAck();

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: sendStart
	// Description:   send the start message
	// Output:        None
	// In:            none
	// Return:        true / false
	///////////////////////////////////////////////////////////////////////////////
	bool sendStart();

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: sendFrame
	// Description:   send frame
	// Output:        None
	// In:            sz - the toal size to send
	// Return:        true/ false
	///////////////////////////////////////////////////////////////////////////////
	bool sendFrame(NUdpSocket::TUByte* vector, NUdpSocket::TUDWord size);

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: sendFrameRGB
	// Description:   send frame
	// Output:        None
	// In:            none
	// Return:        true/ false
	///////////////////////////////////////////////////////////////////////////////
	bool sendFrameRGB();

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: sendFrameDEP
	// Description:   send frame
	// Output:        None
	// In:            none
	// Return:        true/ false
	///////////////////////////////////////////////////////////////////////////////
	bool sendFrameDEP();

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: bindFunctors
	// Description:   bind the functors array
	// Output:        None
	// In:            None 
	// Return:        None 
	///////////////////////////////////////////////////////////////////////////////
	void bindFunctors()
	{
		m_funArr[toIndex(NUdpMessages::OP_ACK_SND)] = 
                        [this]()->bool { this->sendAck(); return true;};
		m_funArr[toIndex(NUdpMessages::OP_START_SND)] = 
                        [this]() ->bool { this->sendStart(); return true;};
		m_funArr[toIndex(NUdpMessages::OP_FRAME_RGB_SND)] = 
                        [this]() ->bool { this->sendFrameRGB(); return true;};
		m_funArr[toIndex(NUdpMessages::OP_FRAME_DEP_SND)] = 
                        [this]() ->bool { this->sendFrameDEP(); return true;};
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: toIndex
	// Description:   convert Enum To Index
	// Output:        NoneRECIVE
	// In:            eIndex - enum Ind
	// Return:        number Ind 
	///////////////////////////////////////////////////////////////////////////////
	template <typename TEIndex>
	unsigned int toIndex(TEIndex ind)
	{
		return static_cast<unsigned int>(ind);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function Name: asBuffer
	// Description:   converts a data type into a buffer
	// Output:        None
	// In:            buff - the recived buffer
	//				  sz  - size of the buffer
	// Return:        number Ind 
	///////////////////////////////////////////////////////////////////////////////
	template <typename TBufferType>
	NUdpSocket::TUByte* asBuffer(TBufferType& buff, NUdpSocket::TUDWord sz)
	{
		sz = sizeof(buff);
		return reinterpret_cast<NUdpSocket::TUByte*>(&buff);
	}
	
	//private data
	//this member is the inner task of this class
	std::unique_ptr<std::thread> m_thread;

	//the socket to use
	NUdpSocket::CUdpSocket m_socket;

	//the thread que
	NSafeContainer::CSafeQue<NUdpMessages::EOpCodesSend> m_queue;

    //array that will hold the prepare functions
	std::array <std::function<bool()>, NUdpMessages::OP_SND_COUNT> m_funArr;

	//safe carusel for buffer
	NSafeContainer::CSafeCarousel<NUdpMessages::SFrameRGB> m_safeCar;

	//que with curr working keys
	NSafeContainer::CSafeQue<NUdpSocket::TUDWord> m_keyQue;

	//kill this thread
	bool m_alive;

};

#endif