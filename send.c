/* 
 * $smu-mark$ 
 * $name: sendudp.c$ 
 * $author: Salvatore Sanfilippo <antirez@invece.org>$ 
 * $copyright: Copyright (C) 1999 by Salvatore Sanfilippo$ 
 * $license: This software is under GPL version 2 of license$ 
 * $date: Fri Nov  5 11:55:49 MET 1999$ 
 * $rev: 8$ 
 */ 

/* $Id: send.c,v 1.1.1.1 2003/08/31 17:23:53 antirez Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "hping2.h"
#include "globals.h"

static void select_next_random_source(void)
{
	unsigned char ra[4];

	ra[0] = hp_rand() & 0xFF;
	ra[1] = hp_rand() & 0xFF;
	ra[2] = hp_rand() & 0xFF;
	ra[3] = hp_rand() & 0xFF;
	memcpy(&local.sin_addr.s_addr, ra, 4);

	if (opt_debug)
		printf("DEBUG: the source address is %u.%u.%u.%u\n",
		    ra[0], ra[1], ra[2], ra[3]);
}

/* Pre-parsed --rand-dest template: is_random[i] true means octet i
 * gets a random value each send, otherwise fixed_val[i] is used. */
static int randdest_parsed = 0;
static int randdest_is_random[4];
static unsigned char randdest_fixed[4];

static void parse_rand_dest_template(void)
{
	char a[4], b[4], c[4], d[4];
	char *octets[4] = { a, b, c, d };
	int i;

	if (sscanf(targetname, "%4[^.].%4[^.].%4[^.].%4[^.]", a, b, c, d) != 4)
	{
		fprintf(stderr,
			"wrong --rand-dest target host, correct examples:\n"
			"  x.x.x.x, 192,168.x.x, 128.x.x.255\n"
			"you typed: %s\n", targetname);
		exit(1);
	}
	a[3] = b[3] = c[3] = d[3] = '\0';
	for (i = 0; i < 4; i++) {
		randdest_is_random[i] = (octets[i][0] == 'x');
		randdest_fixed[i] = randdest_is_random[i] ?
			0 : strtoul(octets[i], NULL, 0);
	}
	randdest_parsed = 1;
}

static void select_next_random_dest(void)
{
	unsigned char ra[4];
	int i;

	if (!randdest_parsed)
		parse_rand_dest_template();

	for (i = 0; i < 4; i++)
		ra[i] = randdest_is_random[i] ?
			(hp_rand() & 0xFF) : randdest_fixed[i];
	memcpy(&remote.sin_addr.s_addr, ra, 4);

	if (opt_debug) {
		printf("DEBUG: the dest address is %u.%u.%u.%u\n",
				ra[0], ra[1], ra[2], ra[3]);
	}
}

/* The signal handler for SIGALRM will send the packets */
void send_packet (int signal_id)
{
	int errno_save = errno;

	if (opt_rand_dest)
		select_next_random_dest();
	if (opt_rand_source)
		select_next_random_source();

	if (opt_rawipmode)	send_rawip();
	else if (opt_icmpmode)	send_icmp();
	else if (opt_udpmode)	send_udp();
	else			send_tcp();

	sent_pkt++;
	Signal(SIGALRM, send_packet);

	if (count != -1 && count == sent_pkt) { /* count reached? */
		Signal(SIGALRM, print_statistics);
		alarm(COUNTREACHED_TIMEOUT);
	} else if (!opt_listenmode) {
		if (opt_waitinusec == FALSE)
			alarm(sending_wait);
		else
			setitimer(ITIMER_REAL, &usec_delay, NULL);
	}
	errno = errno_save;
}
