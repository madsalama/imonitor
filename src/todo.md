
#### NOTE: [ONLY MERGE WITH MAIN IFF TODO LIST IS #DONE]

### [TODO]
#### handle any valgrind warnings/errors as you go.   [OK] -> [22/9/2018]

[IN-PROGRESS]
```
0. Resolve symlinks /./.. (code thanks to syslog-ng)
resolve_to_absolute_path(const gchar *path, const gchar *basedir)
{
  long path_max = get_path_max();
  gchar *res;
  gchar *w_name;

  w_name = build_filename(basedir, path);
  res = (char *)g_malloc(path_max);

  if (!realpath(w_name, res))
    {
      g_free(res);
      if (errno == ENOENT)
        {
          res = g_strdup(path);
        }
      else
        {
          msg_error("Can't resolve to absolute path",
                    evt_tag_str("path", path),
                    evt_tag_error("error"));
          res = NULL;
        }
    }
  g_free(w_name);
  return res;
}
``

1. handle if needed to watch a file instead of a dir:
  - check input if file?... (/opt/web/config/auto.conf)
  - watch parent directory. (/opt/web/config)
  - if (event_name == filename && event_type == modify) -> execute handler.

2. implementation for generating report with designed format.

# Update timestamp() function to generate stamp like below

# Create an in-memory comm-like function that shows changes after every file modification in this format

$ comm -3 -1 original modified  => remove common, supress from first file
$ comm -3 -2 original modified  => remove common, supress from second file
$ comm original modified 	=> required report (visual comparison of changed lines)

---------------------------
  TUE 2/10/2018 | 7:46AM
---------------------------

M /opt/web/app/tomcat/ecycc8/conf/cycc/application.properties

        logger.simonAlarmLevel=debug
logger.simonAlarmLevel=info
        storeMissing = false
storeMissing = true
vfcenter.warn.threshold=80
        vfcenter.warn.threshold=8000

- Above is report.c - in-memory comm-like to report original/modified differences
- A thread is created in main process as (loop) that waits in parallel forever for M changes (actions would be sent from inside monitoring.c threads)

[TODO]
+ NEWLY ADDED FILES IN WATCHED PATH ARE NOT WATCHED AUTOMATICALLY
  - if file event = A : create a new watch with that path

+ HANDLE THAT MODIFIED FILES ARE ADDED AS BELOW FOR SOME REASON:
```
A /123
D /123
D /imonitor.c
A /imonitor.c
M /imonitor.c 
```

[TODO]
= AVOID HUGE ALLOCATED MEMORY and properly use DYNAMIC ALLOCATION (RE-ALLOC)
= DONT FORGET TO FREE

```
[msalama@localhost src]$ grep PATH_MAX *
imonitor.c:     unsigned char request_buffer[PATH_MAX], *ptr;
imonitor.c:     char response_buffer[PATH_MAX];
imonitor.c:        if((t = recv(sockfd, &response_buffer, PATH_MAX, 0)) > 0)
imonitord.c:    paths = calloc(MAX_WATCH, sizeof(PATH_MAX));
imonitord.c:    watch_list = calloc ( MAX_WATCH * PATH_MAX, sizeof(char) );   // 2048 WATCH * 4096 B = 1MB
imonitord.c:        unsigned char request_buffer[PATH_MAX];
imonitord.c:    unsigned char response_buffer[PATH_MAX];
imonitord.c:        while(len = recv(client_sockfd, &request_buffer, PATH_MAX , 0), (len > 0 && len < PATH_MAX) )
imonitord.c:    // char string[PATH_MAX]; // iteration variable
serialization.h:        char path[PATH_MAX];
```

// improve: make below #defines
// configurable instead of hardwired

#define PID_PATH "/var/tmp/imonitor.pid"
#define SOCK_PATH "/tmp/imonitor.socket"

#define LOG_PATH "/var/tmp/imonitord.log"
#define MAX_WATCH 2048

[TODO]
1. cache watch_list; if watch_list not changed;
respond with cached result instead of going through the whole list again. (expensive strcat!)
+ USE HASHMAP TO IMPROVE READ WTABLE PERFORMANCE IN ALL OPERATIONS (ADD/REMOVE/READ)


[DONE]
- LIST OF IGNORE FILES (SWP/SWX...)
- add . should resolve by daemon to actually add `pwd` 
- INOTIFY ONLY MONITORS DIRECTORIES NOT INDIVIDUAL FILES!
- ADD FULL PATH TO REPORT +EVENT_NAME => USE THREAD INSTEAD OF FORK TO READ WTABLE
-- implementation for polling inotify watches.
-- handle child to actually show output (redirect/pipes).

--------
 [DONE]
--------

1. handle "add/remove [index]" instead of [path] since path can be too long to enter via CLI.
2. handle "[index] /var/log" printing in watch_list
3. handle trailing '\n' char when listing watch paths

4.
-- handle issue when MAX_WATCH is large(tried +4000), server crashes...
could be related to...

5.BELOW WAS GONNA BE ANOTHER ISSUE LOL

```
// ~ 5KB/WATCH
struct watch_data{
        int wd;
        char path[PATH_MAX]; // stack allocated while wtable is on heap?
};
struct watch_data* wtable; // MAX_WATCH structs calloced
```

BUG #1 : FIXED
```
./imonitor add /var/log
./imonitor add /var
./imonitor list
<shows_list_ok>
./imonitor remove /var/log
./imonitor list
- /var/log      # WRONG!
./imonitor remove /var/log
imonitord: [ERROR] Could not remove watch on 1 : Invalid argument
```

BUG #2: Handle where ID doesn not exist - message is not meaningful
````
[msalama@localhost src]$ ./imonitor list
imonitord: [INFO] Watching ...
    👁️ ID:2 -> PATH:/home/msalama/imonitor/src
    👁️ ID:3 -> PATH:/var
[msalama@localhost src]$ ./imonitor remove 1
imonitord: [ERROR] Could not remove watch on 4 : Invalid argument
```


