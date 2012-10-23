#ifndef ZMQ_SEMEPHORE_H__
#define ZMQ_SEMEPHORE_H__
#include <pthread.h>
#include <semaphore.h>
#include "Utility.h"

namespace gravity
{

class Semaphore
{
public:
	GRAVITY_API Semaphore();
	GRAVITY_API Semaphore(int count);
	GRAVITY_API void Lock();
	GRAVITY_API void Unlock();
	GRAVITY_API ~Semaphore();
private:
	sem_t semaphore;
};

}

#endif //ZMQ_SEMEPHORE_H__
