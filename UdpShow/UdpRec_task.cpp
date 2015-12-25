#include "UdpRec_task.h"
#include <chrono>

//using namspaces
using namespace NUdpMessages;
using namespace NUdpSocket;
using namespace NSafeContainer;

//constant that defines the timeout in micro sec
static const unsigned int TIME_OUT = 62500;

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
        
        //m_socket.reset(new NUdpSocket::CUdpSocket);
        m_socket = std::make_shared<NUdpSocket::CUdpSocket>();
        
	if (m_socket->configureSocket(config))
	{
		if (m_socket->openSocket())
		{
                    cleanSocket();
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
bool CUdpRecTask::initAndRun(const std::shared_ptr<NUdpSocket::CUdpSocket>& otherSocket)
{
    bool rv(true);
    
    m_socket = otherSocket;
    m_thread.reset(new std::thread([this](){this->mainFunc();}));
    
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
	SAck *ack; //temp pointer to full acl
        
        
        std::lock_guard<std::mutex> guard(m_mutex);
        
        TReal64 ts(timeNow());
        
	ack = reinterpret_cast<SAck*>(buff);
        
        //if the timestamp that was recived larger then the one in the system
        //if yes check if its not in the future.
        if ((m_ack.timeStamp <= ack->timeStamp) && (abs(ack->timeStamp -ts) <= 0.001))
        {
           m_ack.timeStamp = ack->timeStamp; 
        }
        else
        {
            printf("The ack was received with a wrong timestamp currTimestamp "
                    "curr[%lf] rec[%lf] currTime[%lf] \n", m_ack.timeStamp, ack->timeStamp, ts);
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

	if (m_socket.reciveData(reinterpret_cas
                            t<TUByte*>(frameBuffer), sz, TIME_OUT))
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
	while ((m_bytesWritten < buff->size) && (rv))
	{
		//frameBuffer += m_bytesWritten;
		sz = buff->size;

		if (m_socket->reciveData(reinterpret_cast<TUByte*>(frameBuffer), sz, TIME_OUT))
		{
			m_bytesWritten += sz;
                        frameBuffer += sz;
		}
		else
		{
			//very bad drop this frame
			m_bytesWritten = 0;
			rv = false;
		}
	}
        
        if (rv)
        {
            m_bytesWritten = 0;
	    m_newFrame = true;
            m_keyQue.push(m_workingKey);
        }

	return rv;
}


/////////////////////////////////////////////////////////////
// Function Name: recStart
// Description:   rec the start msg
// Output:        None
// In:            none
// Return:        true if successfully sent
///////////////////////////////////////////////////////////////////////////////
bool CUdpRecTask::recStart(NUdpMessages::SHeader* buff)
{
	bool rv(true);
        SStart* start;
        
	std::lock_guard<std::mutex> guard(m_mutex);
        
        start = reinterpret_cast<SStart*>(buff);
        
        TReal64 ts(timeNow());

	//if the timestamp that was recived larger then the one in the system
        //if yes check if its not in the future.
        if ((m_start.timeStamp <= start->timeStamp) && (abs(start->timeStamp - ts) <= 0.001))
        {
           m_start.timeStamp = start->timeStamp; 
        }
        else
        {
            printf("The start was received with a wrong timestamp currTimestamp "
                    "curr[%lf] rec[%lf] currTime[%lf] \n", m_start.timeStamp, start->timeStamp, ts);
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

	m_safeCar.create(1000); //create 65 cells (we want to recive 30 frames/s so we will give duble that size)

	while (m_alive)
	{
		SFrameRGB* data;
		TUDWord sz(0);

		if (m_newFrame)
		{
                    data = nullptr;
                    while (data == nullptr)
                    {      
                        data = m_safeCar.giveCell(m_workingKey);
                    }
		    m_newFrame = false;
		}

		sz = sizeof(SLine0); // MAX MESSAGE SIZE
                
                
		//GET HEADER
		if (m_socket->reciveData(reinterpret_cast<TUByte*>(data),sz,TIME_OUT))
		{	
                        if (data->sync != 0xA5A5)
                        {
                            printf("Sync wasn't found in the received message \n");
                            continue;
                        }
			NUdpSocket::TUDWord ind = toIndex(translateOpCode(static_cast<EOpCodesSend>(data->opCode)));

			if (((ind >= 0) && (ind < OP_REC_COUNT)) && (m_funArr[ind](data)))
			{
                            // if all good just continue
				/*if (m_bytesWritten >= data->size)
				{
					m_bytesWritten = 0;
					m_keyQue.push(m_workingKey);
					m_newFrame = true;
				}*/
			}
			else
			{
				printf("Error receiving msg [%d]", ind);
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
