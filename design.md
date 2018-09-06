## Design
- ``` daemon.c ``` Daemon to support dynamically adding/removing inotify watches on multiple directories/paths. 
- ```$ monitor``` CLI utility that interfaces with the daemon via a unix domain socket.
```		  
$ monitor-cli [start|stop|restart]            # this will remove all running watches, sure?
$ monitor-ctl add-watch [PATH]                # print(now watching ${}).except(${} not found/${} already exists); 
$ monitor-ctl remove-watch [PATH]             # except(watch does not exist);
$ monitor-cli status                          # Currently watching...
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
- UNIX_SOCKET CLIENT/SERVER:
1. client <= send message to server
2. server <= inotify/track watches + handle message from client => fork watch handler


## Pseudo Implementation
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
