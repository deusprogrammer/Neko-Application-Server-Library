#include "nekoServer.h"

bool ApplicationServer::shutdown = false;
int ApplicationServer::nThreads = 0;
int ApplicationServer::nSignals = 0;

void Easter() {
   printf("%s\n\n", NYAN2);
}

void PrintNekoBadge() {
   printf("%s\n", POWERED_BY);
   printf("%s\n\n", NYAN1);
}

void HTTPRequest::init(char* cVerb, char* resource, char* httpVersion) {
   if (stringEquals(cVerb, "GET"))
      verb = GET;
   else if (stringEquals(cVerb, "PUT"))
      verb = PUT;
   else if (stringEquals(cVerb, "POST"))
      verb = POST;
   else if(stringEquals(cVerb, "DELETE"))
      verb = DELETE;

   if (stringContains(resource, '?')) {
      int nQTokens;
      char** qTokens = stringSplit(resource, "?&", &nQTokens);

      for (int i = 0; i < nQTokens; i++) {
         if (stringContains(qTokens[i], '=')) {
            int nElements;
            char** qElements = stringSplit(qTokens[i], "=", &nElements);

            if (nElements == 2)
               queryStringMap[qElements[0]] = qElements[1];

            if (qElements) {
               delete qElements;
            }
         }
      }

      if (qTokens) {
         delete qTokens;
      }
   }

   strcpy(this->resource, resource);
   strcpy(this->httpVersion, httpVersion);
}

void HTTPHeaderObject::consumeLine(char* line) {
   char* next;
   char* token;
   char *resource, *httpVersion;

   token = strtok_r(line, ": ", &next);

   if (strcmp(token, "GET") == 0 || strcmp(token, "PUT") == 0 || strcmp(token, "POST") == 0 || strcmp(token, "DELETE") == 0) {
      int nTokens;
      char** tokens;

      tokens = stringSplit(next, " ", &nTokens);
      if (nTokens >= 2) {
         resource = tokens[0];
         httpVersion = tokens[1];

         httpRequest.init(token, resource, httpVersion);
         malformed = false;
      }

      if (tokens)
         delete tokens;
   }
   else if (stringEquals(token, "Content-Length")) {
      char* p = next;
      while (*p == ' ' && *p != 0) p++;

      contentLength = stringToInt(p);
   }
   else {
      char* p = next;
      while (*p == ' ' && *p != 0) p++;

      headerInfo[token] = p;
   }
}

