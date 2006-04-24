/*
 * Copyright (c) 2003 RogerSeguin <roger_seguin@msn.com>
 * Copyright (c) 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
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


#include "devperf.h"


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


#if defined(__linux__)
	/**************************************************************/
	/**************************	Linux	***********************/
	/**************************************************************/

static const char STATISTICS_FILE_1[] = "/proc/diskstats";	/* Kernel
								   2.6 */
static const char STATISTICS_FILE_2[] = "/proc/partitions";	/* Kernel
								   2.4 */

static const uint64_t SECTOR_SIZE = 512;

static int      m_iInitStatus = 0;
static const char *m_pcStatFile = 0;

typedef int     (*GetPerfData_t) (dev_t dev, struct devperf_t * perf);

static GetPerfData_t m_mGetPerfData = 0;

	/**************************************************************/

static int DevGetPerfData1 (dev_t p_iDevice, struct devperf_t *p_poPerf)
	/* Get disk performance statistics from STATISTICS_FILE_1 */
{
    const int       iMajorNo = (p_iDevice >> 8) & 0xFF, /**/
	iMinorNo = p_iDevice & 0xFF;
    struct timeval  oTimeStamp;
    FILE           *pF;
    unsigned int    major, minor, rsect, wsect, ruse, wuse, use;
    int             running;
    char            acStats[128];
    int             c, n;

    pF = fopen (STATISTICS_FILE_1, "r");
    if (!pF) {
	perror (STATISTICS_FILE_1);
	return (-1);
    }
    while (1) {
	n = fscanf (pF, "%u %u", &major, &minor);
	if (n != 2)
	    goto Error;
	if ((major != iMajorNo) || (minor != iMinorNo)) {
	    while ((c = fgetc (pF)) && (c != '\n'));	/* Goto next line */
	    continue;
	}
	fscanf (pF, "%*s");	/* Skip device name */
	/* Read rest of line into acStats */
	if (!(fgets (acStats, sizeof (acStats), pF)))
	    goto Error;
	n = sscanf (acStats,
		    "%*u %*u %u %u %*u %*u %u %u %d %u %*u",
		    &rsect, &ruse, &wsect, &wuse, &running, &use);
	if (n != 6) {
	    /* Not a full-statistics line */
	    n = sscanf (acStats, "%*u %u %*u %u", &rsect, &wsect);
	    if (n != 2)
		goto Error;
	    running = -1, ruse = wuse = 0;
	}
	fclose (pF);
	gettimeofday (&oTimeStamp, 0);
	p_poPerf->timestamp_ns =
	    (uint64_t) 1000 *1000 * 1000 * oTimeStamp.tv_sec +
	    1000 * oTimeStamp.tv_usec;
	p_poPerf->rbytes = SECTOR_SIZE * rsect;
	p_poPerf->wbytes = SECTOR_SIZE * wsect;
	p_poPerf->qlen = running;
	p_poPerf->rbusy_ns = (uint64_t) 1000 *1000 * ruse;
	p_poPerf->wbusy_ns = (uint64_t) 1000 *1000 * wuse;
	return (0);
    }
  Error:
    fclose (pF);
    return (-1);
}				/* DevGetPerfData1() */


