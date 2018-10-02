#include <stdio.h>

/*
mcomm.c - a simple in-memory version of comm - code from comm.c is reused.
-> used to track changes everytime a file is 'M' modified in inotify report (monitoring.c)

# Remove all line breaks, comments (#/<!--/...) -> sort the files
$ comm -3 -1 original modified  => remove common, supress from first file
$ comm -3 -2 original modified  => remove common, supress from second file
$ comm original modified        => required report below (visual comparison of changed lines)

        logger.simonAlarmLevel=debug
logger.simonAlarmLevel=info
        storeMissing = false
storeMissing = true
vfcenter.warn.threshold=80
        vfcenter.warn.threshold=8000

 * */
