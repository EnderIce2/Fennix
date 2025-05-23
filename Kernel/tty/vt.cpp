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

#include <kcon.hpp>

#include <fs/ioctl.hpp>
#include <memory.hpp>
#include <stropts.h>
#include <string.h>
#include <uart.hpp>

#include "../kernel.h"

namespace KernelConsole
{
	int VirtualTerminal::Open(int Flags, mode_t Mode)
	{
		std::lock_guard<std::mutex> lock(vt_mutex);
		stub;
		return 0;
	}

	int VirtualTerminal::Close()
	{
		std::lock_guard<std::mutex> lock(vt_mutex);
		stub;
		return 0;
	}

	ssize_t VirtualTerminal::Read(void *Buffer, size_t Size, off_t Offset)
	{
		std::lock_guard<std::mutex> lock(vt_mutex);

		KeyboardReport report{};

		/* FIXME: this is a hack, "static" is not a good idea */
		static bool upperCase = false;
		static bool controlKey = false;

	RecheckKeyboard:
		while (DriverManager->GlobalKeyboardInputReports.Count() == 0)
			TaskManager->Yield();

		DriverManager->GlobalKeyboardInputReports.Read(&report, 1);

		int pkey = report.Key & ~KEY_PRESSED;
		if (pkey == KEY_LEFT_SHIFT || pkey == KEY_RIGHT_SHIFT)
		{
			if (report.Key & KEY_PRESSED)
				upperCase = true;
			else
				upperCase = false;
			goto RecheckKeyboard;
		}
		else if (pkey == KEY_LEFT_CTRL || pkey == KEY_RIGHT_CTRL)
		{
			if (report.Key & KEY_PRESSED)
				controlKey = true;
			else
				controlKey = false;
			debug("controlKey = %d", controlKey);
			goto RecheckKeyboard;
		}

		if (controlKey && this->TerminalConfig.c_lflag & ICANON)
		{
			if (report.Key & KEY_PRESSED)
			{
				char cc = Driver::GetControlCharacter(report.Key);
				if (cc == 0x00)
					goto RecheckKeyboard;

				if (this->TerminalConfig.c_lflag & ECHO)
				{
					char c = Driver::GetScanCode(report.Key, true);
					this->Append('^');
					this->Append(c);
					this->Append('\n');
				}

				this->Process(cc);
				goto RecheckKeyboard;
			}
		}

		if (!(report.Key & KEY_PRESSED))
			goto RecheckKeyboard;

		if (!Driver::IsValidChar(report.Key))
			goto RecheckKeyboard;

		char c = Driver::GetScanCode(report.Key, upperCase);
		char *buf = (char *)Buffer;
		buf[0] = c;

		if (this->TerminalConfig.c_lflag & ECHO)
		{
			if (this->TerminalConfig.c_lflag & ICANON)
				this->Process(buf[0]);
			else
				this->Append(buf[0]);
		}

		if (this->TerminalConfig.c_lflag & ICANON)
		{
			fixme("ICANON");
			// if (pkey == KEY_RETURN)
			// {
			//
			// 	return bufLength;
			// }
		}

		return 1;
	}

	ssize_t VirtualTerminal::Write(const void *Buffer, size_t Size, off_t Offset)
	{
		std::lock_guard<std::mutex> lock(vt_mutex);

		char *buf = (char *)Buffer;
		debug("string: \"%*s\"", Size, buf);
		for (size_t i = 0; i < Size; i++)
		{
			// if (this->TerminalConfig.c_lflag & ICANON)
			this->Process(buf[i]);
			// else
			// 	this->Append(buf[i]);
		}

		debug("ret %ld", Size);
		return Size;
	}

