#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32

#define _WINSOCKAPI_
#include <Windows.h>

HANDLE CreateThreadM(LPTHREAD_START_ROUTINE SocketThread, LPVOID lpParameter);
void ExitThreadM(void* returnValue);

#else

#include <pthread.h>

pthread_t CreateThreadM(void *(*start_routine)(void*), void *arg);
void ExitThreadM(void* returnValue);
#endif
#endif
