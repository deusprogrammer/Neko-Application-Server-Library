#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "simSock.h"
#include "strutils.h"
#include "thread.h"

#ifndef NEKOSERVER_H
#define NEKOSERVER_H

#ifndef SOCKET
#define SOCKET int
#endif

#define HTTP_404 "HTTP/1.1 404 Not Found\r\n\r\n<html><body><h1>File Not Found!</h1></body></html>"

#define VERSION 1.0b

#define POWERED_BY "\
\t*******************\n\
\t*  Powered by...  *\n\
\t*******************"

#define NYAN1 "\
　　　_＿_\n\
　(ﾂ'´￣ `'(ﾂ\n\
　 |　|_｣L｣|｣     ------------------------------------------\n\
　 |(i｣ﾟ ｰﾟｿ｣     | Neko-Server Application Server Library |\n\
　 |_/i}笊i匸})   | Version 1.0b                           |\n\
　 く_/†__†〉     | by deusprogrammer                      |\n\
　 └ｕ‐u┘         ------------------------------------------"

#define NYAN2 "\
@    ,'''''''''''''''+@`   `;''''''''''''':`@`  .''''''''''''''''''';:# .` `:'''''''''''#+'.''',``:''''';:''';,:;''''''''''''''\n\
@ ,` ,''''''''''''''''@'    `;''''''''''';..#+   ;'''''''''''''''''.`.#:`,  .;':.;;'''''+#: `@` ,'''''';''';,:;''''''''''''''''\n\
@ ,. .''''''''''#+''''+@`    `:''''''''''';''@.  `'''''''''''''''';,.,:@`,,  .,:'''''''''#.  '':''''''''''::'''''''''''''''''''\n\
@ ., .''''''''''@+'''''##     `:''''''''''';:+;   :''''''''''''''++#@@@@@@@@+::''''''''''+'`  @:''''''''::''''''''''#''''''''''\n\
@``. `'''''''''@@+'''''+@:  `` `:'''''''';:; .@`  `;'''''''''''#@#',....@`,;;#@@#';'''''';@.  '';'''''';'''',.'''''''#''''''':.\n\
#: ,` ;'''''''#.##''''''#@.  `` `:''''''';''``#'`  .;''''''''#@#::;',...'+ ::``:#@@+''''''#:. `@.'''''''';'', ,'''''''#'''';,:'\n\
'# `. ,''''''#: ##:''''''@#`  .,;#@@@@#+''';..,@.   ,''''''+@#'''''',....@.`:,  .;#@#'''''++   +;.'''';:;''''` .;'::,;#+'';:;''\n\
`@;`, `'''''+' ,:#'''''''+@'``#@':'''+#@@@'....#+,   :''';##'''''''......:# .;.  .;'@#''':.@   `# :;;,``''''';` `:` ,`:@';'''''\n\
 #';', '''''# ```@''''''''#@+#, ;'''''''';;....,@'.  `:'''''''';':........#' .:.  .;'@+':,;@:   #.      ::,,.::`  .  , '#''''''\n\
 .@.;'`:'';#``;`.@'''''''''@@;` `''''''';.......+#:`  `;''''';'';,......``:@. .;.  ,;+@;'''#+`. ,+ '#@@@#:..  `;.  ` `, #+'''''\n\
  ++'''.';#.`;` ;##'''''''#@@+';'''''''';........@:``  `'''''''''';........:@, `:`  ,;#';:;+#::  @` .,.,'@#`  `:;.    ..`@'''''\n\
  ,@''':;#; :. :'+@''''''#@'+@+'''''''';.........,@;;.  `'''''''++''';,...``##':::`  ,';;''.@':  ';` `....+@,.,'';.    ,`'#''''\n\
  .+#'';++ `, .'''@#''''+@'''#@'''''''':..........'@'':` ''''+#@@@@@@@@#+, `;#+''':`  ,'''..@':. `@. `.....+@.'''';,   `: @+'''\n\
    #'';#  , `;''''@#'''@'',:;#@+#@@@@#,..........:#@'';''''#@#+'+##+.,'#@+.`.#+''';.  ,':..#+':  #..;+@#;..#+;'''';.   `.:@'''\n\
    .#,@. `` :'''''+@#'#+:... ,@@#++++#@'..........,+#,:'''@@+''#@;:'#  `:##,;'#+'''':``''..+#;,  :+`` `,##.:@.;;::;',   .`#',:\n\
    ::#;  . .''''''''@+@,...` #+#@''+'`;@:...........'@,,'##'''+@`   `'#` `+@';;#+''''''':..:#```  @`.  ..#;.@,`,;'''',   ..@''\n\
   `:;#`   `;'''''''''@+:...`'#  '@#;.+ ++`..........`.##''''''##      :@` `+#.:';+'''':....,@.;.  #,.` `,#;.@;''''''''.   `##'\n\
   `.@,.   ,''''''''''##....;@.   :@+`  `+..........`  :#@+,,:,@#       #,  `@+`` .';,`......@;'`  :+..`;@#,:@+'''''''';`  `.@'\n\
    ++    `''''''''''''@,...@;    #+:+:  #:.......`    .:.,+` '##'      +,   ;@.       `.....@:,`   @;;+;,,;#@'''''''''',    #+\n\
   .@,    :''''''''''''#'..;@     @+:    +;``....             @@@+#,    #.   .@:      `......@;;,   #,.. .'@@+'''''''''';`   ,@\n\
   +'`   .''''''''''''':#..+'     @'@'   +:`....`            `@:@@@##'` @`    #+     `.......#'',   ;'```'@#'';;'''''''''.    @\n\
  `@,    ;''''''''''''':#..#,     @,@##; #,....`             `@,:@@@@;:`@     ++    `........#',,   `#:+##+'';.'''''''''':  ` ; ******************************************************\n\
  ;#`   `''''''''''''';:#.,@`     @;;@@#+@,.....`            `@,.:@@+#@'+     ++    .........#;;,    @,';''''':'''''''''';` ` ` *                                                    *\n\
  #'`   ,''''''''''''''#+.,@      +#..@@#@.......             #'..'';;'@`     #+    `.`......@';     #''''''', ,''''''''''. `   *        Neko-server Application Server Library      *\n\
 `@; `  :''''''''''''';@,.;#      .@:,;+#+......`             .@;:;;;'@:      #;       ......@:   `` ##'''';``:''''''''''', `   *                    version 1.0b                    *\n\
 .@, `  ;'''''''''''';##..+'       +#;;;@.......`              :@#+'#@,               ......,@.   ,` +@''';,;''''''''''''': `   *                  by deusprogrammer                 *\n\
 :@. .` ;'''''''''''''@,...`       `@+;++ ``....      ..        `;++;`              `.......;@`,  :. '@'''''''';:;'''''''':   ` *                                                    *\n\
 ;@` ,``'''''''''''';'#.....        :@@#    ...`    ...``           `.      `       ........+#', `;, '@''''''';;'''''''''';` `` ******************************************************\n\
 ;#` :``'''''''''''':@;.....`        ``     ..`   ......`        `... ```         ``.......,@+'. ,', ;@'''''''#''''''''''''` ,.\n\
 ;#` :,`''''''''''''+@.........``              .`.......`      `.`.`..`  `   `.....``...,.,'@''` ;'. '@';''''''#''''''''''': ;.\n\
 ;#, ,';''''''''''''+#.........` `.`  `.. ``   #........`    ```...`   ```...``    ....,.,,+#''`,''; +@+'''''''++''''''''''' '.\n\
 :#;``''''''''''''''+@......................   #,.......`  `` ...`    `.``         ...,...,@'''''''':+@+''''''''#;''''''''';`'.\n\
 ,@'':,'''''';'''''''@,...................`    +,......`    `.`                    ``.....'#'''''''''#@+''''''';+@'''''''''::'.\n\
 `@;'''''''':''''''''#+..........``..`         ':...``                             `.....:@+'''''';''#@':''''''''@+'''''''''''`\n\
  @;'''''':.''':`,'+';@,........               ,;.``                               ......#@''''''''''@#,'''''''''+@'''''''''';.\n\
  +#'''''' ,';`  `'#''##........                `                                  `....:@''''''''''@@:'''''''''''@''''';,,''''\n\
  :@''''';  `    ,++'''#'........                                                 `.....#+.'''''''+@##''''''''''''@+'''''''''''\n\
  ,@+,''':       '#'''';@:.......`                                               `.....'@''''''''+@+''''''''''''''@+'''''''''''\n\
  `##'''',      .'@'''';'@,.......                        `,,                   ......,@+'''''''#@+'''''''''''''''##'''''''''''\n\
   .@''''`     `;'@'''':'##........                   `:+@@###                   .....##'''''''@@'''''''''''''''''##'''''''''''\n\
   `##':`      .''@'''''''##........             `.;+@@#':;#,@                  `....;@'''''''@#''''''''''''''''''##'''''''''''\n\
   ;'@';        .;#''''''''@#,......`        +#@@@#+;:,,,,,@:@                  ....,@+'''''+@#'''''''''''''''''''@#'''''''''''\n\
   ..;#.`        '#'''''''''##,......`        ````;,,,,,..,,#;                `.....+#'''''#@+''''''''''''''''''''@+''''''''';.\n\
      ':;        '#'''''''';;#@:......           `#',,.....#+                ......,@+'''+@@''''''''''''''''''''''@'''''::,..` \n\
       .,        ;@''''''';'';'@'......`          `@'.....+#`               .......#+'''+@#''''''''''''''''''''''#@'''',       \n\
                 ;@''''''''''';;@#,......          `##:,;@#`               `......;@'''#@+'''''''''''''''''''''''@+'':`        \n\
                 :@''''''''''''''#@;......           :###,               `........@+'#@#''''''''''''''''''''''''+@';.          \n\
                 `'+'''''''''''''';@+......`                            .........'#+@@+'''''''''''''''''''''''''@+,`           \n\
                  `@;''''''''''''';;#@:......                         ..........,@#@#''''''''''''''''''''''''''##',            \n\
                   ;#''''''''''''''''+@'......                      `..........,#@#'''''''''''''''''''''''''''+@';`            \n\
                    #+''''''''''''''+,'@#,.....`                  `..........,+@@,''''''''''''''''''''''''''''#';`"

