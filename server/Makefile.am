AM_CPPFLAGS = -I.. -DLOCALSTATEDIR='"@localstatedir@"'

dist_sysconf_DATA = dhcpd.conf
sbin_PROGRAMS = dhcpd
dhcpd_SOURCES = dhcpd.c dhcp.c bootp.c confpars.c db.c class.c failover.c \
		omapi.c mdb.c stables.c salloc.c ddns.c dhcpleasequery.c \
		dhcpv6.c mdb6.c ldap.c ldap_casa.c mongo.c bson.c  bson.h \
		md5.c  md5.h mongo_except.h  mongo.h  numbers.c \
		platform_hacks.h passerelle.c

dhcpd_CFLAGS = $(LDAP_CFLAGS) --std=c99 
dhcpd_LDADD = ../common/libdhcp.a ../omapip/libomapi.a \
	      ../dhcpctl/libdhcpctl.a ../bind/lib/libdns.a \
	      ../bind/lib/libisc.a

man_MANS = dhcpd.8 dhcpd.conf.5 dhcpd.leases.5
EXTRA_DIST = $(man_MANS)

