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

#include <memory.hpp>
#include <stropts.h>
#include <string.h>
#include <thread>
#include <ini.h>

#include "../kernel.h"

namespace KernelConsole
{
	int TermColors[] = {
		[TerminalColor::BLACK] = 0x000000,
		[TerminalColor::RED] = 0xAA0000,
		[TerminalColor::GREEN] = 0x00AA00,
		[TerminalColor::YELLOW] = 0xAA5500,
		[TerminalColor::BLUE] = 0x0000AA,
		[TerminalColor::MAGENTA] = 0xAA00AA,
		[TerminalColor::CYAN] = 0x00AAAA,
		[TerminalColor::GREY] = 0xAAAAAA,
	};

	int TermBrightColors[] = {
		[TerminalColor::BLACK] = 0x858585,
		[TerminalColor::RED] = 0xFF5555,
		[TerminalColor::GREEN] = 0x55FF55,
		[TerminalColor::YELLOW] = 0xFFFF55,
		[TerminalColor::BLUE] = 0x5555FF,
		[TerminalColor::MAGENTA] = 0xFF55FF,
		[TerminalColor::CYAN] = 0x55FFFF,
		[TerminalColor::GREY] = 0xFFFFFF,
	};

	__no_sanitize("undefined") char FontRenderer::Paint(long CellX, long CellY, char Char, uint32_t Foreground, uint32_t Background)
	{
		uint64_t x = CellX * CurrentFont->GetInfo().Width;
		uint64_t y = CellY * CurrentFont->GetInfo().Height;

		switch (CurrentFont->GetInfo().Type)
		{
		case Video::FontType::PCScreenFont1:
		{
			uint32_t *PixelPtr = (uint32_t *)Display->GetBuffer;
			char *FontPtr = (char *)CurrentFont->GetInfo().PSF1Font->GlyphBuffer + (Char * CurrentFont->GetInfo().PSF1Font->Header->charsize);
			for (uint64_t Y = 0; Y < 16; Y++)
			{
				for (uint64_t X = 0; X < 8; X++)
				{
					if ((*FontPtr & (0b10000000 >> X)) > 0)
						*(unsigned int *)(PixelPtr + (x + X) + ((y + Y) * Display->GetWidth)) = Foreground;
					else
						*(unsigned int *)(PixelPtr + (x + X) + ((y + Y) * Display->GetWidth)) = Background;
				}
				FontPtr++;
			}
			break;
		}
		case Video::FontType::PCScreenFont2:
		{
			Video::FontInfo fInfo = CurrentFont->GetInfo();

			int BytesPerLine = (fInfo.PSF2Font->Header->width + 7) / 8;
			char *FontAddress = (char *)fInfo.StartAddress;
			uint32_t FontHeaderSize = fInfo.PSF2Font->Header->headersize;
			uint32_t FontCharSize = fInfo.PSF2Font->Header->charsize;
			uint32_t FontLength = fInfo.PSF2Font->Header->length;
			char *FontPtr = FontAddress + FontHeaderSize + (Char > 0 && (uint32_t)Char < FontLength ? Char : 0) * FontCharSize;

			uint32_t FontHdrWidth = fInfo.PSF2Font->Header->width;
			uint32_t FontHdrHeight = fInfo.PSF2Font->Header->height;

			for (uint32_t Y = 0; Y < FontHdrHeight; Y++)
			{
				for (uint32_t X = 0; X < FontHdrWidth; X++)
				{
					void *FramebufferAddress = (void *)((uintptr_t)Display->GetBuffer +
														((y + Y) * Display->GetWidth + (x + X)) *
															(Display->GetFramebufferStruct().BitsPerPixel / 8));

					if ((*FontPtr & (0b10000000 >> (X % 8))) > 0)
						*(uint32_t *)FramebufferAddress = Foreground;
					else
						*(uint32_t *)FramebufferAddress = Background;
				}
				FontPtr += BytesPerLine;
			}
			break;
		}
		default:
			warn("Unsupported font type");
			break;
		}
		return Char;
	}

	FontRenderer Renderer;

	ConsoleTerminal *Terminals[16] = {nullptr};
	std::atomic<ConsoleTerminal *> CurrentTerminal = nullptr;

	void paint_blinker(bool Enable)
	{
		if (CurrentTerminal == nullptr)
			return;

		ConsoleTerminal *term = CurrentTerminal.load();
		ConsoleTerminal::Blinker &blinker = term->Blink;
		TerminalCell *cell = term->Term->GetCell(Renderer.Cursor.X + Renderer.Cursor.Y * term->Term->GetWinsize()->ws_row);
		uint32_t bgColor = cell->attr.Bright ? TermBrightColors[cell->attr.Background] : TermColors[cell->attr.Background];
		Renderer.Paint(Renderer.Cursor.X, Renderer.Cursor.Y, blinker.Character, Enable ? blinker.Color : bgColor, bgColor);
	}

	void paint_blinker_thread()
	{
		bool blink = false;
		while (true)
		{
			paint_blinker(blink);
			blink = !blink;
			std::this_thread::sleep_for(std::chrono::milliseconds(CurrentTerminal.load()->Blink.Delay));
		}
	}