	int VirtualTerminal::Ioctl(unsigned long Request, void *Argp)
	{
		std::lock_guard<std::mutex> lock(vt_mutex);

		switch (Request)
		{
		case TIOCGPTN:
		{
			fixme("stub ioctl TIOCGPTN");

			int *n = (int *)Argp;
			*n = -1;
			return 0;
		}
		case TIOCSPTLCK:
		{
			fixme("stub ioctl TIOCSPTLCK");

			int *n = (int *)Argp;
			*n = 0;
			return 0;
		}
		case TCGETS:
		{
			struct termios *t = (struct termios *)Argp;
			*t = TerminalConfig;
			return 0;
		}
		case TCSETS:
		{
			struct termios *t = (struct termios *)Argp;
			TerminalConfig = *t;
			return 0;
		}
		case TCSETSW:
		{
			debug("draining output buffer...");
			TermBuf.DrainOutput();
			debug("output buffer drained");

			TerminalConfig = *(struct termios *)Argp;
			return 0;
		}
		case TIOCGWINSZ:
		{
			struct winsize *ws = (struct winsize *)Argp;
			*ws = TerminalSize;
			return 0;
		}
		case TIOCGPGRP:
		{
			*((pid_t *)Argp) = this->ProcessGroup;
			debug("returning pgid %d", this->ProcessGroup);
			return 0;
		}
		case TIOCSPGRP:
		{
			this->ProcessGroup = *((pid_t *)Argp);
			debug("updated pgid to %d", this->ProcessGroup);
			return 0;
		}
		case TIOCGSID:
		{
			*((pid_t *)Argp) = thisProcess->Security.SessionID;
			debug("returning sid %d", thisProcess->Security.SessionID);
			return 0;
		}
		case TIOCSCTTY:
		{
			fixme("stub ioctl TIOCSCTTY");
			return 0;
		}
		default:
		{
			debug("Unknown ioctl %#lx", Request);
			return -ENOTSUP;
		}
		}
	}

	void VirtualTerminal::Clear(unsigned short StartX, unsigned short StartY, unsigned short EndX, unsigned short EndY)
	{
		assert(this->PaintCB != nullptr);
		for (long i = StartX + StartY * this->TerminalSize.ws_row; i < EndX + EndY * this->TerminalSize.ws_row; i++)
		{
			TerminalCell *cell = &this->Cells[i];
			cell->c = ' ';
			cell->attr = {};

			this->PaintCB(cell, i % this->TerminalSize.ws_row, i / this->TerminalSize.ws_row);
		}
	}

	void VirtualTerminal::Scroll(unsigned short Lines)
	{
		assert(this->PaintCB != nullptr);
		if (Lines == 0)
			return;

		Lines = Lines > this->TerminalSize.ws_col ? this->TerminalSize.ws_col : Lines;

		for (int i = 0; i < ((this->TerminalSize.ws_row * this->TerminalSize.ws_col) - (this->TerminalSize.ws_row * Lines)); i++)
		{
			this->Cells[i] = this->Cells[i + (this->TerminalSize.ws_row * Lines)];
			this->PaintCB(&this->Cells[i], i % this->TerminalSize.ws_row, i / this->TerminalSize.ws_row);
		}

		for (int i = ((this->TerminalSize.ws_row * this->TerminalSize.ws_col) - (this->TerminalSize.ws_row * Lines)); i < this->TerminalSize.ws_row * this->TerminalSize.ws_col; i++)
		{
			TerminalCell *cell = &this->Cells[i];
			cell->attr = {};
			cell->c = ' ';

			this->PaintCB(cell, i % this->TerminalSize.ws_row, i / this->TerminalSize.ws_row);
		}

		// Move the cursor up $lines
		if (this->Cursor.Y > 0)
		{
			this->Cursor.Y -= Lines;

			if (this->Cursor.Y < 0)
				this->Cursor.Y = 0;

			if (this->CursorCB != nullptr)
				this->CursorCB(&this->Cursor);
		}
	}

	void VirtualTerminal::NewLine()
	{
		this->Cursor.X = 0;
		this->Cursor.Y++;

		if (this->Cursor.Y >= this->TerminalSize.ws_col)
			this->Scroll(1);

		if (this->CursorCB != nullptr)
			this->CursorCB(&this->Cursor);
	}

