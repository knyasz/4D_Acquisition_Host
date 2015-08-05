/* 
 * File:   udpSocket.h
 *
 * Created on November 10, 2014, 3:14 PM
 */

//project include
#include "string.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include "udpSocket.h"

//tiny xml include
#include <tinyxml.h>

//using section
using namespace NUdpSocket;

//ctor
CUdpSocket::CUdpSocket():
    m_portNum(-1), m_tgtPort(-1),m_socket(-1),
    m_isConfigured(false), m_isOpen(false),m_maxDataSize(0),
	m_rxBuffSize(0),m_txBuffSize(0)
{
    strcpy(m_name," ");
    memset(&m_localAdd,0,sizeof(m_localAdd));
    memset(&m_tgtAdd,0,sizeof(m_tgtAdd));
}

////////////////////////////////////////
//name  : initialize
//desc  : initializes server
//arg   : vonfig - the socket configuration   
//return: true if init ok false if not 
///////////////////////////////////////
bool CUdpSocket::configureSocket(const SSocketConfig &config)
{
    bool rv(false);
    
    m_maxDataSize = config.maxDataSize;
    
    if (!m_isConfigured)
    {
        if ((config.locPort>0) && (config.tgtPort>0))
        {
           m_portNum = config.locPort;
           m_tgtPort = config.tgtPort;
           rv = true;
        }

        if (rv)
        {
            m_localAdd.sin_family = AF_INET;
            m_localAdd.sin_addr.s_addr = inet_addr(config.locIP);
            m_localAdd.sin_port = htons(m_portNum);
            m_tgtAdd.sin_family = AF_INET;
            m_tgtAdd.sin_addr.s_addr = inet_addr(config.tgtIP);
            m_tgtAdd.sin_port = htons(m_tgtPort);

            m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

            if (m_socket < 0)
            {
               (void)handelError(-1);
               rv = false;
            }

        }

        if (rv)
        {
            strcpy(m_name,config.name);
            m_isConfigured = true;
            m_rxBuffSize = config.rcvBufferSize;
            m_txBuffSize = config.trxBufferSize;
        }
    }
    
    return rv;
}

////////////////////////////////////////
//name  : openSocket
//desc  : open the socket that configuration was passed
//arg   : -  
//return: true if open ok false if not 
///////////////////////////////////////
bool CUdpSocket::openSocket()
{
    bool rv(false);
    static const float MEGA(1024.*1024.);
    
    if ((m_isConfigured) && (!m_isOpen) && 
            (!bind(m_socket,(struct sockaddr*)&m_localAdd,sizeof(m_localAdd))))
    {
        m_isOpen = true;
        rv = true;

        TUDWord sockTrxBuff(0);
        socklen_t len(sizeof(TUDWord));
        TUDWord sockRcvBuff(0);



        (void)getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &sockRcvBuff, &len);
        (void)getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &sockTrxBuff, &len);
        printf("Succesfully changed socket options RcvBuff size is (%.3lf) and  TxBuffsize is (%.3lf) \n",
             ((float)sockRcvBuff/(MEGA)), ((float)sockTrxBuff/(MEGA)));

        socklen_t optLen(sizeof(TUDWord));
        TSDWord res(setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF,&m_txBuffSize,optLen));
        if (res != 0)
        {
          (void)handelError(res);
          rv = false;
         }

         res = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF,&m_rxBuffSize,optLen);
         if (res != 0)
         {
           (void)handelError(res);
            rv = false;
         }
         (void)getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &sockRcvBuff, &len);
         (void)getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &sockTrxBuff, &len);
         printf("Succesfully changed socket options RcvBuff size is (%.3lf) and  TxBuffsize is (%.3lf) \n",
                 ((float)sockRcvBuff/(MEGA)), ((float)sockTrxBuff/(MEGA)));

    }
    else
    {
       (void)handelError(-1);
    }
    
    return rv;
}

////////////////////////////////////////
//name  : closeSocket
//desc  : closes open socket
//arg   : -  
//return: true if closed ok false if not 
///////////////////////////////////////
bool CUdpSocket::closeSocket()
{
    bool rv(false);
    
    if (m_isOpen)
    {
        if (!close(m_socket))
        {
            rv = true;
            m_isOpen = false;
        }
        else
        {
            (void)handelError(-1);
        }
    }

    return rv;
}

