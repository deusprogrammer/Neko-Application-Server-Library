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

void HTTPRequest::init(char* cVerb, char** tokens, int nTokens) {
   if (nTokens < 2)
      return;

   if (stringEquals(cVerb, "GET"))
      verb = GET;
   else if (stringEquals(cVerb, "PUT"))
      verb = PUT;
   else if (stringEquals(cVerb, "POST"))
      verb = POST;
   else if(stringEquals(cVerb, "DELETE"))
      verb = DELETE;

   if (stringContains(tokens[0], '?')) {
      int nQTokens;
      char** qTokens = stringSplit(tokens[0], "?&", &nQTokens);

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

   stringCopy(resource, tokens[0]);
   stringCopy(httpVersion, tokens[1]);
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
      resource = tokens[0];
      httpVersion = tokens[1];

      httpRequest.init(token, tokens, nTokens);
      malformed = false;

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



/*
   int nTokens;

   char** tokens = stringSplit(line, " :", &nTokens);
   
   if (stringEquals(tokens[0], "GET") || stringEquals(tokens[0], "PUT") || stringEquals(tokens[0], "POST") || stringEquals(tokens[0], "DELETE")) {
      httpRequest.init(tokens, nTokens);
      malformed = false;
   }
   else if (stringEquals(tokens[0], "Content-Length")) {
      contentLength = stringToInt(tokens[1]);
   }
   else {
      //Need to have a function that can concatenate tokens 1-n back into a string.
      headerInfo[tokens[0]] = tokens[1];
   }

   free(tokens);
*/
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
   type = HTTP;
   strcpy(this->appName, "NekoServer");
   strcpy(this->port, "80");
   this->nConnections = 0;
   this->maxConnections = 10;
   this->status = STOPPED;
   strcpy(this->htdocsDirectory, "htdocs");
   nGetServices = nPutServices = nPostServices = nDeleteServices = 0;

   mkdir("./htdocs/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

ApplicationServer::ApplicationServer(HTTPProtocol type, char* appName, int maxConnections, char* port) {
   this->type = type;

   strcpy(this->appName, appName);
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
}

ApplicationServer::~ApplicationServer() {
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

   Socket* listener;
   switch (appServer->getType()) {
   case HTTP:
      listener = new TCPSocket(LISTENER);
      break;
   case HTTPS:
      listener = new SSLTCPSocket(LISTENER);
      break;
   };   

   listener->bind(port);

   printf("%s listening on port %s...\n", appServer->getAppName(), appServer->getPort());

   if (appServer->getType() == HTTPS)
      printf("Using %s\n", SSLeay_version(SSLEAY_VERSION));

   Socket* eClient;

   while ((eClient = listener->accept()) && appServer->getStatus() == RUNNING) {
      if (appServer->getAvailableConnections() <= 0 || eClient->wasError()) {
         eClient->close();
         continue;
      }

      appArgs = new ApplicationServerArgs();

      appArgs->appServer = appServer;
      appArgs->sock = eClient;
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
      int nBytes = 0;
      do {
         nBytes = client->readLine(data, header.getContentLength());
         printf("\tRead %d bytes.\n", nBytes);
         if (client->wouldBlock())
            printf("\tClient would block.\n");
      } while (nBytes <= 0 && client->wouldBlock());
      data[nBytes] = 0;
   }

   printf("Checking for registered web service for %s...\n", header.getRequest()->getResource());
   WebService* webService = appServer->fetchService(header.getRequest()->getVerb(), header.getRequest()->getResource());
   if (webService) {
      printf("\tFound %s...\n", header.getRequest()->getResource());
      webService->callback(client, &header, data);
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

   printf("Cleaning up...\n", header.getRequest()->getResource());
   if (data)
      delete data;

   appServer->decrementConnectionCount();

   printf("Exiting thread...\n\n");
   ExitThreadM(0);
}