	void VirtualTerminal::Append(char c)
	{
		if (c == '\n')
			this->NewLine();
		else if (c == '\r')
		{
			this->Cursor.X = 0;
			if (this->CursorCB != nullptr)
				this->CursorCB(&this->Cursor);
		}
		else if (c == '\t')
		{
			int n = 8 - (this->Cursor.X % 8);

			for (int i = 0; i < n; i++)
				this->Append(' ');
		}
		else if (c == '\b')
		{
			if (this->Cursor.X > 0)
			{
				this->Cursor.X--;
			}
			else
			{
				this->Cursor.Y--;
				this->Cursor.X = this->TerminalSize.ws_row - 1;
			}

			if (this->CursorCB != nullptr)
				this->CursorCB(&this->Cursor);
		}
		else
		{
			if (this->Cursor.X >= this->TerminalSize.ws_row)
				this->NewLine();

			TerminalCell *cell = &this->Cells[this->Cursor.X + this->Cursor.Y * this->TerminalSize.ws_row];
			cell->c = c;
			cell->attr = this->Attribute;

			assert(this->PaintCB != nullptr);
			this->PaintCB(cell, this->Cursor.X, this->Cursor.Y);

			this->Cursor.X++;

			if (this->CursorCB != nullptr)
				this->CursorCB(&this->Cursor);
		}
	}

	void VirtualTerminal::csi_cup(ANSIArgument *Args, int ArgsCount)
	{
		if (ArgsCount == 1 && Args[0].Empty)
		{
			this->Cursor.X = 0;
			this->Cursor.Y = 0;
		}
		else if (ArgsCount == 2)
		{
			if (Args[0].Empty)
				this->Cursor.Y = 0;
			else
				this->Cursor.Y = MIN(Args[0].Value - 1, this->TerminalSize.ws_col - 1);

			if (Args[1].Empty)
				this->Cursor.Y = 0;
			else
				this->Cursor.X = MIN(Args[1].Value - 1, this->TerminalSize.ws_row - 1);
		}

		if (this->CursorCB != nullptr)
			this->CursorCB(&this->Cursor);
	}

	void VirtualTerminal::csi_ed(ANSIArgument *Args, int ArgsCount)
	{
		TerminalCursor cursor = this->Cursor;

		if (Args[0].Empty)
			this->Clear(cursor.X, cursor.Y, this->TerminalSize.ws_row, this->TerminalSize.ws_col - 1);
		else
		{
			int attr = Args[0].Value;

			if (attr == 0)
				this->Clear(cursor.X, cursor.Y, this->TerminalSize.ws_row, this->TerminalSize.ws_col - 1);
			else if (attr == 1)
				this->Clear(0, 0, cursor.X, cursor.Y);
			else if (attr == 2)
				this->Clear(0, 0, this->TerminalSize.ws_row, this->TerminalSize.ws_col - 1);
		}
	}

	void VirtualTerminal::csi_el(ANSIArgument *Args, int ArgsCount)
	{
		TerminalCursor cursor = this->Cursor;

		if (Args[0].Empty)
			this->Clear(cursor.X, cursor.Y, this->TerminalSize.ws_row, cursor.Y);
		else
		{
			int attr = Args[0].Value;

			if (attr == 0)
				this->Clear(cursor.X, cursor.Y, this->TerminalSize.ws_row, cursor.Y);
			else if (attr == 1)
				this->Clear(0, cursor.Y, cursor.X, cursor.Y);
			else if (attr == 2)
				this->Clear(0, cursor.Y, this->TerminalSize.ws_row, cursor.Y);
		}
	}

	void VirtualTerminal::csi_sgr(ANSIArgument *Args, int ArgsCount)
	{
		for (int i = 0; i < ArgsCount; i++)
		{
			if (Args[i].Empty || Args[i].Value == 0)
				this->Attribute = {};
			else
			{
				int attr = Args[i].Value;

				if (attr == 1)
					this->Attribute.Bright = true;
				else if (attr >= 30 && attr <= 37)
					this->Attribute.Foreground = (TerminalColor)(attr - 30);
				else if (attr >= 40 && attr <= 47)
					this->Attribute.Background = (TerminalColor)(attr - 40);
			}
		}
	}

	void VirtualTerminal::csi_cuu(ANSIArgument *Args, int ArgsCount)
	{
		int P1 = (ArgsCount > 0 && !Args[0].Empty) ? Args[0].Value : 1;
		Cursor.Y -= P1;
		if (Cursor.Y < 0)
			Cursor.Y = 0;
		if (CursorCB)
			CursorCB(&Cursor);
	}

