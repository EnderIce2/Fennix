int main(int, char *[], char *[])
{
#if defined(__amd64__) || defined(__i386__)
	__asm__ __volatile__("movaps %%xmm1, %%xmm0\n" : : : "memory");
#else
#warning "Unimplemented"
#endif
	return 0;
}
