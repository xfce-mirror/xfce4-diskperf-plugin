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
#ifndef _devperf_h
#define _devperf_h
static char     _devperf_h_id[] =
    "$Id: devperf.h,v 1.1 2003/10/07 00:56:43 rogerms Exp $";

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
} devperf_t;


#ifdef __cplusplus
extern          "C" {
#endif

    int             DevCheckStatAvailability ();
    /* Check the availability of required kernel statistics */
    /* Return 0 on success */

    int             DevGetPerfData (dev_t device, struct devperf_t *perf);
    /* Get disk performance data stored by the kernel (/proc/partitions) */
    /* Return 0 on success, -1 otherwise */

#ifdef __cplusplus
}				/* extern "C" */
#endif
/*
$Log: devperf.h,v $
Revision 1.1  2003/10/07 00:56:43  rogerms
Initial revision

Revision 1.3  2003/10/02 04:14:41  RogerSeguin
Compute using rbytes/wbytes instead of rsect/wsect

Revision 1.2  2003/09/25 12:24:46  RogerSeguin
Implemented some error processing

Revision 1.1  2003/09/22 02:25:35  RogerSeguin
Initial revision

*/
#endif				/* _devperf_h */
