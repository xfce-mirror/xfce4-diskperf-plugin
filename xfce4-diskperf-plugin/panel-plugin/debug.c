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

static char     _debug_id[] =
    "$Id: debug.c,v 1.1 2003/10/06 23:46:27 rogerms Exp $";


#include "debug.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <inttypes.h>


int Trace (const char *const p_pcFormat, ...)
{
    const char     *const pcFile = TRACE_LOG;
    static FILE    *s_pF = 0;
    va_list         ap;

    if (!s_pF)
	s_pF = fopen (pcFile, "w");
    va_start (ap, p_pcFormat);
    vfprintf (s_pF, p_pcFormat, ap);
    va_end (ap);
    fflush (s_pF);
    return (0);
}


/*
$Log: debug.c,v $
Revision 1.1  2003/10/06 23:46:27  rogerms
Initial revision

Revision 1.2  2003/09/23 15:18:57  RogerSeguin
Use vfprintf(3)

Revision 1.1  2003/09/22 02:25:33  RogerSeguin
Initial revision

*/
