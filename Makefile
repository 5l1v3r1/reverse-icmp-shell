CC = gcc
CFLAGS1 = -O2 -Wall
CFLAGS2 = -O2 -Wall -lsocket
STRIP = strip

default:
	@echo "-------------------------------"
	@echo "Make with the OS from the list:"
	@echo ""
	@echo "1.) linux"
	@echo "2.) bsd"
	@echo "3.) solaris"
	@echo ""
	@echo "ex: make bsd"
	@echo "-------------------------------"


clean:
	/bin/rm -f client server

linux:	clean cc1 fin

bsd:	clean cc1 fin

solaris:	clean cc2 fin

cc1:
	$(CC) $(CFLAGS1) -o client client.c core.c
	$(CC) $(CFLAGS1) -o server server.c core.c popen2.c

cc2:
	$(CC) $(CFLAGS2) -o client client.c core.c
	$(CC) $(CFLAGS2) -o server server.c core.c popen2.c

fin:
	$(STRIP) client
	$(STRIP) server