	void VirtualTerminal::csi_cud(ANSIArgument *Args, int ArgsCount)
	{
		int P1 = (ArgsCount > 0 && !Args[0].Empty) ? Args[0].Value : 1;
		Cursor.Y += P1;
		if (Cursor.Y >= this->TerminalSize.ws_col)
			Cursor.Y = this->TerminalSize.ws_col - 1;
		if (CursorCB)
			CursorCB(&Cursor);
	}

	void VirtualTerminal::csi_cuf(ANSIArgument *Args, int ArgsCount)
	{
		int P1 = (ArgsCount > 0 && !Args[0].Empty) ? Args[0].Value : 1;
		Cursor.X += P1;
		if (Cursor.X >= this->TerminalSize.ws_row)
			Cursor.X = this->TerminalSize.ws_row - 1;
		if (CursorCB)
			CursorCB(&Cursor);
	}

	void VirtualTerminal::csi_cub(ANSIArgument *Args, int ArgsCount)
	{
		int P1 = (ArgsCount > 0 && !Args[0].Empty) ? Args[0].Value : 1;
		Cursor.X -= P1;
		if (Cursor.X < 0)
			Cursor.X = 0;
		if (CursorCB)
			CursorCB(&Cursor);
	}

	void VirtualTerminal::csi_cnl(ANSIArgument *Args, int ArgsCount)
	{
		int P1 = (ArgsCount > 0 && !Args[0].Empty) ? Args[0].Value : 1;
		Cursor.Y += P1;
		if (Cursor.Y >= this->TerminalSize.ws_col)
			Cursor.Y = this->TerminalSize.ws_col - 1;
		Cursor.X = 0;
		if (CursorCB)
			CursorCB(&Cursor);
	}

	void VirtualTerminal::csi_cpl(ANSIArgument *Args, int ArgsCount)
	{
		int P1 = (ArgsCount > 0 && !Args[0].Empty) ? Args[0].Value : 1;
		Cursor.Y -= P1;
		if (Cursor.Y < 0)
			Cursor.Y = 0;
		Cursor.X = 0;
		if (CursorCB)
			CursorCB(&Cursor);
	}

	void VirtualTerminal::csi_cha(ANSIArgument *Args, int ArgsCount)
	{
		int P1 = (ArgsCount > 0 && !Args[0].Empty) ? Args[0].Value : 1;
		Cursor.X = P1 - 1;
		if (Cursor.X >= this->TerminalSize.ws_row)
			Cursor.X = this->TerminalSize.ws_row - 1;
		if (CursorCB)
			CursorCB(&Cursor);
	}

	void VirtualTerminal::ProcessControlCharacter(char c)
	{
		auto ccheck = [&](int v)
		{
			return (this->TerminalConfig.c_cc[v] != 0x00 &&
					this->TerminalConfig.c_cc[v] == c);
		};

		auto ciflag = [&](int f)
		{
			return (this->TerminalConfig.c_iflag & f) != 0;
		};

		auto clflag = [&](int f)
		{
			return (this->TerminalConfig.c_lflag & f) != 0;
		};

		if (ciflag(IXON) && ccheck(VSTOP))
		{
			fixme("flow control: stopping output");
			return;
		}

		if (ciflag(IXON) && ccheck(VSTART))
		{
			fixme("flow control: resuming output");
			return;
		}

		if (clflag(ISIG))
		{
			if (ccheck(VINTR))
			{
				if (this->ProcessGroup == 0)
				{
					debug("Process group is 0!!!");
					return;
				}

				for (auto proc : thisProcess->GetContext()->GetProcessList())
				{
					if (proc->Security.ProcessGroupID != this->ProcessGroup)
						continue;

					debug("Sending signal SIGINT to %s(%d)", proc->Name, proc->ID);
					proc->SendSignal(SIGINT);
				}
				return;
			}
			else if (ccheck(VQUIT))
			{
				if (this->ProcessGroup == 0)
				{
					debug("Process group is 0!!!");
					return;
				}

				for (auto proc : thisProcess->GetContext()->GetProcessList())
				{
					if (proc->Security.ProcessGroupID != this->ProcessGroup)
						continue;

					debug("Sending signal SIGQUIT to %s(%d)", proc->Name, proc->ID);
					proc->SendSignal(SIGQUIT);
				}
				return;
			}
			else if (ccheck(VSUSP))
			{
				if (this->ProcessGroup == 0)
				{
					debug("Process group is 0!!!");
					return;
				}

				for (auto proc : thisProcess->GetContext()->GetProcessList())
				{
					if (proc->Security.ProcessGroupID != this->ProcessGroup)
						continue;

					debug("Sending signal SIGTSTP to %s(%d)", proc->Name, proc->ID);
					proc->SendSignal(SIGTSTP);
				}
				return;
			}
		}

		if (c == '\r')
		{
			if (ciflag(IGNCR))
				return;
			if (ciflag(ICRNL))
				c = '\n';
		}
		else if (c == '\n' && (ciflag(INLCR)))
			c = '\r';

		if (clflag(ICANON))
		{
			if (ccheck(VERASE))
			{
				if (this->Cursor.X > 0)
				{
					this->Cursor.X--;
					this->Append('\b');
					this->Append(' ');
					this->Append('\b');
				}
				return;
			}
			else if (ccheck(VKILL))
			{
				fixme("clear the current line");
				return;
			}
		}

		if (clflag(ECHO))
		{
			if (c == '\n')
				this->Append('\n');
			else
				this->Append(c);
		}
	}

