#ifndef NEKOSERVER_H
#define NEKOSERVER_H

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <map>
#include <list>
#include <string>
#include <utility>
#include <iostream>

#include "simSock.h"
#include "strutils.h"
#include "thread.h"
#include "timer.h"

using namespace std;

#ifndef SOCKET
#define SOCKET int
#endif

#define TIMEOUT 1000

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

void Easter();
void PrintNekoBadge();
void signalHandler(int sigNum);

enum HTTPProtocol {HTTP, HTTPS};
enum HTTPVerb {GET, PUT, POST, DELETE};
enum AppServerStatus {RUNNING, STOPPED, PAUSED};

class HTTPRequest {
private:
   HTTPVerb verb;
   map<string, string> queryStringMap;
   char resource[1024], httpVersion[64];
public:
   HTTPRequest() {printf("HTTPRequest::CREATED %u\n", this);}
   ~HTTPRequest() {printf("HTTPRequest::DELETED %u\n", this);}
   void init(char* verb, char* resource, char* httpVersion);
   HTTPVerb getVerb() {return verb;}
   char* getResource() {return resource;}
   char* getHTTPVersion() {return httpVersion;}
   char* getQueryStringItem(char* key) {return (char*)queryStringMap[key].c_str();}
   char* operator[](char* key) {return (char*)queryStringMap[key].c_str();}
};

class HTTPHeaderObject {
private:
   HTTPRequest httpRequest;
   unsigned int contentLength;
   map<string, string> headerInfo;

   bool malformed;
public:
   HTTPHeaderObject() {printf("HTTPHeaderObject::CREATED %u\n", this); contentLength = 0; malformed = true;}
   ~HTTPHeaderObject() {printf("HTTPHeaderObject::DELETED %u\n", this);}
   void consumeLine(char* line);
   bool isMalformed() {return malformed;}
   int getContentLength() {return contentLength;}
   HTTPRequest* getRequest() {return &httpRequest;}
   char* operator[](char* key) {return (char*)headerInfo[key].c_str();}
};

struct WebService;
struct ApplicationServerArgs;
class ApplicationServer;

struct WebService {
   char resourceName[128];
   void *(*callback)(Socket*, HTTPHeaderObject*, void*);

   WebService(void *(*callback)(Socket*, HTTPHeaderObject*, void*));
};

struct ApplicationServerArgs {
   Socket* sock;
   ApplicationServer* appServer;
   
   ApplicationServerArgs() {printf("ApplicationServerArgs::CREATED %u\n", this);}
   ~ApplicationServerArgs() {printf("ApplicationServerArgs::DELETED %u\n", this);}
};

class ApplicationServer {
protected:
   HTTPProtocol type;
   AppServerStatus status;
   char appName[32];
   char port[8];
   char htdocsDirectory[1024];
   int nConnections;
   int maxConnections;

   char certificatePath[1024];
   char privateKeyPath[1024];

   map<string, WebService*> getServices;
   map<string, WebService*> putServices;
   map<string, WebService*> postServices;
   map<string, WebService*> deleteServices;

   int nGetServices;
   int nPutServices;
   int nPostServices;
   int nDeleteServices;

   static bool shutdown;
   static int nThreads;
   static int nSignals;
public:
   ApplicationServer();
   ~ApplicationServer();
   ApplicationServer(HTTPProtocol type, char* appName = "NekoServer", int maxConnections = 1000, char* port = NULL);
   void addService(HTTPVerb verb, char* resourceName, void *(*funcPtr)(Socket*, HTTPHeaderObject*, void*));
   void start();
   void stop();

   void incrementConnectionCount() {nConnections++;}
   void decrementConnectionCount() {nConnections--;}
   int getAvailableConnections() {return maxConnections - nConnections;}

   AppServerStatus getStatus() {return status;}
   void setStatus(AppServerStatus status) {this->status = status;}
   void setHtdocsDirectory(char* htdocsDirectory);
   char* getHtdocsDirectory();
   char* getPort() {return port;}
   char* getAppName() {return appName;}
   HTTPProtocol getType() {return type;}

   void setCertificatePath(char* certPath) {strcpy(this->certificatePath, certPath);}
   void setPrivateKeyPath(char* keyPath) {strcpy(this->privateKeyPath, keyPath);}
   char* getCertificatePath() {return this->certificatePath;}
   char* getPrivateKeyPath() {return this->privateKeyPath;}

   WebService* fetchService(HTTPVerb verb, char* resourceName);

   bool isRunning() {return status == RUNNING;}

   static void signalHandler(int sigNum);
   static bool emergencyShutdown() {return shutdown;}
   static void incrementThreadCount() {ApplicationServer::nThreads++;}
   static void decrementThreadCount() {ApplicationServer::nThreads--;}
   static int getThreadCount() {return ApplicationServer::nThreads;}
};

#endif
