#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "simSock.h"
#include "strutils.h"
#include "thread.h"
#include "timer.h"

#ifndef NEKOSERVER_H
#define NEKOSERVER_H

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

class HTTPHeaderObject {
private:
   HTTPRequest httpRequest;
   unsigned int contentLength;
   bool malformed;
public:
   HTTPHeaderObject() {contentLength = 0; malformed = true;}
   void consumeLine(char* line);
   bool isMalformed() {return malformed;}
   int getContentLength() {return contentLength;}
   HTTPRequest* getRequest() {return &httpRequest;}
};

struct WebService;
struct ApplicationServerArgs;
class ApplicationServer;

struct WebService {
   char resourceName[128];
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
   int maxConnections;
   int nConnections;

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
   char* getPort() {return port;}
   char* getAppName() {return appName;}
   HTTPProtocol getType() {return type;}

   WebService* fetchService(HTTPVerb verb, char* resourceName);

   bool isRunning() {return status == RUNNING;}
};
#endif
