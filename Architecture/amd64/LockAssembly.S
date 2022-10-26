# https://en.wikipedia.org/wiki/Spinlock

.text

# void SpinLock_Lock(volatile uint64_t *LockData)
.global SpinLock_Lock

# void SpinLock_Unlock(volatile uint64_t *LockData)
.global SpinLock_Unlock

# uint64_t SpinLock_CheckAndLock(volatile uint64_t *LockData)
.global SpinLock_CheckAndLock

# uint64_t SpinLock_WithTimeout(volatile uint64_t *LockData, volatile uint64_t Iterations)
.global SpinLock_WithTimeout

# DeadLockHandler(volatile <struct> *LockStructure)
.extern DeadLockHandler

SpinLock_Lock:
    xorq %rax,%rax
    lock btsl $0,(%rdi)
    jc Spin
    ret
Spin:
    incq %rax
    cmpq $0x10000000,%rax
    je Deadlock
    pause
    testl $1,(%rdi)
    jnz Spin
    jmp SpinLock_Lock

SpinLock_Unlock:
    lock btrl $0,(%rdi)
    ret

Deadlock:
    pushq %rdi
    pushq %rdi
    xorq %rax,%rax
    call DeadLockHandler
    popq %rdi
    popq %rdi
    jmp Spin

SpinLock_CheckAndLock:
    xorl %eax,%eax
    lock btsl $0,(%rdi)
    setcb %al
    ret

SpinLock_WithTimeout:
    xorq %rax,%rax
SpinTimeout:
    incq %rax
    lock btsl $0,(%rdi)
    setcb %bl
    cmpb $0,%bl
    je LockAquired
    cmpq %rsi,%rax
    je LockTimedOut
    pause
    jmp SpinTimeout
LockAquired:
    movq $1,%rax
    ret
LockTimedOut:
    xorq %rax,%rax
    ret
