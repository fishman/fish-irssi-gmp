/*
 *   MIRACL compiler/hardware definitions - mirdef.h
 *
 *   sparc32 processor
 *
 *   Copyright (c) 1988-2001 Shamus Software Ltd.
 */

#define MIRACL 32
#define MR_BIG_ENDIAN
#define mr_utype int
#define MR_IBITS 32
#define MR_LBITS 32
#define mr_dltype long long
#define mr_unsign32 unsigned int
#define mr_unsign64 unsigned long long
#define MAXBASE ((mr_small)1<<(MIRACL-1))


#define MR_NOASM	// enable this if you have troubles with the inline-ASM parts

//#define MR_STRIPPED_DOWN
