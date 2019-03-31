/*
 * speaker.h -- interface definitions for speaker ioctl()
 *
 * v1.4 by Eric S. Raymond (esr@snark.thyrsus.com) Aug 1993
 *      modified for FreeBSD by Andrew A. Chernov <ache@astral.msk.su>
 *
 * $FreeBSD: release/9.0.0/sys/dev/speaker/speaker.h 152306 2005-11-11 09:57:32Z ru $
 */

#ifndef	_DEV_SPEAKER_SPEAKER_H_
#define	_DEV_SPEAKER_SPEAKER_H_

#include <sys/ioccom.h>

#define SPKRTONE        _IOW('S', 1, tone_t)    /* emit tone */
#define SPKRTUNE        _IO('S', 2)             /* emit tone sequence*/

typedef struct
{
    int	frequency;	/* in hertz */
    int duration;	/* in 1/100ths of a second */
} tone_t;

/*
 * Strings written to the speaker device are interpreted as tunes and played;
 * see the spkr(4) man page for details.
 */

#endif /* !_DEV_SPEAKER_SPEAKER_H_ */