#ifdef _WIN32
DWORD WINAPI SocketThread(LPVOID lpargs);
#else
void* SocketThread(void* lpargs);
#endif

#ifdef _WIN32
DWORD WINAPI ServerThread(LPVOID lpargs);
#else
void* ServerThread(void* lpargs);
#endif

void Easter() {
   printf("%s\n\n", NYAN2);
}

void PrintNekoBadge() {
   printf("%s\n", POWERED_BY);
   printf("%s\n\n", NYAN1);
}

enum HTTPProtocol {HTTP, HTTPS};

enum HTTPVerb {GET, PUT, POST, DELETE};

enum AppServerStatus {RUNNING, STOPPED, PAUSED};

struct HTTPQueryPair {
   char key[32], value[128];
};

class HTTPQueryString {
private:
   HTTPQueryPair pairs[64];
   int nPairs;
public:
   HTTPQueryString() {nPairs = 0;}
   ~HTTPQueryString() {}
   void add(char* key, char* value);
   char* operator[](char* key);
};

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

class HTTPRequest {
private:
   HTTPVerb verb;
   HTTPQueryString queryString;
   char resource[1024], httpVersion[64];
public:
   HTTPRequest() {}
   void init(char** tokens, int nTokens);
   HTTPVerb getVerb() {return verb;}
   HTTPQueryString getQueryString() {return queryString;}
   char* getResource() {return resource;}
   char* getHTTPVersion() {return httpVersion;}
};

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

