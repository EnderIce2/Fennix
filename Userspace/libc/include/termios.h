/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <sys/ioctl.h>

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32

struct termios
{
	tcflag_t c_iflag; /* input modes */
	tcflag_t c_oflag; /* output modes */
	tcflag_t c_cflag; /* control modes */
	tcflag_t c_lflag; /* local modes */
	cc_t c_cc[NCCS];  /* control chars */
};

/* Subscript values for c_cc array */
#define VEOF 0
#define VEOL 1
#define VERASE 2
#define VINTR 3
#define VKILL 4
#define VMIN 5
#define VQUIT 6
#define VSTART 7
#define VSTOP 8
#define VSUSP 9
#define VTIME 10

/* Input modes */
#define BRKINT 0x0001
#define ICRNL 0x0002
#define IGNBRK 0x0004
#define IGNCR 0x0008
#define IGNPAR 0x0010
#define INLCR 0x0020
#define INPCK 0x0040
#define ISTRIP 0x0080
#define IUCLC 0x0100
#define IXANY 0x0200
#define IXOFF 0x0400
#define IXON 0x0800
#define PARMRK 0x1000

/* Output modes */
#define OPOST 0x0001
#define OLCUC 0x0002
#define ONLCR 0x0004
#define OCRNL 0x0008
#define ONOCR 0x0010
#define ONLRET 0x0020
#define OFILL 0x0040

#define NLDLY 0x0080
#define NL0 0x0000
#define NL1 0x0080

#define CRDLY 0x0100
#define CR0 0x0000
#define CR1 0x0100
#define CR2 0x0200
#define CR3 0x0300

#define TABDLY 0x0400
#define TAB0 0x0000
#define TAB1 0x0400
#define TAB2 0x0800
#define TAB3 0x0C00

#define BSDLY 0x1000
#define BS0 0x0000
#define BS1 0x1000

#define VTDLY 0x2000
#define VT0 0x0000
#define VT1 0x2000

#define FFDLY 0x4000
#define FF0 0x0000
#define FF1 0x4000

/* Baud rates */
#define B0 0
#define B50 50
#define B75 75
#define B110 110
#define B134 134
#define B150 150
#define B200 200
#define B300 300
#define B600 600
#define B1200 1200
#define B1800 1800
#define B2400 2400
#define B4800 4800
#define B9600 9600
#define B19200 19200
#define B38400 38400

/* Control modes */
#define CSIZE 0x0001
#define CS5 0x0000
#define CS6 0x0001
#define CS7 0x0002
#define CS8 0x0003

#define CSTOPB 0x0004
#define CREAD 0x0008
#define PARENB 0x0010
#define PARODD 0x0020
#define HUPCL 0x0040
#define CLOCAL 0x0080

/* Local modes */
#define ECHO 0x0001
#define ECHOE 0x0002
#define ECHOK 0x0004
#define ECHONL 0x0008
#define ICANON 0x0010
#define IEXTEN 0x0020
#define ISIG 0x0040
#define NOFLSH 0x0080
#define TOSTOP 0x0100
#define XCASE 0x0200

/* Attribute selection */
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2

/* Line control */
#define TCIFLUSH 0
#define TCIOFLUSH 1
#define TCOFLUSH 2

#define TCIOFF 0
#define TCION 1
#define TCOOFF 2
#define TCOON 3

/* Function prototypes */
speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);
int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);
int tcdrain(int fildes);
int tcflow(int fildes, int action);
int tcflush(int fildes, int queue_selector);
int tcgetattr(int fildes, struct termios *termios_p);
pid_t tcgetsid(int fildes);
int tcsendbreak(int fildes, int duration);
int tcsetattr(int fildes, int optional_actions, struct termios *termios_p);

#endif // _TERMIOS_H
