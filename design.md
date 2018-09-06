## Design
- Motivation: one daemon to support dynamically adding/removing inotify watches on multiple directories/paths, multiple users can do this through same daemon through a CLI utility ```$ monitor-cli``` that interfaces with the daemon via a unix domain socket.

- Actionable: invoke a shell script to log changes instead of manually making lists about file changes.
- format for changes.properties, inpsired by subversion's command output for `svn status`
```
++  application.properties;eai.soap.getHistory.url=$value
--  application.properties;eai.soap.getData.url=$value

A /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-new-stuff.ftl
M /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-number.ftl
D /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/deleteme.ftl	
```
