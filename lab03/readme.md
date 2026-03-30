# Labo 3

# Hoisting an Invariant Function Call Out of a Loop

A function is called inside a loop, which returns always the same value, because it depends only on a variable which isn't changed in the loop.

The way to optimize this is to call the function outside the loop and store the result in a variable.

It is also to make the function call an inline function, if it isn't too complex.

## Non optimized code

[Link to Compiler Explorer](https://godbolt.org/z/rs34zxv66)

```C
int compute(int x) { return x * x + 1; }

void fill(int *a, int n, int x) {
    for (int i = 0; i < n; i++)
        a[i] = compute(x);
}
```

```Assembly
compute:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], edi
        mov     eax, DWORD PTR [rbp-4]
        imul    eax, eax
        add     eax, 1
        pop     rbp
        ret
fill:
        push    rbp
        mov     rbp, rsp
        push    rbx
        sub     rsp, 32
        mov     QWORD PTR [rbp-32], rdi
        mov     DWORD PTR [rbp-36], esi
        mov     DWORD PTR [rbp-40], edx
        mov     DWORD PTR [rbp-12], 0
        jmp     .L4
.L5:
        mov     eax, DWORD PTR [rbp-12]
        cdqe
        lea     rdx, [0+rax*4]
        mov     rax, QWORD PTR [rbp-32]
        lea     rbx, [rdx+rax]
        mov     eax, DWORD PTR [rbp-40]
        mov     edi, eax
        call    compute
        mov     DWORD PTR [rbx], eax
        add     DWORD PTR [rbp-12], 1
.L4:
        mov     eax, DWORD PTR [rbp-12]
        cmp     eax, DWORD PTR [rbp-36]
        jl      .L5
        nop
        nop
        mov     rbx, QWORD PTR [rbp-8]
        leave
        ret
```

## Optimized code

[Link to Compiler Explorer](https://godbolt.org/z/Tf6M78eEP)

```C
inline int compute(int x) { return x * x + 1; }

void fill(int *a, int n, int x) {
    int value = compute(x);
    for (int i = 0; i < n; i++)
        a[i] = value;
}
```

```Assembly
fill:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        mov     QWORD PTR [rbp-24], rdi
        mov     DWORD PTR [rbp-28], esi
        mov     DWORD PTR [rbp-32], edx
        mov     eax, DWORD PTR [rbp-32]
        mov     edi, eax
        call    compute
        mov     DWORD PTR [rbp-8], eax
        mov     DWORD PTR [rbp-4], 0
        jmp     .L2
.L3:
        mov     eax, DWORD PTR [rbp-4]
        cdqe
        lea     rdx, [0+rax*4]
        mov     rax, QWORD PTR [rbp-24]
        add     rdx, rax
        mov     eax, DWORD PTR [rbp-8]
        mov     DWORD PTR [rdx], eax
        add     DWORD PTR [rbp-4], 1
.L2:
        mov     eax, DWORD PTR [rbp-4]
        cmp     eax, DWORD PTR [rbp-28]
        jl      .L3
        nop
        nop
        leave
        ret
```

# Compiler optimised with -02

[Link to Compiler Explorer](https://godbolt.org/z/hY5G1TToE)

```Assembly
compute:
        imul    edi, edi
        lea     eax, [rdi+1]
        ret
fill:
        test    esi, esi
        jle     .L3
        imul    edx, edx
        movsx   rsi, esi
        lea     rax, [rdi+rsi*4]
        add     edx, 1
        and     esi, 1
        je      .L5
        mov     DWORD PTR [rdi], edx
        add     rdi, 4
        cmp     rdi, rax
        je      .L12
.L5:
        mov     DWORD PTR [rdi], edx
        add     rdi, 8
        mov     DWORD PTR [rdi-4], edx
        cmp     rdi, rax
        jne     .L5
.L3:
        ret
.L12:
        ret
```

# Strength Reduction (modulo)

The modulo operation is a slow operation, because it is not a simple arithmetic operation.

It is possible to replace the modulo with a bitwise operation.

'Modulo 8' is the same as 'AND 7' for positive numbers.

## Non optimized code

[Link to Compiler Explorer](https://godbolt.org/z/vvTGvs79z)

```C
int classify(int x) {
    return x % 8;
}
```

```Assembly
classify:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], edi
        mov     edx, DWORD PTR [rbp-4]
        mov     eax, edx
        sar     eax, 31
        shr     eax, 29
        add     edx, eax
        and     edx, 7
        sub     edx, eax
        mov     eax, edx
        pop     rbp
        ret
```

## Optimized code

[Link to Compiler Explorer](https://godbolt.org/z/as49MTh4e)

```C
int classify(int x) {
    return x & 7;
}
```

```Assembly
classify:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], edi
        mov     eax, DWORD PTR [rbp-4]
        and     eax, 7
        pop     rbp
        ret
```

## Optimized code with -O2

[Link to Compiler Explorer](https://godbolt.org/z/vbaTfebov)

```Assembly
classify:
        mov     edx, edi
        sar     edx, 31
        shr     edx, 29
        lea     eax, [rdi+rdx]
        and     eax, 7
        sub     eax, edx
        ret
```

# Branch Elimination via Conditional Move (CMOV)

It is possible to eliminate branches by using a conditional move. The conditional move is an instruction that moves a value from one register to another only if a condition is met.

In -O0, it is possible to tell the compiler to use the conditional move by using a ternary operator.

## Non optimized code

[Link to Compiler Explorer](https://godbolt.org/z/5PYKxofbG)

```C
int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}
```

```Assembly
max:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], edi
        mov     DWORD PTR [rbp-8], esi
        mov     eax, DWORD PTR [rbp-4]
        cmp     eax, DWORD PTR [rbp-8]
        jle     .L2
        mov     eax, DWORD PTR [rbp-4]
        jmp     .L3
.L2:
        mov     eax, DWORD PTR [rbp-8]
.L3:
        pop     rbp
        ret
```

## Optimized code

[Link to Compiler Explorer](https://godbolt.org/z/vzxvr9hsa)

```C
int max(int a, int b) {
    return a > b ? a : b;
}
```

```Assembly
max:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], edi
        mov     DWORD PTR [rbp-8], esi
        mov     edx, DWORD PTR [rbp-8]
        mov     eax, DWORD PTR [rbp-4]
        cmp     edx, eax
        cmovge  eax, edx
        pop     rbp
        ret
```

## Optimized code with -O2

[Link to Compiler Explorer](https://godbolt.org/z/KKnb43K35)

```Assembly
max:
        cmp     edi, esi
        mov     eax, esi
        cmovge  eax, edi
        ret
```