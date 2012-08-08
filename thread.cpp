#include "thread.h"

#ifdef _WIN32
HANDLE CreateThreadM(LPTHREAD_START_ROUTINE SocketThread, LPVOID lpParameter) {
   return CreateThread(NULL, 0, SocketThread, (LPVOID)lpParameter, 0, NULL);
}

void ExitThreadM(void* returnValue) {
   ExitThread((int)*returnValue);
}
#else
pthread_t CreateThreadM(void *(*start_routine)(void*), void *arg) {
   pthread_t handle;
   pthread_create(&handle, NULL, start_routine, arg);
   pthread_detach(handle);
   return handle;
}

void ExitThreadM(void* returnValue) {
   pthread_exit(&returnValue);
}
#endif
