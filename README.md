# imonitor
### Dynamic filesystem monitoring daemon

- imonitord: daemon that manages inotify watches on multiple directories:
	- add/remove/list any of the running watches.
	- spawns a worker thread that uses polling to read inotify events.

- imonitor: command line utility that talks to daemon via unix domain socket.

#### USAGE:

```
$ imonitor add /var/log
imonitord: [INFO] Watch added on /var/log | ID: 1

$ imonitor add /home/msalama/imonitor/src
imonitord: [INFO] Watch added on /home/msalama/imonitor/src | ID: 2

$ imonitor add /var
imonitord: [INFO] Watch added on /var | ID: 3

$ imonitor remove 2 
imonitord: [INFO] Watch removed on /home/msalama/imonitor/src | ID: 2

$ imonitor list
imonitord: [INFO] Watching ...
    👁️ ID:1 -> PATH:/var/log
    👁️ ID:3 -> PATH:/var
```

