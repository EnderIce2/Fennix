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

#include "kernel.h"

#include <filesystem/ustar.hpp>
#include <kshell.hpp>
#include <power.hpp>
#include <lock.hpp>
#include <printf.h>
#include <exec.hpp>
#include <cwalk.h>
#include <vm.hpp>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_THREAD_LOCALS
#define STBI_NO_HDR
#define STBI_ONLY_TGA
#include <stb/image.h>

#include "mapi.hpp"
#include "Fex.hpp"

using vfs::RefNode;

/* Files: 0.tga 1.tga ... 26.tga */
uint8_t *Frames[27];
uint32_t FrameSizes[27];
size_t FrameCount = 1;

void BootLogoAnimationThread()
{
	char BootAnimPath[16];
	while (FrameCount < 27)
	{
		sprintf(BootAnimPath, "/etc/boot/%ld.tga", FrameCount);
		RefNode *frame = fs->Open(BootAnimPath);
		if (!frame)
		{
			debug("Failed to load boot animation frame %s", BootAnimPath);
			break;
		}

		FrameSizes[FrameCount] = s_cst(uint32_t, frame->Size);
		Frames[FrameCount] = new uint8_t[frame->Size];
		frame->read(Frames[FrameCount], frame->Size);
		delete frame;
		FrameCount++;
	}

	uint32_t DispX = Display->GetBuffer(1)->Width;
	uint32_t DispY = Display->GetBuffer(1)->Height;

	for (size_t i = 1; i < FrameCount; i++)
	{
		int x, y, channels;

		if (!stbi_info_from_memory((uint8_t *)Frames[i], FrameSizes[i],
								   &x, &y, &channels))
			continue;

		uint8_t *img = stbi_load_from_memory((uint8_t *)Frames[i],
											 FrameSizes[i], &x, &y,
											 &channels, STBI_rgb_alpha);

		if (img == NULL)
			continue;

		int offsetX = DispX / 2 - x / 2;
		int offsetY = DispY / 2 - y / 2;

		for (int i = 0; i < x * y; i++)
		{
			uint32_t pixel = ((uint32_t *)img)[i];
			int r = (pixel >> 16) & 0xFF;
			int g = (pixel >> 8) & 0xFF;
			int b = (pixel >> 0) & 0xFF;
			int a = (pixel >> 24) & 0xFF;

			if (a != 0xFF)
			{
				r = (r * a) / 0xFF;
				g = (g * a) / 0xFF;
				b = (b * a) / 0xFF;
			}

			Display->SetPixel((i % x) + offsetX, (i / x) + offsetY,
							  (r << 16) | (g << 8) | (b << 0), 1);
		}

		free(img);
		Display->SetBuffer(1);
		TaskManager->Sleep(50);
	}

	int brightness = 100;
	while (brightness >= 0)
	{
		brightness -= 10;
		Display->SetBrightness(brightness, 1);
		Display->SetBuffer(1);
		TaskManager->Sleep(5);
	}
}

void ExitLogoAnimationThread()
{
	Display->SetBrightness(100, 1);
	Display->SetBuffer(1);

	/* Files: 26.tga 25.tga ... 1.tga */
	uint32_t DispX = Display->GetBuffer(1)->Width;
	uint32_t DispY = Display->GetBuffer(1)->Height;

	for (size_t i = FrameCount - 1; i > 0; i--)
	{
		int x, y, channels;

		if (!stbi_info_from_memory((uint8_t *)Frames[i], FrameSizes[i],
								   &x, &y, &channels))
			continue;

		uint8_t *img = stbi_load_from_memory((uint8_t *)Frames[i],
											 FrameSizes[i], &x, &y,
											 &channels, STBI_rgb_alpha);

		if (img == NULL)
			continue;

		int offsetX = DispX / 2 - x / 2;
		int offsetY = DispY / 2 - y / 2;

		for (int i = 0; i < x * y; i++)
		{
			uint32_t pixel = ((uint32_t *)img)[i];
			int r = (pixel >> 16) & 0xFF;
			int g = (pixel >> 8) & 0xFF;
			int b = (pixel >> 0) & 0xFF;
			int a = (pixel >> 24) & 0xFF;

			if (a != 0xFF)
			{
				r = (r * a) / 0xFF;
				g = (g * a) / 0xFF;
				b = (b * a) / 0xFF;
			}

			Display->SetPixel((i % x) + offsetX, (i / x) + offsetY,
							  (r << 16) | (g << 8) | (b << 0), 1);
		}

		free(img);
		Display->SetBuffer(1);
		TaskManager->Sleep(50);
	}

	int brightness = 100;
	while (brightness >= 0)
	{
		brightness -= 10;
		Display->SetBrightness(brightness, 1);
		Display->SetBuffer(1);
		TaskManager->Sleep(5);
	}
}
