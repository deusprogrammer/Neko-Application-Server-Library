#include "nekoServer.h"

void Easter() {
   printf("%s\n\n", NYAN2);
}

void PrintNekoBadge() {
   printf("%s\n", POWERED_BY);
   printf("%s\n\n", NYAN1);
}

void HTTPQueryString::add(char* key, char* value) {
   if (nPairs < 64) {
      stringCopy(pairs[nPairs].key, key);
      stringCopy(pairs[nPairs++].value, value);
   }
}

char* HTTPQueryString::operator[](char* key) {
   for (int i = 0; i < nPairs; i++) {
      if (stringEquals(pairs[i].key, key))
         return pairs[i].value;
   }

   return NULL;
}

void HTTPRequest::init(char** tokens, int nTokens) {
   if (nTokens < 3)
      return;

   if (stringEquals(tokens[0], "GET"))
      verb = GET;
   else if (stringEquals(tokens[0], "PUT"))
      verb = PUT;
   else if (stringEquals(tokens[0], "POST"))
      verb = POST;
   else if(stringEquals(tokens[0], "DELETE"))
      verb = DELETE;

   if (stringContains(tokens[1], '?')) {
      int nQTokens;
      char** qTokens = stringSplit(tokens[1], "?&", &nQTokens);

      for (int i = 0; i < nQTokens; i++) {
         if (stringContains(qTokens[i], '=')) {
            int nElements;
            char** qElements = stringSplit(qTokens[i], "=", &nElements);

            if (nElements == 2) {
               queryString.add(qElements[0], qElements[1]);
            }

            free(qElements);
         }
      }

      free(qTokens);
   }

   stringCopy(resource, tokens[1]);
   stringCopy(httpVersion, tokens[2]);
}

void HTTPHeaderObject::consumeLine(char* line) {
   int nTokens;

   char** tokens = stringSplit(line, " :", &nTokens);
   
   if (stringEquals(tokens[0], "GET") || stringEquals(tokens[0], "PUT") || stringEquals(tokens[0], "POST") || stringEquals(tokens[0], "DELETE")) {
      httpRequest.init(tokens, nTokens);
      malformed = false;
   }
   else if (stringEquals(tokens[0], "Content-Length")) {
      contentLength = stringToInt(tokens[1]);
   }
   //More header items to come

   free(tokens);
}

void ApplicationServer::start() {
   PrintNekoBadge();
   CreateThreadM(ServerThread, this);
   while(!this->isRunning());
}

void ApplicationServer::stop() {
   this->status = STOPPED;
}

ApplicationServer::ApplicationServer() {
   type = HTTP;
   strcpy(this->appName, "NekoServer");
   strcpy(this->port, "80");
   this->nConnections = 0;
   this->maxConnections = 10;
   this->status = STOPPED;
   nGetServices = nPutServices = nPostServices = nDeleteServices = 0;
}

ApplicationServer::ApplicationServer(HTTPProtocol type, char* appName, int maxConnections, char* port) {
   this->type = type;

   strcpy(this->appName, appName);
   this->status = STOPPED;

   this->nConnections = 0;
   this->maxConnections = maxConnections;

   if (port == NULL) {
      switch(type) {
      case HTTP:
         strcpy(this->port, "80"); 
         break;
      case HTTPS:
         strcpy(this->port, "443"); 
         break;
      }
   }
   else
      strcpy(this->port, port); 

   nGetServices = nPutServices = nPostServices = nDeleteServices = 0;
}

ApplicationServer::~ApplicationServer() {
}

void ApplicationServer::addService(HTTPVerb verb, char* resourceName, void *(*funcPtr)(Socket*, HTTPHeaderObject*, void*)) {
   switch(verb) {
   case GET:
      getServices[nGetServices].callback = funcPtr;
      strcpy(getServices[nGetServices++].resourceName, resourceName);      
      break;
   case PUT:
      putServices[nPutServices].callback = funcPtr;
      strcpy(putServices[nPutServices++].resourceName, resourceName);      
      break;
   case POST:
      postServices[nPostServices].callback = funcPtr;
      strcpy(postServices[nPostServices++].resourceName, resourceName);      
      break;
   case DELETE:
      deleteServices[nDeleteServices].callback = funcPtr;
      strcpy(deleteServices[nDeleteServices++].resourceName, resourceName);      
      break;
   }
}

WebService* ApplicationServer::fetchService(HTTPVerb verb, char* resourceName) {
   int nServices;
   WebService* services;

   switch(verb) {
   case GET:
      nServices = nGetServices;
      services = getServices;
      break;
   case PUT:
      nServices = nPutServices;
      services = putServices;
      break;
   case POST:
      nServices = nPostServices;
      services = postServices;
      break;
   case DELETE:
      nServices = nDeleteServices;
      services = deleteServices;
      break;
   default:
      nServices = nGetServices;
      services = getServices;
      break;
   }

   for (int i = 0; i < nServices; i++) {
      if (stringEquals(services[i].resourceName, resourceName)) {
         return &services[i];
      }
   }

   return NULL;
}