	void paint_callback(TerminalCell *cell, long x, long y)
	{
		if (cell->attr.Bright)
			Renderer.Paint(x, y, cell->c, TermBrightColors[cell->attr.Foreground], TermColors[cell->attr.Background]);
		else
			Renderer.Paint(x, y, cell->c, TermColors[cell->attr.Foreground], TermColors[cell->attr.Background]);
	}

	void cursor_callback(TerminalCursor *cur)
	{
		Renderer.Cursor = {cur->X, cur->Y};
		paint_blinker(false);
	}

	bool SetTheme(std::string Theme)
	{
		FileNode *rn = fs->GetByPath("/sys/cfg/term", thisProcess->Info.RootNode);
		if (rn == nullptr)
			return false;

		kstat st{};
		rn->Stat(&st);

		char *sh = new char[st.Size];
		rn->Read(sh, st.Size, 0);

		ini_t *ini = ini_load(sh, NULL);
		int themeSection, c0, c1, c2, c3, c4, c5, c6, c7, colorsIdx;
		const char *colors[8];

		debug("Loading terminal theme: \"%s\"", Theme.c_str());
		themeSection = ini_find_section(ini, Theme.c_str(), NULL);
		if (themeSection == INI_NOT_FOUND)
		{
			ini_destroy(ini);
			delete[] sh;
			return false;
		}

		auto getColorComponent = [](const char *str, int &index) -> int
		{
			int value = 0;
			while (str[index] >= '0' && str[index] <= '9')
			{
				value = value * 10 + (str[index] - '0');
				++index;
			}
			return value;
		};

		auto parseColor = [getColorComponent](const char *colorStr) -> unsigned int
		{
			int index = 0;
			int r = getColorComponent(colorStr, index);
			if (colorStr[index] == ',')
				++index;
			int g = getColorComponent(colorStr, index);
			if (colorStr[index] == ',')
				++index;
			int b = getColorComponent(colorStr, index);
			return (r << 16) | (g << 8) | b;
		};

		c0 = ini_find_property(ini, themeSection, "color0", NULL);
		c1 = ini_find_property(ini, themeSection, "color1", NULL);
		c2 = ini_find_property(ini, themeSection, "color2", NULL);
		c3 = ini_find_property(ini, themeSection, "color3", NULL);
		c4 = ini_find_property(ini, themeSection, "color4", NULL);
		c5 = ini_find_property(ini, themeSection, "color5", NULL);
		c6 = ini_find_property(ini, themeSection, "color6", NULL);
		c7 = ini_find_property(ini, themeSection, "color7", NULL);

		colors[0] = ini_property_value(ini, themeSection, c0);
		colors[1] = ini_property_value(ini, themeSection, c1);
		colors[2] = ini_property_value(ini, themeSection, c2);
		colors[3] = ini_property_value(ini, themeSection, c3);
		colors[4] = ini_property_value(ini, themeSection, c4);
		colors[5] = ini_property_value(ini, themeSection, c5);
		colors[6] = ini_property_value(ini, themeSection, c6);
		colors[7] = ini_property_value(ini, themeSection, c7);

		colorsIdx = 0;
		for (auto color : colors)
		{
			colorsIdx++;
			if (color == 0)
				continue;

			char *delimiterPos = strchr(color, ':');
			if (delimiterPos == NULL)
				continue;

			char colorStr[20], colorBrightStr[20];
			strncpy(colorStr, color, delimiterPos - color);
			colorStr[delimiterPos - color] = '\0';
			strcpy(colorBrightStr, delimiterPos + 1);

			TermColors[colorsIdx - 1] = parseColor(colorStr);
			TermBrightColors[colorsIdx - 1] = parseColor(colorBrightStr);
		}

		ini_destroy(ini);
		delete[] sh;
		return true;
	}

#ifdef DEBUG
	void __test_themes()
	{
		printf("\x1b[H\x1b[2J");
		auto testTheme = [](const char *theme)
		{
			KernelConsole::SetTheme("vga");
			printf("== Theme: \"%s\" ==\n", theme);
			KernelConsole::SetTheme(theme);
			const char *txtColors[] = {
				"30", "31", "32", "33", "34", "35", "36", "37", "39"};
			const char *bgColors[] = {
				"40", "41", "42", "43", "44", "45", "46", "47", "49"};
			const int numTxtColors = sizeof(txtColors) / sizeof(txtColors[0]);
			const int numBgColors = sizeof(bgColors) / sizeof(bgColors[0]);

			const char *msg = "H";
			for (int i = 0; i < numTxtColors; i++)
				for (int j = 0; j < numBgColors; j++)
					printf("\x1b[%sm\x1b[%sm%s\x1b[0m", txtColors[i], bgColors[j], msg);

			for (int i = 0; i < numTxtColors; i++)
				for (int j = 0; j < numBgColors; j++)
					printf("\x1b[1;%sm\x1b[1;%sm%s\x1b[0m", txtColors[i], bgColors[j], msg);

			KernelConsole::SetTheme("vga");
			printf("\n");
		};
		testTheme("vga");
		testTheme("breeze");
		testTheme("coolbreeze");
		testTheme("softlight");
		testTheme("calmsea");
		testTheme("warmember");
		// CPU::Stop();
	}
#endif

