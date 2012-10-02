#include "GravitySemaphore.h"
#include <zmq.h>
#include <sstream>
#include <pthread.h>

using namespace std;

namespace gravity {

Semaphore::Semaphore()
{
	sem_init(&semaphore, 0, 1);
}

Semaphore::Semaphore(int count)
{
	sem_init(&semaphore, 0, count);
}

void Semaphore::Lock()
{
	sem_wait(&semaphore);
}
void Semaphore::Unlock()
{
	sem_post(&semaphore);
}

Semaphore::~Semaphore()
{
	sem_destroy(&semaphore);
}

}//namespace gravity
