void __attribute__((naked, used, no_stack_protector)) _start()
{
	__asm__("movq $0, %rbp\n"
			"pushq %rbp\n"
			"pushq %rbp\n"
			"movq %rsp, %rbp\n"

			"pushq %rcx\n"
			"pushq %rdx\n"
			"pushq %rsi\n"
			"pushq %rdi\n"

			"call ld_main\n"
			"movl %eax, %edi\n" // Move return value to edi
			"cmp $0, %edi\n" // Check if return value is 0
			"jne _exit\n" // If not, jump to _exit

			"popq %rdi\n"
			"popq %rsi\n"
			"popq %rdx\n"
			"popq %rcx\n"

			"call ld_load\n"
			"movl %eax, %edi\n" // Move return value to edi
			"call _exit"); // Call _exit
}

void _exit(int Code)
{
	__asm__ __volatile__("syscall"
						 :
						 : "a"(0), "D"(Code)
						 : "rcx", "r11", "memory");
	while (1)
		;
}
