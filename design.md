## Design
- One daemon to support dynamically adding/removing inotify watches on multiple directories/paths, multiple users can do this through the same daemon using a CLI utility ```$ monitor-cli``` that interfaces with the daemon via a unix domain socket.

- Invoke a shellscript ```diff_script.sh >> changes.log``` to log actual file changes. No need to reinvent ```diff``` in C. 
- Format for ```changes.log```, inpsired by subversion's command output for `svn status`:
```
++  application.properties;eai.soap.getHistory.url=$value
--  application.properties;eai.soap.getData.url=$value

A /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-new-stuff.ftl
M /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-number.ftl
D /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/deleteme.ftl	
```
