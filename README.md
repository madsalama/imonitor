# imonitor
### Dynamic filesystem monitoring daemon

- *imonitor*: command line utility that sends requests to daemon via unix domain socket.
- *imonitord*: daemon that tracks inotify watches on multiple directories:
	- handles requests from imonitor: "add/remove/list"
	- spawns one parallel worker thread that uses polling to keep reading new inotify events\
and logs filesystem events inside watched directory (added/removed/modified) in the format:
```
M /var/log/some_daemon.log
A /var/log/added_file.txt
D /var/log/delete_me.log
```

#### USAGE:

```
$ imonitor add /var/log
imonitord: [INFO] Watch added on /var/log | ID: 1

$ imonitor add /home/msalama/imonitor/src
imonitord: [INFO] Watch added on /home/msalama/imonitor/src | ID: 2

$ imonitor add /var
imonitord: [INFO] Watch added on /var | ID: 3

$ imonitor remove /var
imonitord: [INFO] Watch removed on /home/msalama/imonitor/src | ID: 3

$ imonitor list
imonitord: [INFO] Watching ...
    👁️ ID:1 -> PATH:/var/log
    👁️ ID:2 -> PATH:/home/msalama/imonitor/src 
```

