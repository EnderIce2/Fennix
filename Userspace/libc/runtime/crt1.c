// https://wiki.osdev.org/Creating_a_C_Library
#define asm __asm__ __volatile__
__attribute__((naked, used, no_stack_protector, section(".text"))) void _start()
{
	asm("movq $0, %rbp\n");
	asm("pushq %rbp\n");
	asm("pushq %rbp\n");
	asm("movq %rsp, %rbp\n");
	asm("pushq %rcx\n"
		"pushq %rdx\n"
		"pushq %rsi\n"
		"pushq %rdi\n");
	asm("call __libc_init\n");
	asm("popq %rdi\n"
		"popq %rsi\n"
		"popq %rdx\n"
		"popq %rcx\n");
	asm("call main\n");
	asm("movl %eax, %edi\n");
	asm("call _exit\n");
}
