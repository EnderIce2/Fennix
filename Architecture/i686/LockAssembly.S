# https://en.wikipedia.org/wiki/Spinlock

.text

# void SpinLock_Lock(volatile uint32_t *LockData)
.global SpinLock_Lock

# void SpinLock_Unlock(volatile uint32_t *LockData)
.global SpinLock_Unlock

# uint32_t SpinLock_CheckAndLock(volatile uint32_t *LockData)
.global SpinLock_CheckAndLock

# uint32_t SpinLock_WithTimeout(volatile uint32_t *LockData, volatile uint32_t Iterations)
.global SpinLock_WithTimeout

# DeadLockHandler(volatile <struct> *LockStructure)
.extern DeadLockHandler

SpinLock_Lock:
    xor %eax,%eax
    lock btsl $0,(%edi)
    jc Spin
    ret
Spin:
    inc %eax
    cmp $0x10000000,%eax
    je Deadlock
    pause
    test $1,(%edi)
    jnz Spin
    jmp SpinLock_Lock

SpinLock_Unlock:
    lock btrl $0,(%edi)
    ret

Deadlock:
    push %edi
    push %edi
    xor %eax,%eax
    call DeadLockHandler
    pop %edi
    pop %edi
    jmp Spin

SpinLock_CheckAndLock:
    xor %eax,%eax
    lock btsl $0,(%edi)
    setc %al
    ret

SpinLock_WithTimeout:
    xor %eax,%eax
SpinTimeout:
    inc %eax
    lock btsl $0,(%edi)
    setc %bl
    cmp $0,%bl
    je LockAquired
    cmp %esi,%eax
    je LockTimedOut
    pause
    jmp SpinTimeout
LockAquired:
    mov $1,%eax
    ret
LockTimedOut:
    xor %eax,%eax
    ret
