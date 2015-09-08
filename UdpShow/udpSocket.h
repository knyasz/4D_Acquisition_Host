/* 
 * File:   udpSocket.h
 *
 * Created on November 10, 2014, 3:14 PM
 */

#ifndef UDPSOCKET_H
#define	UDPSOCKET_H

//std include
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

//socket includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

namespace NUdpSocket
{  
    typedef unsigned char TUByte;
    typedef unsigned short int TUWord;
    typedef unsigned int TUDWord;
    typedef int TSDWord;
    typedef char* TString;
    typedef float TReal32;
    typedef double TReal64;
    
    //constant 
    static const TUDWord WAIT_FOREVER(-1);
    static const TUDWord MAX_NUM_CHARS(255);
    static const TUDWord KINECT_FRAME_SIZE(640*480);
    static const TUDWord KINECT_FRAME_DEPTH_SIZE(640 * 480 * 2);
    static const TUDWord KINECT_FRAME_RGB_SIZE(640 * 480 * 3);
    

    /*enum that defines posible socket buffer sizes
     * Sets or gets the maximum socket send buffer in bytes.  The  ker-
        nel doubles this value (to allow space for bookkeeping overhead)
        when it is set using setsockopt(), and  this  doubled  value  is
        returned  by  getsockopt().   The  default  value  is set by the
        wmem_default sysctl and the maximum allowed value is set by  the
        wmem_max sysctl.  The minimum (doubled) value for this option is
        2048.
     */
    enum ESockBufferSizes
    {
    	BUFFER_65K  = (1024*65)/2,
    	BUFFER_512k = (1024*512)/2,
    	BUFFER_64M  = (1024*1024*64)/2,
    	BUFFER_128M = (1024*1024*128)/2,
    	BUFFER_254M = (1024*1024*254)/2
    };

    //structure that configures the 
    struct SSocketConfig
    {
        char locIP[MAX_NUM_CHARS];
        char tgtIP[MAX_NUM_CHARS];
        TSDWord locPort;
        TSDWord tgtPort;
        char name[255];
        TSDWord maxDataSize;
        TUDWord rcvBufferSize;
        TUDWord trxBufferSize;
        
        //ctor
        SSocketConfig(TString myIP,TString secIP, TSDWord aLocPort, TSDWord aTgtPort, 
        TSDWord dataSize = KINECT_FRAME_SIZE,TString myName = "General UDP Socket",
        TUDWord rcvBuffSize = BUFFER_128M, TUDWord trxBuffSize = BUFFER_128M)
        {
        	memset(locIP,0,MAX_NUM_CHARS);
        	memset(tgtIP,0,MAX_NUM_CHARS);
            strcpy(locIP,myIP);
            strcpy(tgtIP,secIP);
            locPort = aLocPort;
            tgtPort = aTgtPort;
            strcpy(name,myName);  
            maxDataSize = dataSize;
            rcvBufferSize = rcvBuffSize;
            trxBufferSize = trxBuffSize;
        }

        SSocketConfig()
        {
        	memset(locIP,0,MAX_NUM_CHARS);
        	memset(tgtIP,0,MAX_NUM_CHARS);
        	locPort = 0;
        	tgtPort = 0;
        	memset(name,0,MAX_NUM_CHARS);
        	maxDataSize = 0;
        	rcvBufferSize = 134217728;
        	trxBufferSize = 134217728;
        }

    };
    
    //////////////////////////////////////////////////////
    /// CUdpSocket : class that handles  the udp functions
    ///
    /////////////////////////////////////////////////////
    class CUdpSocket
    {
        //public declarations
    public:

        //constructor
        CUdpSocket();
        
        ////////////////////////////////////////
        //name  : initialize
        //desc  : initializes server
        //arg   : vonfig - the socket configuration   
        //return: true if init ok false if not 
        ///////////////////////////////////////
        bool configureSocket(const SSocketConfig &config);
        
        ////////////////////////////////////////
        //name  : openSocket
        //desc  : open the socket that configuration was passed
        //arg   : -  
        //return: true if open ok false if not 
        ///////////////////////////////////////
        bool openSocket();
        
        ////////////////////////////////////////
        //name  : closeSocket
        //desc  : closes open socket
        //arg   : -  
        //return: true if open ok false if not 
        ///////////////////////////////////////
        bool closeSocket();

        ////////////////////////////////////////
        //name  : reciveData
        //desc  : recives data sequence
        //arg   : buffer - buffer to fill
        //      : size: recived data size
        //        to - timeout in mili seconds
        //return: true if recived ok false if not 
        ///////////////////////////////////////
        bool reciveData(TUByte* buffer, TUDWord& size,TSDWord to = WAIT_FOREVER);
        
        ////////////////////////////////////////
        //name  : sendData
        //desc  : send data on the socket
        //arg   : buffer - buffer to send
        //        size - the size of the buffer to send
        //return: true if sent ok false if not 
        ///////////////////////////////////////
        bool sendData(TUByte* buffer, TUDWord size);

        ////////////////////////////////////////
        //name  : initFromFile
        //desc  : inits socket from xml file
        //arg   : fileName - file to read from
        //        socketName - name of the current socket
        //return: true if sent ok false if not
        ///////////////////////////////////////
        bool initFromFile(const char* fileName, const char* socketName);
        
        ////////////////////////////////////////
        //name  : isConfiguref
        //return: true if configured false if not
        ///////////////////////////////////////
        inline bool isConfiguref() const
        { return m_isConfigured; };
        
        ////////////////////////////////////////
        //name  : isOpened
        //return: true if opened false if not
        ///////////////////////////////////////
        inline bool isOpened() const
        { return m_isOpen; };

        
        //dtor
        virtual ~CUdpSocket(){};


        //private declarations
        private:
      




        ////////////////////////////////////////
        //name  : handelError
        //desc  : handles system errors
        //arg   : the error number and the error string
        //return: the error number
        ///////////////////////////////////////
        int handelError(const int errorNum, const char* errorStr="UDP_SOCKET_ERROR");



        //constructor
        CUdpSocket& operator=(const CUdpSocket& other);
        CUdpSocket(CUdpSocket& other);

        //member that represents the port that will be listened to
        TUDWord m_portNum;
        
        //member that represent the tgt_port
        TUDWord m_tgtPort; 

        //member that represents the socket
        TUDWord m_socket;

        //server address
        struct sockaddr_in m_localAdd;

        //clianet address
        struct sockaddr_in m_tgtAdd;
        
        //meber that represent the maximum size of data
        TUDWord m_maxDataSize;
        
        //member fd_set
        fd_set m_fdr;
        
        //member that will calculate the time out
        struct timeval m_timeOut;
        
        //member that represents the name of the socket
        char m_name[MAX_NUM_CHARS];
        
        //meber that represent it the socket was configured
        bool m_isConfigured;
        
        //member that represent if the socket was opened
        bool m_isOpen;
        
        //members that represent rcv and transmit buffer sizes
        TUDWord m_rxBuffSize;
        TUDWord m_txBuffSize;

       
        //member that says if a fatal error accured 
        //int  m_lastErr;
    
};
}
#endif	/* UDPSOCKET_H */