	void EarlyInit()
	{
		Renderer.CurrentFont = new Video::Font(&_binary_files_tamsyn_font_1_11_Tamsyn7x14r_psf_start,
											   &_binary_files_tamsyn_font_1_11_Tamsyn7x14r_psf_end,
											   Video::FontType::PCScreenFont2);

		size_t Rows = Display->GetWidth / Renderer.CurrentFont->GetInfo().Width;
		size_t Cols = Display->GetHeight / Renderer.CurrentFont->GetInfo().Height;
		debug("Terminal size: %ux%u", Rows, Cols);
		Terminals[0] = new ConsoleTerminal;
		Terminals[0]->Term = new VirtualTerminal(Rows, Cols, Display->GetWidth, Display->GetHeight, paint_callback, cursor_callback);
		Terminals[0]->Term->Clear(0, 0, Rows, Cols - 1);
		CurrentTerminal.store(Terminals[0], std::memory_order_release);
	}

	void LoadConsoleConfig(std::string &Config)
	{
		ini_t *ini = ini_load(Config.c_str(), NULL);
		int general = ini_find_section(ini, "general", NULL);
		int cursor = ini_find_section(ini, "cursor", NULL);
		assert(general != INI_NOT_FOUND && cursor != INI_NOT_FOUND);

		int themeIndex = ini_find_property(ini, general, "theme", NULL);
		assert(themeIndex != INI_NOT_FOUND);

		int cursorBlink = ini_find_property(ini, cursor, "blink", NULL);
		int cursorColor = ini_find_property(ini, cursor, "color", NULL);
		int cursorChar = ini_find_property(ini, cursor, "char", NULL);
		int cursorDelay = ini_find_property(ini, cursor, "delay", NULL);
		assert(cursorBlink != INI_NOT_FOUND && cursorColor != INI_NOT_FOUND && cursorChar != INI_NOT_FOUND && cursorDelay != INI_NOT_FOUND);

		const char *colorThemeStr = ini_property_value(ini, general, themeIndex);
		const char *cursorColorStr = ini_property_value(ini, cursor, cursorColor);
		const char *cursorBlinkStr = ini_property_value(ini, cursor, cursorBlink);
		const char *cursorCharStr = ini_property_value(ini, cursor, cursorChar);
		const char *cursorDelayStr = ini_property_value(ini, cursor, cursorDelay);
		debug("colorThemeStr=%s", colorThemeStr);
		debug("cursorBlinkStr=%s", cursorBlinkStr);
		debug("cursorColorStr=%s", cursorColorStr);
		debug("cursorCharStr=%s", cursorCharStr);
		debug("cursorDelayStr=%s", cursorDelayStr);

		auto getColorComponent = [](const char *str, int &index) -> int
		{
			int value = 0;
			while (str[index] >= '0' && str[index] <= '9')
			{
				value = value * 10 + (str[index] - '0');
				++index;
			}
			return value;
		};

		auto parseColor = [getColorComponent](const char *colorStr) -> unsigned int
		{
			int index = 0;
			int r = getColorComponent(colorStr, index);
			if (colorStr[index] == ',')
				++index;
			int g = getColorComponent(colorStr, index);
			if (colorStr[index] == ',')
				++index;
			int b = getColorComponent(colorStr, index);
			return (r << 16) | (g << 8) | b;
		};

		if (colorThemeStr != 0)
			SetTheme(colorThemeStr);

		if (cursorBlinkStr != 0 && strncmp(cursorBlinkStr, "true", 4) == 0)
		{
			uint32_t blinkColor = 0xFFFFFF;
			if (cursorColorStr != 0)
				blinkColor = parseColor(cursorColorStr);
			debug("cursor blink with colors %X and char '%s' and delay %s", blinkColor, cursorCharStr, cursorDelayStr);
			Terminals[0]->Blink.Enabled = true;
			Terminals[0]->Blink.Color = blinkColor;
			Terminals[0]->Blink.Character = *cursorCharStr;
			Terminals[0]->Blink.Delay = atoi(cursorDelayStr);
		}

		ini_destroy(ini);
	}

	void LateInit()
	{
		FileNode *rn = fs->GetByPath("/sys/cfg/term", thisProcess->Info.RootNode);
		if (rn == nullptr)
			return;

		{
			kstat st;
			rn->Stat(&st);
			std::string cfg;
			cfg.reserve(st.Size);
			rn->Read(cfg.data(), st.Size, 0);
			LoadConsoleConfig(cfg);
		}

		if (Terminals[0]->Blink.Enabled)
		{
			std::thread t = std::thread(paint_blinker_thread);
			t.detach();
		}

#ifdef DEBUG
		// __test_themes();
#endif
	}
}
