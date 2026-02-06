/* getifname.c -- network interface handling
 * Copyright(C) 1999,2000,2001 Salvatore Sanfilippo <antirez@invece.org>
 * Copyright(C) 2001 by Nicolas Jombart <Nicolas.Jombart@hsc.fr>
 * This code is under the GPL license */

/* BSD support thanks to Nicolas Jombart <Nicolas.Jombart@hsc.fr> */

/* $Id: getifname.c,v 1.3 2003/10/22 10:41:00 antirez Exp $ */

#include <stdio.h>		/* perror */
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>		/* struct sockaddr_in */
#include <arpa/inet.h>		/* inet_ntoa */
#include <net/if.h>
#include <unistd.h>		/* close */
#include <stdlib.h>

#include "hping2.h"
#include "globals.h"

#if !defined(__linux__)
#error Sorry, interface code not implemented.
#endif

/* Determine the output interface address for a given destination using
 * the kernel routing table.  Creates a UDP socket, connects it to the
 * destination (which doesn't send traffic), then reads back the local
 * address the kernel selected.  Returns 0 on success, -1 on error. */
int get_output_if(struct sockaddr_in *dest, struct sockaddr_in *ifip)
{
	int sd;
	struct sockaddr_in saddr;
	socklen_t slen = sizeof(saddr);

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == -1)
		return -1;
	/* Use an arbitrary port; connect() with UDP only sets the route. */
	dest->sin_port = htons(1025);
	if (connect(sd, (struct sockaddr *)dest, sizeof(*dest)) == -1) {
		close(sd);
		return -1;
	}
	if (getsockname(sd, (struct sockaddr *)&saddr, &slen) == -1) {
		close(sd);
		return -1;
	}
	close(sd);
	memcpy(ifip, &saddr, sizeof(saddr));
	return 0;
}

int get_if_name(void)
{
	int fd;
	struct ifconf	ifc;
	struct ifreq	ibuf[16],
			ifr,
			*ifrp,
			*ifend;
	struct sockaddr_in sa;
	struct sockaddr_in output_if_addr;
	int known_output_if = 0;

	/* Try to get the output interface address according to
	 * the OS routing table */
	if (ifname[0] == '\0') {
		if (get_output_if(&remote, &output_if_addr) == 0) {
			known_output_if = 1;
			if (opt_debug)
				printf("DEBUG: Output interface address: %s\n",
					inet_ntoa(sa.sin_addr));
		} else {
			fprintf(stderr, "Warning: Unable to guess the output "
					"interface\n");
		}
	}

	if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("[get_if_name] socket(AF_INET, SOCK_DGRAM, 0)");
		return -1;
	}

	memset(ibuf, 0, sizeof(struct ifreq)*16);
	ifc.ifc_len = sizeof ibuf;
	ifc.ifc_buf = (caddr_t) ibuf;

	/* gets interfaces list */
	if ( ioctl(fd, SIOCGIFCONF, (char*)&ifc) == -1 ||
	     ifc.ifc_len < sizeof(struct ifreq)		) {
		perror("[get_if_name] ioctl(SIOCGIFCONF)");
		close(fd);
		return -1;
	}

	/* ifrp points to buffer and ifend points to buffer's end */
	ifrp = ibuf;
	ifend = (struct ifreq*) ((char*)ibuf + ifc.ifc_len);

	for (; ifrp < ifend; ifrp++) {
		strlcpy(ifr.ifr_name, ifrp->ifr_name, sizeof(ifr.ifr_name));

		if ( ioctl(fd, SIOCGIFFLAGS, (char*)&ifr) == -1) {
			if (opt_debug)
				perror("DEBUG: [get_if_name] ioctl(SIOCGIFFLAGS)");
			continue;
		}

		if (opt_debug)
			printf("DEBUG: if %s: ", ifr.ifr_name);

		/* Down interface? */
		if ( !(ifr.ifr_flags & IFF_UP) )
		{
			if (opt_debug)
				printf("DOWN\n");
			continue;
		}

		if (known_output_if) {
			/* Get the interface address */
			if (ioctl(fd, SIOCGIFADDR, (char*)&ifr) == -1) {
				perror("[get_if_name] ioctl(SIOCGIFADDR)");
				continue;
			}
			/* Copy it */
			memcpy(&sa, &ifr.ifr_addr,
				sizeof(struct sockaddr_in));
			/* Check if it is what we are locking for */
			if (sa.sin_addr.s_addr !=
			    output_if_addr.sin_addr.s_addr) {
				if (opt_debug)
					printf("The address doesn't match\n");
				continue;
			}
		} else if (ifname[0] != '\0' && !strstr(ifr.ifr_name, ifname)) {
			if (opt_debug)
				printf("Don't Match (but seems to be UP)\n");
			continue;
		}

		if (opt_debug)
			printf("OK\n");

		/* interface found, save if name */
		strlcpy(ifname, ifr.ifr_name, 1024);

		/* get if address */
		if ( ioctl(fd, SIOCGIFADDR, (char*)&ifr) == -1) {
			perror("DEBUG: [get_if_name] ioctl(SIOCGIFADDR)");
			exit(1);
		}

		/* save if address */
		memcpy(&sa, &ifr.ifr_addr,
			sizeof(struct sockaddr_in));
		strlcpy(ifstraddr, inet_ntoa(sa.sin_addr), 1024);

		/* get if mtu */
		if ( ioctl(fd, SIOCGIFMTU, (char*)&ifr) == -1) {
			perror("Warning: [get_if_name] ioctl(SIOCGIFMTU)");
			fprintf(stderr, "Using a fixed MTU of 1500\n");
			h_if_mtu = 1500;
		}
		else
		{
			h_if_mtu = ifr.ifr_mtu;
		}
		close(fd);
		return 0;
	}
	/* interface not found, use 'lo' */
	strlcpy(ifname, "lo", 1024);
	strlcpy(ifstraddr, "127.0.0.1", 1024);
	h_if_mtu = 1500;

	close(fd);
	return 0;
}
