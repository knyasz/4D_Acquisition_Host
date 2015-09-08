/*
* File:   SafeQue.hpp
* Author: AlexD
*/




#ifndef _SAFE_QUE_22082015_HPP_
#define _SAFE_QUE_22082015_HPP_

#include <queue> //for std::queue
#include <chrono> //for std::chrono::miliseconds
#include <mutex> //for std::mutex
#include <condition_variable> //for std::condition_variable


namespace NSafeContainer
{

	/*
	* enum that defines the return statuses of the que
	*/
	enum class ESafeQueRetTypes
	{
		SUCCESS = 0, 

		TIME_OUT,
		FAIL,
		EMPTY,

		COUNT
	};

	//constant that defines the wait forever timeout
	static const int WAIT_FOR_EVER(0xffffffff);

	///////////////////////////////////////////////////////////////////////////////
	// Template safe Queue it will provide a safe usage of que for many threads, 
	// the class will use conditional variables to make threads wait for each other 
	// (similaar to semaphore)
	/////////////////////////////////////////////////////////////////////////////// 
	template<typename TData>
	class CSafeQue
	{
	
	public:
		/////////////////////////////////////////////////////////////////////////////// 
		// cto'r
		/////////////////////////////////////////////////////////////////////////////// 
		CSafeQue()
		{}

		///////////////////////////////////////////////////////////////////////////////
		// Function Name: push
		// Description:   push a new object into the que .
		// Output:        None
		// In:            data - what to push 
		// Return:        allways return success
		///////////////////////////////////////////////////////////////////////////////
		ESafeQueRetTypes push(TData const& data)
		{
			{
				//CS AREA THAT WILL BE OPENED AFTER THE AREA ENDS
				std::lock_guard<std::mutex> guard(m_mutex); //lock the que
				m_queue.push(data);
			}

			m_cv.notify_one(); //notify any other thread that is waiting for new data

			return ESafeQueRetTypes::SUCCESS;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Function Name: isEmpty
		// Description:   is the que empty.
		// Output:        None
		// In:			  - 
		// Return:        True/False
		///////////////////////////////////////////////////////////////////////////////
		bool isEmpty() const
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			return m_queue.empty();
		}

		///////////////////////////////////////////////////////////////////////////////
		// Function Name: pop
		// Description:   pop object if available,  if no object in the que try to wait for timeout.
		// Output:        None
		// In:            data - the data that will be returned 
		//				  timeout - time to wait for  in miliseconds
		// Return:        ESafeQueRetTypes 
		///////////////////////////////////////////////////////////////////////////////
		ESafeQueRetTypes pop(TData& data, unsigned int to = WAIT_FOR_EVER)
		{
			ESafeQueRetTypes rv(ESafeQueRetTypes::SUCCESS);

			std::unique_lock<std::mutex> cvLock(m_mutex); //lock for the cv
			
			if (m_queue.empty())
			{
				if (WAIT_FOR_EVER == to)
				{
					m_cv.wait(cvLock);
				}
				else
				{
					if (std::cv_status::timeout == m_cv.wait_for(cvLock, std::chrono::milliseconds(to)))
					{
						rv = ESafeQueRetTypes::TIME_OUT;
					}
				}
			}

			//pop only if successfull
			if (ESafeQueRetTypes::SUCCESS == rv)
			{
				data = m_queue.front();
				m_queue.pop();
			}

			return rv;
		}

	private:
		std::queue<TData> m_queue;
		std::mutex m_mutex;
		std::condition_variable m_cv;
	};

}


#endif   //__SAFE_que__22082015_HPP_