class HTTPHeaderObject {
private:
   HTTPRequest httpRequest;
   unsigned int contentLength;
public:
   HTTPHeaderObject() {contentLength = 0;}
   void consumeLine(char* line);
   int getContentLength() {return contentLength;}
   HTTPRequest* getRequest() {return &httpRequest;}
};

void HTTPHeaderObject::consumeLine(char* line) {
   int nTokens;

   char** tokens = stringSplit(line, " :", &nTokens);
   
   if (stringEquals(tokens[0], "GET") || stringEquals(tokens[0], "PUT") || stringEquals(tokens[0], "POST") || stringEquals(tokens[0], "DELETE")) {
      httpRequest.init(tokens, nTokens);
   }
   else if (stringEquals(tokens[0], "Content-Length")) {
      contentLength = stringToInt(tokens[1]);
   }
   //More header items to come

   free(tokens);
}

struct WebService;
struct ApplicationServerArgs;
class ApplicationServer;

struct WebService {
   char resourceName[1024];
   void *(*callback)(Socket*, HTTPHeaderObject*, void*);
};

struct ApplicationServerArgs {
   Socket* sock;
   ApplicationServer* appServer;
};

class ApplicationServer {
protected:
   HTTPProtocol type;
   AppServerStatus status;
   char appName[32];
   char port[8];
   WebService getServices[128];
   WebService putServices[128];
   WebService postServices[128];
   WebService deleteServices[128];
   int nGetServices;
   int nPutServices;
   int nPostServices;
   int nDeleteServices;
public:
   ApplicationServer();
   ~ApplicationServer() {if (type == HTTPS) CleanUpSSL();}
   ApplicationServer(HTTPProtocol type, char* appName = "NekoServer", char* port = NULL);
   void addService(HTTPVerb verb, char* resourceName, void *(*funcPtr)(Socket*, HTTPHeaderObject*, void*));
   void start();
   void stop();