static int DevGetPerfData2 (dev_t p_iDevice, struct devperf_t *p_poPerf)
	/* Get disk performance statistics from STATISTICS_FILE_2 */
{
    const int       iMajorNo = (p_iDevice >> 8) & 0xFF, /**/
	iMinorNo = p_iDevice & 0xFF;
    struct timeval  oTimeStamp;
    FILE           *pF;
    unsigned int    major, minor, rsect, wsect, ruse, wuse, use;
    int             running;
    int             c, n;

    pF = fopen (STATISTICS_FILE_2, "r");
    if (!pF) {
	perror (STATISTICS_FILE_2);
	return (-1);
    }
    while ((c = fgetc (pF)) && (c != '\n'));	/* Skip the header line */
    while ((n = fscanf (pF,
			"%u %u %*u %*s %*u %*u %u %u %*u %*u %u %u %d %u %*u",
			&major, &minor, &rsect, &ruse, &wsect,
			&wuse, &running, &use)) == 8)
	if ((major == iMajorNo) && (minor == iMinorNo)) {
	    fclose (pF);
	    gettimeofday (&oTimeStamp, 0);
	    p_poPerf->timestamp_ns =
		(uint64_t) 1000 *1000 * 1000 * oTimeStamp.tv_sec +
		1000 * oTimeStamp.tv_usec;
	    p_poPerf->rbytes = SECTOR_SIZE * rsect;
	    p_poPerf->wbytes = SECTOR_SIZE * wsect;
	    p_poPerf->qlen = running;
	    p_poPerf->rbusy_ns = (uint64_t) 1000 *1000 * ruse;
	    p_poPerf->wbusy_ns = (uint64_t) 1000 *1000 * wuse;
	    return (0);
	}
    fclose (pF);
    return (-1);
}				/* DevGetPerfData2() */

	/**************************************************************/

int DevPerfInit ()
{
    FILE           *pF = 0;
    char            acLine[256];

    /* Kernel 2.6 ? */
    m_pcStatFile = STATISTICS_FILE_1;
    m_mGetPerfData = DevGetPerfData1;
    pF = fopen (m_pcStatFile, "r");
    m_iInitStatus = 0;
    if (pF)
	goto End;

    /* Kernel 2.4 */
    m_pcStatFile = STATISTICS_FILE_2;
    m_mGetPerfData = DevGetPerfData2;
    pF = fopen (m_pcStatFile, "r");
    if (pF)
	m_iInitStatus = (((fgets (acLine, sizeof (acLine), pF))
			  && (strstr (acLine, "rsect"))) ? 0 :
			 NO_EXTENDED_STATS);
    else
	m_iInitStatus = -errno;

  End:
    if (pF)
	fclose (pF);
    return (m_iInitStatus);
}				/* DevPerfInit() */


int DevCheckStatAvailability (char const **p_ppcStatFile)
{
    if (p_ppcStatFile)
	*p_ppcStatFile = m_pcStatFile;
    return (m_iInitStatus);
}				/* DevCheckStatAvailability() */


int DevGetPerfData (const void *p_pvDevice, struct devperf_t *p_poPerf)
{
    const dev_t     p_iDevice = *((dev_t *) p_pvDevice);
    return ((m_mGetPerfData && !m_iInitStatus) ?
	    (*m_mGetPerfData) (p_iDevice, p_poPerf) : -1);
}				/* DevGetPerfData() */

	/**************************************************************/

#if 0				/* Standalone test purpose */
int main ()
{
    int             iMajor = 3, /**/ iMinor = 3;
    dev_t           iDev = (iMajor << 8) + iMinor;
    struct devperf_t oPerf;
    unsigned int    rsect, wsect;
    int             status;

    status = DevPerfInit ();
    if (status)
	fprintf (stderr, "DevPerfInit() error\n");
    status = DevGetPerfData (&iDev, &oPerf);
    if (status)
	fprintf (stderr, "DevGetPerfData() error\n");
    rsect = oPerf.rbytes / SECTOR_SIZE;
    wsect = oPerf.wbytes / SECTOR_SIZE;
    printf ("%u\t%u\n", rsect, wsect);
    return (0);
}
#endif

	/**************************	Linux End	***************/


#elif defined(__NetBSD__)
	/**************************************************************/
	/**************************	NetBSD	***********************/
	/**************************************************************/
/* *INDENT-OFF* */

#include <sys/disk.h>
#include <sys/param.h>
#include <sys/sysctl.h>

int DevPerfInit ()
{
	return (0);
}

int DevCheckStatAvailability(char const **strptr)
{
	return (0);
}