	void VirtualTerminal::Process(char c)
	{
#ifdef DEBUG
#if defined(__amd64__) || defined(__i386__)
		static int once = 0;
		static uint8_t com4 = 0xFF;
		if (!once++)
		{
			com4 = inb(0x2E8);
			debug("COM4 is available");
			outb(s_cst(uint16_t, 0x2E8 + 1), 0x00);
			outb(s_cst(uint16_t, 0x2E8 + 3), 0x80);
			outb(s_cst(uint16_t, 0x2E8 + 0), 0x01);
			outb(s_cst(uint16_t, 0x2E8 + 1), 0x00);
			outb(s_cst(uint16_t, 0x2E8 + 3), 0x03);
			outb(s_cst(uint16_t, 0x2E8 + 2), 0xC7);
			outb(s_cst(uint16_t, 0x2E8 + 4), 0x0B);
			outb(s_cst(uint16_t, 0x2E8 + 4), 0x0F);
		}
		if (com4 != 0xFF)
		{
			while ((inb(s_cst(uint16_t, 0x2E8 + 5)) & 0x20) == 0)
				;
			outb(0x2E8, c);
		}

// while (true)
// {
// 	while ((inb(0x2E8 + 5) & 1) == 0)
// 		;
// 	outb(0x2E8, inb(0x2E8));
// }
#endif
#endif

		if (this->TerminalConfig.c_lflag & ICANON)
		{
			if ((c > 0x00 && c <= 0x1F) && c != '\x1b')
			{
				this->ProcessControlCharacter(c);
				return;
			}
		}

		ANSIParser *parser = &this->Parser;

		switch (parser->State)
		{
		case ANSIParser::ParserState::Escape:
		{
			if (c == '\x1b')
			{
				parser->State = ANSIParser::ParserState::Bracket;
				parser->Index = 0;
				parser->Stack[parser->Index].Value = 0;
				parser->Stack[parser->Index].Empty = true;
			}
			else
			{
				parser->State = ANSIParser::ParserState::Escape;
				this->Append(c);
			}
			break;
		}
		case ANSIParser::ParserState::Bracket:
		{
			if (c == '[')
				parser->State = ANSIParser::ParserState::Attribute;
			else
			{
				parser->State = ANSIParser::ParserState::Escape;
				this->Append(c);
			}
			break;
		}
		case ANSIParser::ParserState::Attribute:
		{
			if (isdigit(c))
			{
				parser->Stack[parser->Index].Value *= 10;
				parser->Stack[parser->Index].Value += c - '0';
				parser->Stack[parser->Index].Empty = false;
			}
			else
			{
				if (parser->Index < 8)
					parser->Index++;

				parser->Stack[parser->Index].Value = 0;
				parser->Stack[parser->Index].Empty = true;
				parser->State = ANSIParser::ParserState::EndValue;
			}
			break;
		}
		case ANSIParser::ParserState::EndValue:
			break;
		default:
		{
			error("Invalid parser state: %d", parser->State);
			assert(!"Invalid parser state");
		}
		}

		if (parser->State == ANSIParser::ParserState::EndValue)
		{
			if (c == ';')
				parser->State = ANSIParser::ParserState::Attribute;
			else
			{
				switch (c)
				{
				case 'A':
					this->csi_cuu(parser->Stack, parser->Index);
					break;
				case 'B':
					this->csi_cud(parser->Stack, parser->Index);
					break;

				case 'C':
					this->csi_cuf(parser->Stack, parser->Index);
					break;
				case 'D':
					this->csi_cub(parser->Stack, parser->Index);
					break;

				case 'E':
					this->csi_cnl(parser->Stack, parser->Index);
					break;
				case 'F':
					this->csi_cpl(parser->Stack, parser->Index);
					break;

				case 'G':
					this->csi_cha(parser->Stack, parser->Index);
					break;
				case 'd':
					fixme("move cursor left P1 columns");
					break;

				case 'H':
					this->csi_cup(parser->Stack, parser->Index);
					break;
				case 'J':
					this->csi_ed(parser->Stack, parser->Index);
					break;
				case 'K':
					this->csi_el(parser->Stack, parser->Index);
					break;
				case 'm':
					this->csi_sgr(parser->Stack, parser->Index);
					break;
				default:
					break;
				}
				parser->State = ANSIParser::ParserState::Escape;
			}
		}
	}

