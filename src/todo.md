
NOTE: [ONLY MERGE WITH MAIN IFF TODO LIST IS #DONE]

--------
 [TODO]
--------

-0- handle any valgrind warnings/errors as you go.
-1- handle trailing '\n' char when listing watch paths
-2- make allocated watch_list an actual list (array?) then deserialize it back to client instead of long string. 
-3- cache watch_list; if watch_list not changed;
respond with cached result instead of going through the whole list again. (expensive strcat!)

-4- handle "add/remove [index]" instead of [path] since path can be too long to enter via CLI.
-5- handle "[index] /var/log" printing in watch_list

-6- implementation for handling polling inotify watches.
-7- implementation to handle the required report formatting according to watch events.

--------
 [DONE]
--------

-- handle issue when MAX_WATCH is large(tried +4000), server crashes...
could be related to...

[BELOW WAS GONNA BE ANOTHER ISSUE LOL]
// ~ 5KB/WATCH
struct watch_data{
        int wd;
        char path[PATH_MAX]; // stack allocated while wtable is on heap?
};

struct watch_data* wtable; // MAX_WATCH structs calloced

----------------
 BUG #1 : FIXED
----------------
./imonitor add /var/log
./imonitor add /var
./imonitor list
<shows_list_ok>

./imonitor remove /var/log

./imonitor list
- /var/log      # WRONG!

./imonitor remove /var/log
imonitord: [ERROR] Could not remove watch on 1 : Invalid argument
