/*
 * Copyright (c) 2003 RogerSeguin <roger_seguin@msn.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

static char     _devperf_id[] =
    "$Id: devperf.c,v 1.1 2003/10/06 23:45:15 rogerms Exp $";


#define DEBUG	0

#include "devperf.h"
#include "debug.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>


#define STATISTICS_FILE	"/proc/partitions"


int DevCheckStatAvailability ()
{
    FILE           *pF;
    char            acLine[256];
    int             status;

    pF = fopen (STATISTICS_FILE, "r");
    if (!pF)
	return (-errno);
    status = (((fgets (acLine, sizeof (acLine), pF))
	       && (strstr (acLine, "rsect"))) ? 0 : NO_EXTENDED_STATS);
    fclose (pF);
    return (status);
}				/* DevCheckStatAvailability() */


int DevGetPerfData (dev_t p_iDevice, struct devperf_t *p_poPerf)
{
    const int       iMajorNo = (p_iDevice >> 8) & 0xFF, /**/
	iMinorNo = p_iDevice & 0xFF;
    struct timeval  oTimeStamp;
    FILE           *pF;
    unsigned int    major, minor, ruse, wuse, use, rsect, wsect;
    int             c, n;

    pF = fopen (STATISTICS_FILE, "r");
    if (!pF)
	return (-1);
    while ((c = fgetc (pF)) && (c != '\n'));	/* Skip the header line */
    while ((n =
	    fscanf (pF,
		    "%u %u %*u %*s %*u %*u %u %u %*u %*u %u %u %*u %u %*u",
		    &major, &minor, &rsect, &ruse, &wsect, &wuse,
		    &use)) == 7)
	if ((major == iMajorNo) && (minor == iMinorNo)) {
	    fclose (pF);
	    gettimeofday (&oTimeStamp, 0);
	    p_poPerf->timestamp_ns =
		(uint64_t) 1000 *1000 * 1000 * oTimeStamp.tv_sec +
		1000 * oTimeStamp.tv_usec;
	    p_poPerf->wsect = wsect;
	    p_poPerf->rsect = rsect;
	    return (0);
	}
    fclose (pF);
    return (-1);
}				/* DevGetPerfData() */


/*
$Log: devperf.c,v $
Revision 1.1  2003/10/06 23:45:15  rogerms
Initial revision

Revision 1.2  2003/09/25 12:25:09  RogerSeguin
Implemented some error processing

Revision 1.1  2003/09/22 02:25:34  RogerSeguin
Initial revision

*/
