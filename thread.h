#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32

#define _WINSOCKAPI_
#include <Windows.h>

HANDLE CreateThreadM(LPTHREAD_START_ROUTINE SocketThread, LPVOID lpParameter) {
   printf("Entering CreateThreadM()\n");
   return CreateThread(NULL, 0, SocketThread, (LPVOID)lpParameter, 0, NULL);
}

void ExitThreadM(void* returnValue) {
   ExitThread((int)*returnValue);
}

#else

#include <pthread.h>

pthread_t CreateThreadM(void *(*start_routine)(void*), void *arg) {
   printf("Entering CreateThreadM()\n");
   pthread_t handle;
   pthread_create(&handle, NULL, start_routine, arg);

   printf("Created thread...\n");

   return handle;
}

void ExitThreadM(void* returnValue) {
   pthread_exit(&returnValue);
}

#endif
#endif
