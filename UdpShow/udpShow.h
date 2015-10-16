/* 
 * File:   udpShow.h
 * Author: alexandalex
 *
 * Created on October 16, 2015, 12:12 PM
 */

#ifndef UDPSHOW_H__
#define	UDPSHOW_H__

#include <string>
#include "udpSocket.h"

//template functions to kill all threads with the help of variadic templates
//just send it all the threads and special thread you want to kill and it will kill them recursivly.
//the killThread is specialized to kill special threads with the kill command and std::threads with join
//killEeveryone is specialized to recive all threads and kill them with kill thread then stop when the 
//variadic list is empty
template<typename Thrd>
void killThread(Thrd& thrd)
{
    thrd.kill();
};

template<>
void killThread(std::thread& thrd)
{
    thrd.join();
}

void killEveryone()
{};

template<typename FirstType , typename ...TRest>
void killEveryone(FirstType& thrd, TRest&... rest)
{
    killThread(thrd);
    killEveryone(rest...);
}
////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function Name: sendStartAndWaitForAck
// Description:   send start msg to clients and wait for ack
// Output:        none
// In:            reckTask - the task to use for receive
//                tranTask - the task to use for transmit
// Return:        true if successfully false if not
///////////////////////////////////////////////////////////////////////////////
bool sendStartAndWaitForAck(CUdpRecTask* recTask, CUdpTransTask* tranTask);

///////////////////////////////////////////////////////////////////////////////
// Function Name: showerFunc
// Description:   this func will be used as a small thread that will be used to 
//                the frames that are received from the network
// Output:        none
// In:            none
// Return:        none
///////////////////////////////////////////////////////////////////////////////
void showerFunc();

///////////////////////////////////////////////////////////////////////////////
// Function Name: countAndShowFrameCount
// Description:   performance count function to count number of frames recived 
//                per sec and showing it
// Output:        none
// In:            name - The Frame type
// Return:        none
///////////////////////////////////////////////////////////////////////////////
void countAndShowFrameCount(std::string name);

#endif	/* UDPSHOW_H */