	VirtualTerminal::VirtualTerminal(unsigned short Rows, unsigned short Columns,
									 unsigned short XPixels, unsigned short YPixels,
									 PaintCallback _Paint, CursorCallback _Print)
		: PaintCB(_Paint), CursorCB(_Print)
	{
		this->TerminalSize = {
			.ws_row = Rows,
			.ws_col = Columns,
			.ws_xpixel = XPixels,
			.ws_ypixel = YPixels,
		};

		/*
		- ICRNL  - Map Carriage Return to New Line
		- IXON   - Enable XON/XOFF flow control

		- OPOST  - Enable output processing
		- ONLCR  - Map New Line to Carriage Return - New Line

		- CS8    - 8-bit characters
		- CREAD  - Enable receiver
		- HUPCL  - Hang up on last close

		- ECHO   - Echo input characters
		- ICANON - Enable canonical input (enable line editing)
		- ISIG   - Enable signals
		*/
		this->TerminalConfig.c_iflag = /*ICRNL |*/ IXON;
		this->TerminalConfig.c_oflag = OPOST | ONLCR;
		this->TerminalConfig.c_cflag = CS8 | CREAD | HUPCL;
		this->TerminalConfig.c_lflag = ECHO | ICANON | ISIG;

		this->TerminalConfig.c_cc[VINTR] = 'C' - 0x40;
		this->TerminalConfig.c_cc[VQUIT] = '\\' - 0x40;
		this->TerminalConfig.c_cc[VERASE] = '\177';
		this->TerminalConfig.c_cc[VKILL] = 'U' - 0x40;
		this->TerminalConfig.c_cc[VEOF] = 'D' - 0x40;
		this->TerminalConfig.c_cc[VSTART] = 'Q' - 0x40;
		this->TerminalConfig.c_cc[VSTOP] = 'S' - 0x40;
		this->TerminalConfig.c_cc[VSUSP] = 'Z' - 0x40;
		this->TerminalConfig.c_cc[VREPRINT] = 'R' - 0x40;
		this->TerminalConfig.c_cc[VDISCARD] = 'O' - 0x40;
		this->TerminalConfig.c_cc[VWERASE] = 'W' - 0x40;
		this->TerminalConfig.c_cc[VLNEXT] = 'V' - 0x40;

		this->TerminalConfig.c_cc[VTIME] = 0; /* Timeout for non-canonical read */
		this->TerminalConfig.c_cc[VMIN] = 1;  /* Minimum number of characters for non-canonical read */

		this->Cells = new TerminalCell[Rows * Columns];

		debug("Allocated %d entries (%d bytes at %#lx-%#lx for terminal cells)", Rows * Columns, (Rows * Columns) * sizeof(TerminalCell), this->Cells, (char *)this->Cells + (Rows * Columns) * sizeof(TerminalCell));
	}

	VirtualTerminal::~VirtualTerminal() { delete[] this->Cells; }
}
