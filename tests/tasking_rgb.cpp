#ifdef DEBUG

#include "t.h"

#include "../kernel.h"

BootInfo::FramebufferInfo fb_ptr{};
void tasking_test_fb_loop(int x, int y, uint32_t color)
{
	assert(fb_ptr.BaseAddress != nullptr);
	while (true)
	{
		for (int i = 0; i < 16; i++)
		{
			uint32_t *Pixel = (uint32_t *)((uintptr_t)fb_ptr.BaseAddress +
										   ((y + i) * fb_ptr.Width + x) *
											   (fb_ptr.BitsPerPixel / 8));
			for (int j = 0; j < 16; j++)
			{
				*Pixel = color;
				Pixel++;
			}
		}
	}
}

void TTfbL_red() { tasking_test_fb_loop(0, 0, 0xFFFF0000); }
void TTfbL_green() { tasking_test_fb_loop(16, 0, 0xFF00FF00); }
void TTfbL_blue() { tasking_test_fb_loop(32, 0, 0xFF0000FF); }
void TTfbL_white() { tasking_test_fb_loop(48, 0, 0xFFFFFFFF); }
void TTfbL_gray() { tasking_test_fb_loop(64, 0, 0xFF888888); }
void TTfbL_red_neg() { tasking_test_fb_loop(0, 0, 0xFF00FFFF); }
void TTfbL_green_neg() { tasking_test_fb_loop(16, 0, 0xFFFF00FF); }
void TTfbL_blue_neg() { tasking_test_fb_loop(32, 0, 0xFFFFFF00); }
void TTfbL_white_neg() { tasking_test_fb_loop(48, 0, 0xFF000000); }
void TTfbL_gray_neg() { tasking_test_fb_loop(64, 0, 0xFF777777); }
void TTfbL_rainbow_fct(int offset)
{
	while (true)
	{
		/*                        AARRGGBB*/
		static uint32_t color = 0xFF000000;

		for (int i = 0; i < 64; i++)
		{
			uint32_t *Pixel = (uint32_t *)((uintptr_t)fb_ptr.BaseAddress +
										   ((offset + i) * fb_ptr.Width) *
											   (fb_ptr.BitsPerPixel / 8));
			for (int j = 0; j < 16; j++)
			{
				*Pixel = color;
				Pixel++;
			}
		}
		if (color >= 0xFFFFFFFF)
			color = 0xFF000000;
		color++;
	}
}
void TTfbL_rainbow_idle() { TTfbL_rainbow_fct(16); }
void TTfbL_rainbow_low() { TTfbL_rainbow_fct(80); }
void TTfbL_rainbow_norm() { TTfbL_rainbow_fct(144); }
void TTfbL_rainbow_high() { TTfbL_rainbow_fct(208); }
void TTfbL_rainbow_crit() { TTfbL_rainbow_fct(272); }
void tasking_test_fb()
{
	fb_ptr = Display->GetFramebufferStruct();
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_red));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_green));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_blue));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_white));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_gray));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_red_neg));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_green_neg));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_blue_neg));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_white_neg));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_gray_neg));

	{
		CriticalSection cs; /* Start all threads at the same time */
		auto tti = TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_rainbow_idle));
		auto ttl = TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_rainbow_low));
		auto ttn = TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_rainbow_norm));
		auto tth = TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_rainbow_high));
		auto ttc = TaskManager->CreateThread(thisProcess, Tasking::IP(TTfbL_rainbow_crit));

		tti->SetPriority(Tasking::TaskPriority::Idle);
		ttl->SetPriority(Tasking::TaskPriority::Low);
		ttn->SetPriority(Tasking::TaskPriority::Normal);
		tth->SetPriority(Tasking::TaskPriority::High);
		ttc->SetPriority(Tasking::TaskPriority::Critical);
	}
	// Exit
}

#endif // DEBUG
