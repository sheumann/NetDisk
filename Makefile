CC = occ
CFLAGS = -w-1 -O-1

HTTPTEST_OBJS = httptest.a hostname.a http.a readtcp.a seturl.a strcasecmp.a tcpconnection.a urlparser.a
HTTPTEST_PROG = httptest

NETDISKINIT_OBJS = initstart.a netdiskinit.a hostname.a http.a readtcp.a seturl.a strcasecmp.a tcpconnection.a urlparser.a
# NETDISKINIT_RSRC = 
NETDISKINIT_PROG = NetDiskInit

# NETDISKCDEV_OBJS = 
# NETDISKCDEV_RSRC = 
# NETDISKCDEV_CDEV = 

PROGS = $(HTTPTEST_PROG) $(NETDISKINIT_PROG)

.PHONY: default
default: $(PROGS)

$(HTTPTEST_PROG): $(HTTPTEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(NETDISKINIT_PROG): $(NETDISKINIT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	iix chtyp -tpif $@

.PHONY: clean
clean:
	rm -f $(PROGS) *.a *.o *.root *.A *.O *.ROOT

%.a: %.c *.h
	$(CC) $(CFLAGS) -c $<

%.a: %.asm
	$(CC) $(CFLAGS) -c $<
