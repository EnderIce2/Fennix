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

#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

/* FIXME: Not implemented! Stub code! */
#define TCSBRK 0
#define TCGETS 0
#define TIOCGSID 0
#define TIOCGWINSZ 0
#define TCSETS 0
#define TCSETSW 0
#define TCSETSF 0
#define TIOCSWINSZ 0

export speed_t cfgetispeed(const struct termios *termios_p)
{
	return termios_p->c_ispeed;
}

export speed_t cfgetospeed(const struct termios *termios_p)
{
	return termios_p->c_ospeed;
}

export int cfsetispeed(struct termios *termios_p, speed_t speed)
{
	if (speed < 0)
	{
		errno = EINVAL;
		return -1;
	}

	termios_p->c_ispeed = speed;
	return 0;
}

export int cfsetospeed(struct termios *termios_p, speed_t speed)
{
	if (speed < 0)
	{
		errno = EINVAL;
		return -1;
	}

	termios_p->c_ospeed = speed;
	return 0;
}

export int tcdrain(int fildes)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	if (ioctl(fildes, TCSBRK, 1) == -1)
	{
		if (errno == EINTR)
			errno = EINTR;
		else if (errno == ENOTTY)
			errno = ENOTTY;
		else
			errno = EINVAL;
		return -1;
	}

	return 0;
}

export int tcflow(int fildes, int action)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	switch (action)
	{
	case TCOOFF:
		// Suspend output
		// Implementation specific code to suspend output
		break;
	case TCOON:
		// Restart suspended output
		// Implementation specific code to restart output
		break;
	case TCIOFF:
		// Transmit STOP character
		// Implementation specific code to transmit STOP character
		break;
	case TCION:
		// Transmit START character
		// Implementation specific code to transmit START character
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}

export int tcflush(int fildes, int queue_selector)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	switch (queue_selector)
	{
	case TCIFLUSH:
		// Implementation specific code to flush data received but not read
		break;
	case TCOFLUSH:
		// Implementation specific code to flush data written but not transmitted
		break;
	case TCIOFLUSH:
		// Implementation specific code to flush both data received but not read and data written but not transmitted
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}

export int tcgetattr(int fildes, struct termios *termios_p)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	// Implementation specific code to get terminal attributes
	// For example, using ioctl system call
	if (ioctl(fildes, TCGETS, termios_p) == -1)
	{
		if (errno == ENOTTY)
			errno = ENOTTY;
		else
			errno = EINVAL;
		return -1;
	}

	return 0;
}

export pid_t tcgetsid(int fildes)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	pid_t sid = ioctl(fildes, TIOCGSID);
	if (sid == -1)
	{
		if (errno == ENOTTY)
			errno = ENOTTY;
		else
			errno = EINVAL;
		return -1;
	}

	return sid;
}

export int tcgetwinsize(int fildes, struct winsize *winsize_p)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	if (ioctl(fildes, TIOCGWINSZ, winsize_p) == -1)
	{
		if (errno == ENOTTY)
			errno = ENOTTY;
		else
			errno = EINVAL;
		return -1;
	}

	return 0;
}

export int tcsendbreak(int fildes, int duration)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	if (ioctl(fildes, TCSBRK, duration) == -1)
	{
		if (errno == ENOTTY)
			errno = ENOTTY;
		else if (errno == EIO)
			errno = EIO;
		else
			errno = EINVAL;
		return -1;
	}

	return 0;
}

export int tcsetattr(int fildes, int optional_actions, struct termios *termios_p)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	switch (optional_actions)
	{
	case TCSANOW:
		// Change occurs immediately
		if (ioctl(fildes, TCSETS, termios_p) == -1)
		{
			if (errno == ENOTTY)
				errno = ENOTTY;
			else
				errno = EINVAL;
			return -1;
		}
		break;
	case TCSADRAIN:
		// Change occurs after all output is transmitted
		if (ioctl(fildes, TCSETSW, termios_p) == -1)
		{
			if (errno == ENOTTY)
				errno = ENOTTY;
			else
				errno = EINVAL;
			return -1;
		}
		break;
	case TCSAFLUSH:
		// Change occurs after all output is transmitted and all input is discarded
		if (ioctl(fildes, TCSETSF, termios_p) == -1)
		{
			if (errno == ENOTTY)
				errno = ENOTTY;
			else
				errno = EINVAL;
			return -1;
		}
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}

export int tcsetwinsize(int fildes, const struct winsize *winsize_p)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	if (ioctl(fildes, TIOCSWINSZ, winsize_p) == -1)
	{
		if (errno == ENOTTY)
			errno = ENOTTY;
		else if (errno == EIO)
			errno = EIO;
		else
			errno = EINVAL;
		return -1;
	}

	return 0;
}
