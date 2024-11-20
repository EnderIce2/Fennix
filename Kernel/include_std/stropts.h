/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _STROPTS_H
#define _STROPTS_H

#define __SID ('S' << 8)

#define I_NREAD (__SID | 1)
#define I_PUSH (__SID | 2)
#define I_POP (__SID | 3)
#define I_LOOK (__SID | 4)
#define I_FLUSH (__SID | 5)
#define I_SRDOPT (__SID | 6)
#define I_GRDOPT (__SID | 7)
#define I_STR (__SID | 8)
#define I_SETSIG (__SID | 9)
#define I_GETSIG (__SID | 10)
#define I_FIND (__SID | 11)
#define I_LINK (__SID | 12)
#define I_UNLINK (__SID | 13)
#define I_PEEK (__SID | 15)
#define I_FDINSERT (__SID | 16)
#define I_SENDFD (__SID | 17)
#define I_RECVFD (__SID | 14)
#define I_SWROPT (__SID | 19)
#define I_GWROPT (__SID | 20)
#define I_LIST (__SID | 21)
#define I_PLINK (__SID | 22)
#define I_PUNLINK (__SID | 23)
#define I_FLUSHBAND (__SID | 28)
#define I_CKBAND (__SID | 29)
#define I_GETBAND (__SID | 30)
#define I_ATMARK (__SID | 31)
#define I_SETCLTIME (__SID | 32)
#define I_GETCLTIME (__SID | 33)
#define I_CANPUT (__SID | 34)

#define TCGETS		0x5401
#define TCSETS		0x5402
#define TCSETSW		0x5403
#define TCSETSF		0x5404
#define TCGETA		0x5405
#define TCSETA		0x5406
#define TCSETAW		0x5407
#define TCSETAF		0x5408
#define TCSBRK		0x5409
#define TCXONC		0x540A
#define TCFLSH		0x540B
#define TIOCEXCL	0x540C
#define TIOCNXCL	0x540D
#define TIOCSCTTY	0x540E
#define TIOCGPGRP	0x540F
#define TIOCSPGRP	0x5410
#define TIOCOUTQ	0x5411
#define TIOCSTI		0x5412
#define TIOCGWINSZ	0x5413
#define TIOCSWINSZ	0x5414
#define TIOCMGET	0x5415
#define TIOCMBIS	0x5416
#define TIOCMBIC	0x5417
#define TIOCMSET	0x5418
#define TIOCGSID    0x5429

#endif
