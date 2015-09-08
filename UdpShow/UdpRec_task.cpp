#include "UdpRec_task.h"
#include <chrono>

//using namspaces
using namespace NUdpMessages;
using namespace NUdpSocket;
using namespace NSafeContainer;

//constant that defines the timeout in mili sec
static const unsigned int TIME_OUT = 2500;

///////////////////////////////////////////////////////////////////////////////
// Function Name: initAndRun
// Description:   init the listener and start it
// Output:        None
// In:            config - socket configuration
//				  
// Return:        true/false 
///////////////////////////////////////////////////////////////////////////////
bool CUdpRecTask::initAndRun(SSocketConfig& config)
{
	bool rv(false);

	if (m_socket.configureSocket(config))
	{
		if (m_socket.openSocket())
		{
                    cleanSocket();
                    rv = true;
                    m_thread.reset(new std::thread([this](){this->mainFunc();}));
		}
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: recAck
// Description:   send the ack msg
// Output:        None
// In:            none
// Return:        true if successfully sent
///////////////////////////////////////////////////////////////////////////////
bool CUdpRecTask::recAck(NUdpMessages::SHeader* buff)
{
	bool rv(true);
	std::lock_guard<std::mutex> guard(m_mutex);
	
	TUDWord sz(sizeof(SAck) - sizeof(SHeader));
	buff += sizeof(SHeader); //move buffer

	if (m_socket.reciveData(reinterpret_cast<TUByte*>(buff), sz, TIME_OUT))
	{
		buff -= sizeof(SHeader); //move back to the begining of the buffer
		SAck* ack = reinterpret_cast<SAck*>(buff);
		m_ack = *ack;
	}
	else
	{
		rv = false;
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: recFrame
// Description:   send frame
// Output:        None
// In:            none
// Return:        true/ false
///////////////////////////////////////////////////////////////////////////////
/*bool CUdpRecTask::recFrame(NUdpMessages::SHeader* buff)
{
	bool rv(true);
	
	TUDWord sz(buff->size - m_bytesWritten);
	
	TUByte* frameBuffer = reinterpret_cast<SFrameRGB*>(buff)->byteVector;

	frameBuffer += m_bytesWritten;

	if (m_socket.reciveData(reinterpret_cast<TUByte*>(frameBuffer), sz, TIME_OUT))
	{
		m_bytesWritten += sz;
	}
	else
	{
		rv = false;
	}

	return rv;
}*/

///////////////////////////////////////////////////////////////////////////////
// Function Name: recFrame
// Description:   send frame
// Output:        None
// In:            none
// Return:        true/ false
///////////////////////////////////////////////////////////////////////////////
bool CUdpRecTask::recFrame(NUdpMessages::SHeader* buff)
{
	bool rv(true);

	TUDWord sz(0);

	TUByte* frameBuffer = reinterpret_cast<SFrameRGB*>(buff)->byteVector;

	//keep recieving frame blocks untill the whole frame is field with data
	while ((buff->size < m_bytesWritten) && (rv))
	{
		frameBuffer += m_bytesWritten;
		sz = buff->size;

		if (m_socket.reciveData(reinterpret_cast<TUByte*>(frameBuffer), sz, TIME_OUT))
		{
			m_bytesWritten += sz;
		}
		else
		{
			//very bad drop this frame
			m_bytesWritten = 0;
			rv = false;
		}
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: recStart
// Description:   rec the start msg
// Output:        None
// In:            none
// Return:        true if successfully sent
///////////////////////////////////////////////////////////////////////////////
bool CUdpRecTask::recStart(NUdpMessages::SHeader* buff)
{
	bool rv(true);
	std::lock_guard<std::mutex> guard(m_mutex);

	TUDWord sz(sizeof(SStart) - sizeof(SHeader));
	buff += sizeof(SHeader); //move buffer

	if (m_socket.reciveData(reinterpret_cast<TUByte*>(buff), sz, TIME_OUT))
	{
		buff -= sizeof(SHeader); //move back to the begining of the buffer
		SStart* start = reinterpret_cast<SStart*>(buff);
		m_start = *start;
	}
	else
	{
		rv = false;
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: mainFunc
// Description:   runs untill alive and performs the threds duties
// Output:        None
// In:            none
// Return:        none 
///////////////////////////////////////////////////////////////////////////////
void CUdpRecTask::mainFunc()
{
	m_alive = true;

	m_safeCar.create(65); //create 65 cells (we want to recive 30 frames/s so we will give duble that size)

	while (m_alive)
	{
		SFrameRGB* data;
		TUDWord sz(0);

		if (m_newFrame)
		{
                    data = m_safeCar.giveCell(m_workingKey);
		    m_newFrame = false;
		}

		sz = sizeof(SHeader);

		//GET HEADER
		if (m_socket.reciveData(reinterpret_cast<TUByte*>(data),sz,TIME_OUT))
		{	
                        if (data->sync != 0xA5A5)
                        {
                            printf("Sync wasnt found in the recived message \n");
                            continue;
                        }
			NUdpSocket::TUDWord ind = toIndex(translateOpCode(static_cast<EOpCodesSend>(data->opCode)));

			if (((ind >= 0) && (ind < OP_REC_COUNT)) && (m_funArr[ind](data)))
			{
				if (m_bytesWritten >= data->size)
				{
					m_bytesWritten = 0;
					m_keyQue.push(m_workingKey);
					m_newFrame = true;
				}
			}
			else
			{
				printf("Error reciving");
			}
		}
		else
		{
			//no data yet sleep for a while
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function Name: translateOpCode
// Description:   translates snd opcodes to rec opcode
// Output:        None
// In:            None 
// Return:        the competable rec opcode 
///////////////////////////////////////////////////////////////////////////////
NUdpMessages::EOpCodesRec CUdpRecTask::translateOpCode(const NUdpMessages::EOpCodesSend& opSnd)
{
    NUdpMessages::EOpCodesRec rv(OP_REC_COUNT);
    
    switch (opSnd)
    {
        case OP_ACK_SND:
            rv = OP_ACK_REC;
            break;
        case OP_START_SND:
            rv = OP_START_REC;
            break;
        case OP_FRAME_DEP_SND:
        case OP_FRAME_RGB_SND:
            rv = OP_FRAME_REC;
            break;
        case OP_SND_COUNT:
        default:
            printf("Couldnt translate the opcode %d \n", static_cast<TUDWord>(opSnd));
    }
    
    return rv;
}