int DevGetPerfData (const void *p_pvDevice, struct devperf_t *perf)
{
	const char     *device = (const char *) p_pvDevice;
	struct timeval tv;
	size_t size, i, ndrives;
	struct disk_sysctl *drives, drive;
	int mib[3];

	mib[0] = CTL_HW;
	mib[1] = HW_DISKSTATS;
	mib[2] = sizeof(struct disk_sysctl);
	if (sysctl(mib, 3, NULL, &size, NULL, 0) == -1)
		return(-1);
	ndrives = size / sizeof(struct disk_sysctl);
	drives = malloc(size);
	if (sysctl(mib, 3, drives, &size, NULL, 0) == -1)
		return(-1);

	for (i = 0; i < ndrives; i++) {
		if (strcmp(drives[i].dk_name, device) == 0) {
			drive = drives[i];
			break;
		}
	}

	free(drives);

	if (i == ndrives)
		return(-1);

	gettimeofday (&tv, 0);
	perf->timestamp_ns = (uint64_t)1000ull * 1000ull * 1000ull *
		tv.tv_sec + 1000ull * tv.tv_usec;
#if defined(__NetBSD_Version__) && (__NetBSD_Version__ < 106110000)
  /* NetBSD < 1.6K does not have separate read/write statistics. */
	perf->rbytes = drive.dk_bytes;
	perf->wbytes = drive.dk_bytes;
#else
	perf->rbytes = drive.dk_rbytes;
	perf->wbytes = drive.dk_wbytes;
#endif

  /*
   * XXX - Currently, I don't know of any way to determine write/read busy
   * time separatly.
   *                                              -- Benedikt
   */
  perf->qlen = drive.dk_xfer;
	perf->rbusy_ns = ((uint64_t)1000ull * 1000ull * 1000ull * drive.dk_time_sec
    + 1000ull * drive.dk_time_usec) / 2ull;
  perf->wbusy_ns = perf->rbusy_ns;

	return(0);
}

/* *INDENT-ON* */
	/**************************	NetBSD End	***************/

#else
	/**************************************************************/
	/********************	Unsupported platform	***************/
	/**************************************************************/
#error "Your plattform is not yet supported"
#endif


	/**************************************************************/

/*
$Log: devperf.c,v $
Revision 1.8  2003/11/30 10:58:54  rogerms
Release DiskPerf 1.4.1 - Just a bug fix for Linux-2.6 systems
Thanks to Ivan Todoroski who found a bug introduced with release 1.3

Revision 1.7  2003/11/17 09:24:14  benny
NetBSD < 1.6K does not have separate read/write statistics. Thanks to martti.

Revision 1.6  2003/11/10 23:00:41  benny
Added "busy time" support for NetBSD.

Revision 1.5  2003/11/04 10:26:13  rogerms
DiskPerf 1.3

Revision 1.9  2003/11/04 09:43:09  RogerSeguin
Now retrieve both read and write busy times for Linux

Revision 1.4  2003/11/02 06:57:50  rogerms
Release 1.2

Revision 1.8  2003/11/02 06:17:30  RogerSeguin
Safer processing for Linux 2.6 - Added busy time (and queue length) for Linux 2.4 and 2.6

Revision 1.3  2003/10/18 23:02:58  rogerms
DiskPerf release 1.1

Revision 1.7  2003/10/18 06:56:41  RogerSeguin
Integration of Benedikt Meurer's work on NetBSD port

Revision 1.6  2003/10/17 20:15:05  RogerSeguin
Minor change related to Linux kernel 2.6

Revision 1.5  2003/10/16 13:08:18  RogerSeguin
Kernel 2.6 support

Revision 1.2  2003/10/16 18:48:39  benny
Added support for NetBSD.

Revision 1.1.1.1  2003/10/07 03:39:22  rogerms
Initial release - v1.0

Revision 1.4  2003/10/04 06:05:08  RogerSeguin
Fixed a possibility of 32-bit integer overflow introcduced by previous release

Revision 1.3  2003/10/02 04:15:06  RogerSeguin
Compute using rbytes/wbytes instead of rsect/wsect

Revision 1.2  2003/09/25 12:25:09  RogerSeguin
Implemented some error processing

Revision 1.1  2003/09/22 02:25:34  RogerSeguin
Initial revision

*/
