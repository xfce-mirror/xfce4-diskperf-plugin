/*
 * This file is part of Xfce (https://gitlab.xfce.org).
 *
 * Copyright (c) 2003-2004 Roger Seguin <roger_seguin@msn.com>
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

#ifndef _devperf_h
#define _devperf_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>


enum {
    NO_ERROR,
    NO_EXTENDED_STATS
};


typedef struct devperf_t {
    uint64_t        timestamp_ns;
    uint64_t        rbytes;	/* Number of bytes read from the device */
    uint64_t        wbytes;	/* Number of bytes written to the device */
    uint64_t        rbusy_ns;	/* Device read busy time */
    uint64_t        wbusy_ns;	/* Device write busy time */
    int32_t         qlen;	/* Current queue length */
} devperf_t;


#ifdef __cplusplus
extern          "C" {
#endif

    int             DevPerfInit (void);
    /* Make required initialisations */
    /* Return 0 on success */

    int             DevCheckStatAvailability (char const **StatisticsFile);
    /* Check the availability of required kernel statistics */
    /* Get the statistics file name */
    /* Return 0 on success */

    int             DevGetPerfData (const void *devid,
				    struct devperf_t *perf);
    /* Get disk performance data stored by the kernel */
    /* Return 0 on success, -1 otherwise */

#ifdef __cplusplus
}				/* extern "C" */
#endif

#endif				/* _devperf_h */
