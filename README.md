# imonitor
### Dynamic filesystem monitoring daemon

- imonitord: daemon that tracks inotify watches on multiple directories:
	- handles "add/remove/list" for the running watches.
	- spawns a parallel worker thread that uses polling to read inotify events\
and logs file system events inside watched directory (added/removed/modified) in the format:
```
M /var/log/some_daemon.log
A /var/log/added_file.txt
D /var/log/delete_me.log
```

- imonitor: command line utility to use along with the daemon via unix domain socket. 

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

