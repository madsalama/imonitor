
#### NOTE: [ONLY MERGE WITH MAIN IFF TODO LIST IS #DONE]

### [TODO]

#### handle any valgrind warnings/errors as you go.   [OK] -> [22/9/2018]

1. cache watch_list; if watch_list not changed;
respond with cached result instead of going through the whole list again. (expensive strcat!)

3. implementation for polling inotify watches.
4. implementation for generating report with designed format.

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

