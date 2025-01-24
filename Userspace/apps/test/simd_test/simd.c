int main(int, char *argv[], char *[])
{
#if defined(__amd64__) || defined(__i386__)
	__asm__ __volatile__("movaps (%0), %%xmm0\n"
						 "movaps %%xmm0, (%0)\n"
						 : : "r"(argv) : "memory");
#else
#warning "Unimplemented"
#endif
	return 0;
}
