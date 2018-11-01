# imonitor
### Dynamic filesystem monitoring daemon (experimental)

- **imonitor**: command line utility that sends requests to daemon via unix domain socket.
- **imonitord**: daemon that tracks inotify watches on multiple directories:
	- handles requests from imonitor: "add/remove/list"
	- spawns one parallel worker thread that uses polling to keep reading new inotify events\
and logs filesystem events inside watched directory (added/removed/modified) in the format:

```
Fri Sep 28 10:08:01 2018
D /home/msalama/imonitor/src/to_delete.txt

Sat Sep 29 13:06:21 2018
A /home/msalama/imonitor/src/new_file.txt

Sat Sep 29 13:06:33 2018
M /home/msalama/imonitor/src/modified.txt

```
- Requires ``inotify`` = linux kernel > 2.6.13

#### USAGE:

```
$ imonitor add /var/log
imonitord: [INFO] Watch added on /var/log | ID: 1

$ imonitor add /home/msalama/imonitor/src
imonitord: [INFO] Watch added on /home/msalama/imonitor/src | ID: 2

$ imonitor list
imonitord: [INFO] Watching ...
    👁️ ID:1 -> PATH:/var/log
    👁️ ID:2 -> PATH:/home/msalama/imonitor/src
```
