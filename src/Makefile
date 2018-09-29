
# gcc imonitord.c serialization.c monitoring.c -o imonitord -pthread
# gcc imonitor.c serialization.c -o imonitor

CC=gcc
CFLAGS=-pthread
RM=/usr/bin/rm -f

serialization.o: serialization.c
	$(CC) -c serialization.c -o serialization.o

monitoring.o: monitoring.c
	$(CC) -c monitoring.c -o monitoring.o

imonitor:  imonitor.c serialization.o
	$(CC) imonitor.c serialization.o -o imonitor

imonitord: imonitord.c serialization.o monitoring.o
	$(CC) $(CFLAGS) imonitord.c serialization.o monitoring.o -o imonitord

clean:
	${RM} *.o imonitord imonitor

