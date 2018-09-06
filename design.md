- motivation: one daemon to dynamically add/remove watches on multiple directories/paths
- action: invoke


:DESIGN:
- format for changes.properties, inpsired by subversion's command output for `svn status`
```
++ application.properties;eai.soap.getHistory.url=$value
--  application.properties;eai.soap.getData.url=$value

A /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-new-stuff.ftl
M /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/check-number.ftl
D /opt/web/app/tomcat/ecycc8/conf/cycc/web_templates/serviceportal/deleteme.ftl	
```