   AppServerStatus getStatus() {return status;}
   void setStatus(AppServerStatus status) {this->status = status;}
   char* getPort() {return port;}
   char* getAppName() {return appName;}
   HTTPProtocol getType() {return type;}

   WebService* fetchService(HTTPVerb verb, char* resourceName);

   bool isRunning() {return status == RUNNING;}

#ifdef _WIN32
   friend DWORD WINAPI ServerSocket(void* lpargs);
   friend DWORD WINAPI SocketSocket(void* lpargs);
#else
   friend void* ServerSocket(void* lpargs);
   friend void* SocketSocket(void* lpargs);
#endif
};

void ApplicationServer::start() {
   PrintNekoBadge();
   CreateThreadM(ServerThread, this);
}

void ApplicationServer::stop() {
   this->status = STOPPED;
}

ApplicationServer::ApplicationServer() {
   type = HTTP;
   strcpy(this->appName, "NekoServer");
   strcpy(this->port, "80");
   nGetServices = nPutServices = nPostServices = nDeleteServices = 0;
}

ApplicationServer::ApplicationServer(HTTPProtocol type, char* appName, char* port) {
   this->type = type;

   strcpy(this->appName, appName);

   if (port == NULL) {
      switch(type) {
      case HTTP:
         strcpy(this->port, "80"); 
         break;
      case HTTPS:
         InitSSL();
         strcpy(this->port, "443"); 
         break;
      }
   }
   else
      strcpy(this->port, port); 

   nGetServices = nPutServices = nPostServices = nDeleteServices = 0;
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
   ApplicationServerArgs* asa = (ApplicationServerArgs*)lpargs;
   Socket* client = asa->sock;
   ApplicationServer* appServer = asa->appServer;
   HTTPHeaderObject header;

   printf("Consuming HTTP header...\n");
   while ((nBytes = client->readLine(buffer, 1024)) > 2) {
      buffer[nBytes - 2] = 0;
      printf("HEADER: %s\n", buffer);
      header.consumeLine(buffer);
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

   printf("Exiting thread...\n\n");
   ExitThreadM(0);
}

#endif
