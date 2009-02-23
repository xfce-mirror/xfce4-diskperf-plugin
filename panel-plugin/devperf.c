/* $Id$
 * 
 * Copyright (c) 2003 RogerSeguin <roger_seguin@msn.com>
 * Copyright (c) 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
	n = fscanf (pF, "%*s");	/* Skip device name */
	if (n != 1)
	    goto Error;
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

#elif defined(__OpenBSD__)
/*
 * OpenBSD support, taken from ports-tree cvs.
 * x11/xfce4/xfce4-diskperf/patches/patch-panel-plugin_devperf_c
 */

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/disk.h>

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
	int mib[3], diskn, x;
	size_t len;
	char *devname = (char *)p_pvDevice;
	struct diskstats *ds;
	struct timeval tv;

	mib[0] = CTL_HW;
	mib[1] = HW_DISKCOUNT;
	len = sizeof(diskn);

	if (sysctl(mib, 2, &diskn, &len, NULL, 0) < 0)
		return (-1);

	mib[0] = CTL_HW;
	mib[1] = HW_DISKSTATS;
	len = diskn * sizeof(struct diskstats);

	ds = malloc(len);
	if (ds == NULL)
		return (-1);

	if (sysctl(mib, 2, ds, &len, NULL, 0) < 0) {
		free(ds);
		return (-1);
	}

	for (x = 0; x < diskn; x++)
		if (!strcmp(ds[x].ds_name, devname))
			break;

	if (x == diskn) {
		free(ds);
		return (-1);
	}

	if (gettimeofday(&tv, NULL)) {
		free(ds);
		return (-1);
	}

	perf->timestamp_ns = (uint64_t)1000ull * 1000ull * 1000ull * tv.tv_sec
	    + 1000ull * tv.tv_usec; 
        perf->rbusy_ns = ((uint64_t)1000ull * 1000ull * 1000ull *
	    ds[x].ds_time.tv_sec + 1000ull * ds[x].ds_time.tv_usec) / 2ull;

	perf->wbusy_ns = perf->rbusy_ns / 2ull;
	perf->rbytes = ds[x].ds_rbytes;
	perf->wbytes = ds[x].ds_wbytes;
	perf->qlen = ds[x].ds_rxfer + ds[x].ds_wxfer;

	free(ds);

	return (0);
}

#else
	/**************************************************************/
	/********************	Unsupported platform	***************/
	/**************************************************************/
#error "Your plattform is not yet supported"
#endif