#ifdef _WIN32
DWORD WINAPI ServerThread(LPVOID lpargs) {
#else
void* ServerThread(void* lpargs) {
#endif
   ApplicationServer* appServer = (ApplicationServer*)lpargs;
   SOCKET server, client;
   Socket* tsocket;
   ApplicationServerArgs* appArgs;
   char* port = appServer->getPort();

   appServer->setStatus(RUNNING);
         
   #ifdef _WIN32
   InitializeWS();
   #endif

   //Open server socket, type can be either TCP or UDP...this example uses TCP
   if (OpenServerSocket(&server, port, IPV4, TCP) == -1) {
      appServer->stop();
      ExitThreadM(0);
   }

   printf("%s listening on port %s...\n", appServer->getAppName(), appServer->getPort());

   if (appServer->getType() == HTTPS)
      printf("Using %s\n", SSLeay_version(SSLEAY_VERSION));

   //Start an accept connection loop
   while((client = AcceptConnection(server)) && appServer->getStatus() == RUNNING) {
      char* connectedIP = GetIPAddressString(GetConnectedIP(&client));
      printf("Connection from %s\n", connectedIP);

      if (appServer->getAvailableConnections() <= 0) {
         CloseSocket(client);
         continue;
      }

      switch (appServer->getType()) {
      case HTTP:
         tsocket = new TCPSocket();
         break;
      case HTTPS:
         tsocket = new SSLTCPSocket();
         break;
      };

      if(!tsocket->setFD(client)) {
         CloseSocket(client);
         continue;
      }
            

      appArgs = new ApplicationServerArgs();

      appArgs->appServer = appServer;
      appArgs->sock = tsocket;
      appServer->incrementConnectionCount();
         
      CreateThreadM(SocketThread, (LPVOID)appArgs);
   }

   appServer->setStatus(STOPPED);

   CloseSocket(server);

   #ifdef _WIN32
   CleanupWS();
   #endif


   ExitThreadM(0);
}

#ifdef _WIN32
DWORD WINAPI SocketThread(LPVOID lpargs) {
#else
void* SocketThread(void* lpargs) {
#endif
   char buffer[4096];
   char *data = NULL;
   int nBytes = 0;
   bool reading = true;

   ApplicationServerArgs* asa = (ApplicationServerArgs*)lpargs;
   Socket* client = asa->sock;
   ApplicationServer* appServer = asa->appServer;
   HTTPHeaderObject header;
   Timer timer(TIMEOUT);
   timer.start();

   client->setNoBlock();

   printf("Consuming HTTP header...\n");
   while (reading) {
      nBytes = client->readLine(buffer, 1024);
      //End of HTTP header
      if (nBytes > 0 && nBytes <= 2) {
         printf("END OF HTTP HEADER!\n");
         break;
      }
      //Socket open, but no data
      else if (nBytes < 0 && client->wouldBlock()) {
         //Implement a timeout here.
         if (timer.isExpired()) {
            printf("TIME OUT!\n");
            client->close();
            ExitThreadM(0);
         }
         else
            continue;
      }
      //Socket closed
      else if (nBytes < 0 && !client->wouldBlock()) {
         printf("READ FAILED!\n");
         client->close();
         ExitThreadM(0);
      }
      //Data available to read
      else {
         buffer[nBytes - 2] = 0;
         printf("HEADER: %s\n", buffer);
         header.consumeLine(buffer);

         if (header.isMalformed()) {
            printf("MALFORMED HTTP HEADER!\n");
            client->close();
            ExitThreadM(0);
         }

         timer.reset();
      }
   }

   printf("Checking content length...\n");
   if (header.getContentLength() > 0) {
      data = new char[header.getContentLength()];

      printf("Reading %d bytes of data...\n", header.getContentLength());
      nBytes = client->readLine(data, header.getContentLength());
      data[nBytes] = 0;
   }

   printf("Checking for registered web service for %s...\n", header.getRequest()->getResource());
   WebService* webService = appServer->fetchService(header.getRequest()->getVerb(), header.getRequest()->getResource());
   if (webService) {
      printf("\tFound %s...\n", header.getRequest()->getResource());
      webService->callback(client, &header, data);
   }
   else {
      fprintf(stderr, "\tUnable to find %s!\n", header.getRequest()->getResource());
      client->write((void*)HTTP_404, sizeof(HTTP_404) - 1);
      client->close();
   }

   printf("Cleaning up...\n", header.getRequest()->getResource());
   if (data)
      delete data;

   appServer->decrementConnectionCount();

   printf("Exiting thread...\n\n");
   ExitThreadM(0);
}

