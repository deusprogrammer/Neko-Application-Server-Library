#define _WINSOCKAPI_

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma comment(lib, "Ws2_32.lib")

#else

#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>

#endif

#ifndef SIMSOCK_H
#define SIMSOCK_H

#define DEFAULT_HOST   "127.0.0.1"
#define DEFAULT_PORT   "1234"
#define DEFAULT_BUFLEN   512
#define PACKET_SIZE      1024

#define UDP 0
#define TCP 1

#define IPV4 AF_INET
#define IPV6 AF_INET6

#define SERVER    0
#define CLIENT    1
#define LISTENER  2

#ifdef _WIN32

#define socklen_t unsigned int

#else

#define SOCKET int
#define LPVOID void*
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif


//IP Address Struct

struct IPAddress {
   unsigned char octet[4];
};


#ifdef _WIN32

int InitializeWS();
int CleanupWS();

#endif

int OpenClientSocket(SOCKET *pSock, char *hostname, char *port, int ip_version, int type);
int OpenServerSocket(SOCKET *pSock, char *port, int ip_version, int type);
void CloseSocket(SOCKET sock);

SOCKET AcceptConnection(SOCKET sock);

int RecvFromSocket(SOCKET sock, LPVOID data, int len, sockaddr* out);
int RecvFromSocket(SOCKET sock, LPVOID data, sockaddr* out);
int SendToSocket(SOCKET sock, LPVOID data, int len, sockaddr* in);
int SendToSocket(SOCKET sock, LPVOID data, sockaddr* in);
int WriteSocket(SOCKET sock, LPVOID data, int buf_sz);
int WriteSocket(SOCKET sock, LPVOID data);
int ReadSocket(SOCKET sock, LPVOID data, int buf_sz);
int ReadLineSocket(SOCKET sock, LPVOID data, int buf_sz);
bool SetSocketNoBlock(SOCKET sock);

unsigned long GetConnectedIP(SOCKET *pSock);
char* GetIPAddressString(unsigned long ip);

class Socket {
protected:
   SOCKET sock;
   int error;
   int endPoint;
   bool fatalError;
   bool isClosed;
public:
   Socket(int endPoint = SERVER) {sock = -1; this->endPoint = endPoint; this->fatalError = false; this->isClosed = false;}
   virtual ~Socket() {}
   virtual bool setFD(SOCKET sock) {this->sock = sock; return true;}
   virtual bool setNoBlock();
   virtual bool wouldBlock() {return errno == EWOULDBLOCK;}
   virtual int getError() {return errno;}
   bool wasError() {return error == -1;}
   virtual void setFatalError() {fatalError = true;}
   virtual bool wasFatalError() {return fatalError;}

   virtual bool connect(char* hostname, char* port) {};

   virtual bool bind(char* port) {};
   virtual Socket* accept() {};

   virtual int write(LPVOID data, int buf_sz) {}
   virtual int read(LPVOID data, int buf_sz) {}
   virtual int readLine(LPVOID data, int buf_sz) {}
   virtual void close() {if (!isClosed) { CloseSocket(sock); isClosed = true;}}
};

class TCPSocket: public Socket {
protected:
public:
   TCPSocket(int endPoint = SERVER) {sock = -1; this->endPoint = endPoint; this->fatalError = false;}
   ~TCPSocket() {}
   bool setFD(SOCKET sock, int endPoint = SERVER) {printf("SET FD TO %d\n", sock); this->sock = sock; return true;}
   bool setNoBlock();
   bool wouldBlock() {return errno == EWOULDBLOCK;}
   int getError() {return errno;}
   void setFatalError() {fatalError = true;}
   bool wasFatalError() {return fatalError;}

   bool connect(char* hostname, char* port);

   bool bind(char* port);
   Socket* accept();

   int write(LPVOID data, int buf_sz);
   int read(LPVOID data, int buf_sz);
   int readLine(LPVOID data, int buf_sz);
   void close();
};

class SSLTCPSocket: public Socket {
protected:
   SSL* ssl;
   static SSL_CTX *serverContext, *clientContext;
   int sslError;
public:
   SSLTCPSocket(int endPoint = SERVER);
   ~SSLTCPSocket();
   
   static bool initSSL(char* certPath = "server.crt", char* keyPath = "server.key");
   static void tearDownSSL();

   bool setFD(SOCKET sock);
   bool setNoBlock();
   bool wouldBlock() {if (endPoint == CLIENT || endPoint == SERVER) return SSL_get_error(ssl, sslError) == SSL_ERROR_WANT_READ; else if (endPoint == LISTENER) return errno == EWOULDBLOCK;}
   int getError() {return SSL_get_error(ssl, sslError);}
   void setFatalError() {fatalError = true;}
   bool wasFatalError() {return fatalError;}

   bool connect(char* hostname, char* port);

   bool bind(char* port);
   Socket* accept();

   int write(LPVOID data, int buf_sz);
   int read(LPVOID data, int buf_sz);
   int readLine(LPVOID data, int buf_sz);
   void close();
};

#endif
