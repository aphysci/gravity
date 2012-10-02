#ifndef ZMQ_SEMEPHORE_H__
#define ZMQ_SEMEPHORE_H__
#include <pthread.h>
#include <semaphore.h>

namespace gravity
{

class Semaphore
{
public:
	Semaphore();
	Semaphore(int count);
	void Lock();
	void Unlock();
	~Semaphore();
private:
	sem_t semaphore;
};

}

#endif //ZMQ_SEMEPHORE_H__