WebService::WebService(void *(*funcPtr)(Socket*, HTTPHeaderObject*, void*)) {
   this->callback = funcPtr;
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
   printf("ApplicationServer::CREATED %u\n", this);

   type = HTTP;
   strcpy(this->appName, "NekoServer");
   strcpy(this->port, "80");
   this->nConnections = 0;
   this->maxConnections = 10;
   this->status = STOPPED;
   strcpy(this->htdocsDirectory, "htdocs");

   strcpy(this->certificatePath, "server.crt");
   strcpy(this->privateKeyPath, "server.key");

   nGetServices = nPutServices = nPostServices = nDeleteServices = 0;

   mkdir("./htdocs/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

   signal(SIGINT, ApplicationServer::signalHandler);
   signal(SIGQUIT, ApplicationServer::signalHandler);
   signal(SIGHUP, ApplicationServer::signalHandler);
   signal(SIGTERM, ApplicationServer::signalHandler);
   signal(SIGTSTP, ApplicationServer::signalHandler);
   signal(SIGUSR1, SIG_IGN);
   signal(SIGUSR2, SIG_IGN);
}

ApplicationServer::ApplicationServer(HTTPProtocol type, char* appName, int maxConnections, char* port) {
   printf("ApplicationServer::CREATED %u\n", this);
   
   this->type = type;

   strcpy(this->appName, appName);

   strcpy(this->certificatePath, "server.crt");
   strcpy(this->privateKeyPath, "server.key");

   this->status = STOPPED;

   this->nConnections = 0;
   this->maxConnections = maxConnections;
   strcpy(this->htdocsDirectory, "htdocs");

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

   signal(SIGINT, ApplicationServer::signalHandler);
   signal(SIGQUIT, ApplicationServer::signalHandler);
   signal(SIGHUP, ApplicationServer::signalHandler);
   signal(SIGTERM, ApplicationServer::signalHandler);
   signal(SIGTSTP, ApplicationServer::signalHandler);
   signal(SIGUSR1, SIG_IGN);
   signal(SIGUSR2, SIG_IGN);
}

ApplicationServer::~ApplicationServer() {
   printf("ApplicationServer::DELETED %u\n", this);

   map<string, WebService*>::iterator it;

   for (it = getServices.begin(); it != getServices.end(); it++) {
      delete it->second;
   }
   for (it = putServices.begin(); it != putServices.end(); it++) {
      delete it->second;
   }
   for (it = postServices.begin(); it != postServices.end(); it++) {
      delete it->second;
   }
   for (it = deleteServices.begin(); it != deleteServices.end(); it++) {
      delete it->second;
   }
}

void ApplicationServer::addService(HTTPVerb verb, char* resourceName, void *(*funcPtr)(Socket*, HTTPHeaderObject*, void*)) {
   switch(verb) {
   case GET:
      getServices[resourceName] = new WebService(funcPtr);
      break;
   case PUT:
      putServices[resourceName] = new WebService(funcPtr);
      break;
   case POST:
      postServices[resourceName] = new WebService(funcPtr);
      break;
   case DELETE:
      deleteServices[resourceName] = new WebService(funcPtr);
      break;
   }
}

WebService* ApplicationServer::fetchService(HTTPVerb verb, char* resourceName) {
   switch(verb) {
   case GET:
      return getServices[resourceName];      
   case PUT:
      return putServices[resourceName];      
   case POST:
      return postServices[resourceName];      
   case DELETE:
      return deleteServices[resourceName];      
   default:
      return getServices[resourceName];      
   }
}

void ApplicationServer::setHtdocsDirectory(char* htdocsDirectory) {
   strcpy(this->htdocsDirectory, htdocsDirectory);
}

char* ApplicationServer::getHtdocsDirectory() {
   if (!htdocsDirectory)
      return "htdocs";
   else
      return htdocsDirectory;
}

void ApplicationServer::signalHandler(int sigNum) {
   ApplicationServer::shutdown = true;

   printf("Caught signal %d\n", sigNum);

   switch(sigNum) {
   case SIGTSTP:
   case SIGINT:
   case SIGTERM:
   case SIGQUIT:
   case SIGHUP:
      printf("Shutting down...if you want to kill the process, just hit CTRL-C again.\n");
      nSignals++;
      break;
   };

   if (nSignals >= 2) {
      kill(getpid(), SIGKILL);
   }
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
   ApplicationServer::incrementThreadCount();
         
   #ifdef _WIN32
   InitializeWS();
   #endif

   Socket* listener;
   switch (appServer->getType()) {
   case HTTP:
      listener = new TCPSocket(LISTENER);
      break;
   case HTTPS:
      if (!SSLTCPSocket::initSSL(appServer->getCertificatePath(), appServer->getPrivateKeyPath())) {
         fprintf(stderr, "Unable to find certificate or key file for SSL!");
         appServer->setStatus(STOPPED);
         ApplicationServer::decrementThreadCount();
         ExitThreadM(0);
      }
      listener = new SSLTCPSocket(LISTENER);
      break;
   };

   if (!listener->bind(port)) {
      fprintf(stderr, "Unable to bind port %s (ERROR: %d)\n", port, errno);
      appServer->setStatus(STOPPED);
      ApplicationServer::decrementThreadCount();
      ExitThreadM(0);
   }

   printf("%s listening on port %s...\n", appServer->getAppName(), appServer->getPort());

   if (appServer->getType() == HTTPS)
      printf("Using %s\n", SSLeay_version(SSLEAY_VERSION));

   Socket* eClient;
   listener->setNoBlock();

   while (appServer->getStatus() == RUNNING && !ApplicationServer::emergencyShutdown()) {
      eClient = listener->accept();

      if (!eClient && listener->wouldBlock()) {
         sleep(1);
         continue;
      }
      else if (!eClient && !listener->wouldBlock() && !listener->wasFatalError()) {
         fprintf(stderr, "Non-fatal error\n");
         sleep(1);
         continue;
      }
      else if (!eClient && !listener->wouldBlock() && listener->wasFatalError()) {
         fprintf(stderr, "FATAL ERROR!\n");
         break;
      }

      if (appServer->getAvailableConnections() <= 0) {
         eClient->close();
         delete eClient;
         continue;
      }

      appArgs = new ApplicationServerArgs();

      appArgs->appServer = appServer;
      appArgs->sock = eClient;
      appServer->incrementConnectionCount();
         
      CreateThreadM(SocketThread, (LPVOID)appArgs);
   }

   printf("Main accept loop exited...\n");

   appServer->setStatus(STOPPED);

   printf("Cleaning up...\n");
   listener->close();
   delete listener;
   
   switch (appServer->getType()) {
   case HTTP:
      break;
   case HTTPS:
      SSLTCPSocket::tearDownSSL();
      break;
   };

   #ifdef _WIN32
   CleanupWS();
   #endif

   ApplicationServer::decrementThreadCount();

   ExitThreadM(0);
}

#ifdef _WIN32
DWORD WINAPI SocketThread(LPVOID lpargs) {
#else
void* SocketThread(void* lpargs) {
#endif
   char buffer[1024];
   char data[4096];
   //char *data = NULL;
   int nBytes = 0;
   bool reading = true;

   ApplicationServer::incrementThreadCount();

   ApplicationServerArgs* asa = (ApplicationServerArgs*)lpargs;
   Socket* client = asa->sock;
   ApplicationServer* appServer = asa->appServer;
   HTTPHeaderObject header;
   Timer timer(TIMEOUT);
   timer.start();

   delete asa;

   client->setNoBlock();

   printf("Consuming HTTP header...\n");
   while (reading) {
      nBytes = client->readLine(buffer, 1024);
      //End of HTTP header
      if (nBytes >= 0 && nBytes <= 2) {
         printf("END OF HTTP HEADER (nBytes=%d)!\n", nBytes);
         break;
      }
      //Socket open, but no data
      else if (nBytes < 0 && client->wouldBlock()) {
         //Implement a timeout here.
         if (timer.isExpired()) {
            printf("TIME OUT!\n");

            client->close();
            delete client;

            ApplicationServer::decrementThreadCount();
            ExitThreadM(0);
         }
         else {
            sleep(1);
            continue;
         }
      }
      //Socket closed
      else if (nBytes < 0 && !client->wouldBlock()) {
         fprintf(stderr, "READ FAILED!\n");

         client->close();
         delete client;

         ApplicationServer::decrementThreadCount();
         ExitThreadM(0);
      }
      //Data available to read
      else {
         buffer[nBytes - 2] = 0;
         printf("HEADER: %s\n", buffer);
         header.consumeLine(buffer);

         if (header.isMalformed()) {
            fprintf(stderr, "MALFORMED HTTP HEADER!\n");

            client->close();
            delete client;

            ApplicationServer::decrementThreadCount();
            ExitThreadM(0);
         }

         timer.reset();
      }
   }

   printf("Checking content length...\n");
   if (header.getContentLength() > 0) {
      //data = new char[header.getContentLength() + 1];
      
      printf("Reading %d bytes of data...\n", header.getContentLength());
      int nBytes = 0;
      int bytesRead = 0;
      do {
         nBytes = client->read(data + bytesRead, 4096);
         if (nBytes > 0) {
            bytesRead += nBytes;
            data[bytesRead] = 0;
            //strcat(data, buffer);
            printf("\tRead %d bytes: %s\n", nBytes, data);
         }
      } while (nBytes <= 0 && client->wouldBlock());
   }

   //printf("Checking for registered web service for %s...\n", header.getRequest()->getResource());
   WebService* webService = appServer->fetchService(header.getRequest()->getVerb(), header.getRequest()->getResource());

   if (webService) {
      printf("\tFound %s...\n", header.getRequest()->getResource());
      webService->callback(client, &header, data);
      client->close();
   }
   else {
      char path[1024];
      sprintf(path, "%s/%s", appServer->getHtdocsDirectory(), header.getRequest()->getResource());
      FILE* fp = fopen(path, "rb");
      
      if (!fp) {
         fprintf(stderr, "\tUnable to find %s!\n", header.getRequest()->getResource());
         client->write((void*)HTTP_404, sizeof(HTTP_404) - 1);
         client->close();
      }
      else {
         fseek(fp, 0, SEEK_END);
         int size = ftell(fp);
         fseek(fp, 0, SEEK_SET);

         char httpHeader[1024];
         char buffer[1024];
         int nBytes = 0;
         sprintf(httpHeader, "HTTP/1.1 200 Okay\r\nContent-Length:%d\r\n\r\n", size);

         client->write(httpHeader, strlen(httpHeader));
         while ((nBytes = fread(buffer, 1, 1024, fp)) > 0) {
            buffer[nBytes] = 0;
            client->write(buffer, nBytes);
         }

         client->close();
      }
   }

   printf("Cleaning up...\n");
   //if (data)
   //   delete data;

   if (client) {
      delete client;
   }

   appServer->decrementConnectionCount();

   printf("Exiting thread...\n\n");
   ApplicationServer::decrementThreadCount();
   ExitThreadM(0);
}
