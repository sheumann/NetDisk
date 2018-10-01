CC = occ
CFLAGS = -w-1 -O-1

HTTPTEST_OBJS = httptest.a hostname.a http.a readtcp.a seturl.a strcasecmp.a tcpconnection.a urlparser.a
HTTPTEST_PROG = httptest

MOUNTURL_OBJS = mounturl.a
MOUNTURL_PROG = mounturl

NETDISKINIT_OBJS = initstart.a netdiskinit.a hostname.a http.a readtcp.a seturl.a strcasecmp.a tcpconnection.a urlparser.a driver.a installdriver.a asmglue.a driverwrapper.a session.a systemservices.a
# NETDISKINIT_RSRC = 
NETDISKINIT_PROG = NetDiskInit

NETDISKCDEV_OBJS = cdev.a cdevutil.a netdiskerror.a strcasecmp.a
NETDISKCDEV_RSRC = cdev.rez
NETDISKCDEV_CDEV = NetDisk

MACROS = asmglue.macros

PROGS = $(HTTPTEST_PROG) $(NETDISKINIT_PROG) $(MOUNTURL_PROG) $(NETDISKCDEV_CDEV)

.PHONY: default
default: $(PROGS)

$(HTTPTEST_PROG): $(HTTPTEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(MOUNTURL_PROG): $(MOUNTURL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(NETDISKINIT_PROG): $(NETDISKINIT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	iix chtyp -tpif $@

cdev.obj: $(NETDISKCDEV_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(NETDISKCDEV_CDEV): cdev.obj $(NETDISKCDEV_RSRC)
	occ $(NETDISKCDEV_RSRC) -o $@
	iix chtyp -tcdv $@

%.macros: %.asm
	iix macgen $< $@ 13/ORCAInclude/m16.= < /dev/null > /dev/null

.PHONY: macros
macros: $(MACROS)

.PHONY: clean
clean:
	rm -f $(PROGS) *.a *.o *.root *.A *.O *.ROOT

%.a: %.c *.h
	$(CC) $(CFLAGS) -c $<

%.a: %.asm
	$(CC) $(CFLAGS) -c $<
