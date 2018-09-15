## Design
- ``` imonitord.c ``` Daemon to support dynamically adding/removing inotify watches on multiple directories/paths. 
- ``` imonitor.c``` CLI utility that interfaces with the daemon via a unix domain socket.
```		  
$ imonitor add [PATH]                      # print(now watching ${}).except(${} not found/${} already exists); 
$ imonitor remove [PATH]                   # except(watch does not exist);
$ imonitor list                            # Currently watching...
$ imonitor help                            # Prints help/synopsis
```
- Format for ```changes.log```, inpsired by subversion's command output for `svn status`:
```
++  application.properties;eai.soap.getHistory.url=$value
--  application.properties;eai.soap.getData.url=$value

A /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-new-stuff.ftl
M /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-number.ftl
D /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/deleteme.ftl	
```
## Component Implementation
- UNIX_SOCKET CLIENT/SERVER: [#DONE]
  - serialize/deserialize request_data
  - handle request depending on payload data -> dummy
- inotify/track watches + handle message from client, no need to fork children per request since daemon will use long polling on watches. 

### Challenges! 
- daemon will get slower handling of requests if number of watches are too large or polling is too frequent, how to scale?
- what type of reports needed per watch? how to separate reports per watch as well?
- how to actually generate above report!? 
  
## Pseudo for inotify!
```
// MONITORING application.properties:
OPENED..?
  GET(FD);
  ORIGINAL = READ();
  while(POLL_EACH_1s){
  WRITTEN?
    MODIFIED = READ_FROM_DISK();
  CLOSED?
    CLOSE(FD);
    INVOKE DIFF();
    break; 
  }
```