////////////////////////////////////////
//name  : sendData
//desc  : send data on the socket
//arg   : buffer - buffer to send
//        size - the size of the buffer to send
//return: true if sent ok false if not 
///////////////////////////////////////
bool CUdpSocket::sendData(TUByte* buffer, TUDWord size)  
{
    bool rv(false);
    
    if (m_isOpen)
    {
        int dataSent(sendto(m_socket,reinterpret_cast<const void*>(buffer),
                size,0,(struct sockaddr*)&m_tgtAdd,sizeof(m_tgtAdd)));
        if (dataSent == size)
        {
            rv = true;
        }
    }
    
    return rv;
}

////////////////////////////////////////
//name  : reciveData
//desc  : recives data sequence
//arg   : buffer - buffer to fill
//      : size: recived data size (the buffer size that is given)
//        to - timeout in mili seconds
//return: true if recived ok false if not 
///////////////////////////////////////
bool CUdpSocket::reciveData( TUByte*  buffer, TUDWord& size,TSDWord to)
{
    bool rv(false);
    struct sockaddr_in currCli;
    static TUDWord clientLen(sizeof(currCli));
    TSDWord status(1);
  
    //memset(buffer,0,size);
    
    if (to != WAIT_FOREVER)
    {
        FD_ZERO(&m_fdr);
        FD_SET(m_socket,&m_fdr);
        m_timeOut.tv_sec = 0;
        m_timeOut.tv_usec = to;
        status = handelError(select(m_socket+1,&m_fdr,NULL,NULL,&m_timeOut)); 
    }
    
    if (status > 0)
    {
        status = handelError(recvfrom(m_socket,buffer,size,0,
                                            (struct sockaddr *) &currCli,&clientLen));
        if (status > 0)
        {
        	size=status;
            if (m_tgtAdd.sin_addr.s_addr == currCli.sin_addr.s_addr)
            {
                rv = true;
            }
        }
    }
    
    return rv;
}


////////////////////////////////////////
//name  : handelError
//desc  : handles system errors
//arg   : the error number and the error string
//return: the error number
///////////////////////////////////////
int CUdpSocket::handelError(const int errorNum, const char* errorStr)
{
    if (errorNum < 0)
    {
        perror(errorStr);
    }
    
    return errorNum;
}
    

////////////////////////////////////////
//name  : initFromFile
//desc  : inits socket from xml file
//arg   : fileName - file to read from
//        socketName - name of the current socket
//return: true if sent ok false if not
///////////////////////////////////////
bool CUdpSocket::initFromFile(const char* fileName, const char* socketName)
{
	bool rv(false);
	SSocketConfig conf;

	if ((fileName) && (socketName))
	{
		TiXmlDocument doc(fileName);
		if (doc.LoadFile())
		{
			TiXmlElement* elem(NULL);
			TiXmlNode* nod(NULL);
			nod = doc.FirstChild(socketName);
			if (nod)
			{
				elem = nod->ToElement();
				if (elem)
				{
					elem = elem->FirstChildElement("Active");
					TUByte isActive(atoi(elem->Value()));
					if (isActive)
					{
						elem = elem->FirstChildElement("Properties");
						if (elem)
						{
							if (elem->Attribute("LocalIp"))
							{
								strcpy(conf.locIP,elem->Attribute("LocalIp"));
								if (elem->Attribute("TgtIp"))
								{
									strcpy(conf.tgtIP,elem->Attribute("TgtIp"));
									if (elem->Attribute("LocalPort"))
									{
										conf.locPort = atoi(elem->Attribute("LocalPort"));
										if (elem->Attribute("TgtPort"))
										{
											conf.tgtPort = atoi(elem->Attribute("TgtPort"));
											if (elem->Attribute("MaxBlockSize"))
											{
												conf.maxDataSize = atoi(elem->Attribute("MaxBlockSize"));
												strcpy(conf.name,socketName);
												rv = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (rv)
		{
			if (!configureSocket(conf))
			{
				rv = false;
			}
		}
		else
		{
			handelError(-1,socketName);
		}
	}

	return rv;
}
     
