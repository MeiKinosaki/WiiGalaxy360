#ifndef PPC_FPU_H
#define PPC_FPU_H
#include <stdint.h>
typedef struct { double fpr[32]; uint32_t fpscr; } ppc_fpu_t;
#define PPC_FPSCR_FX 0x80000000u
#define PPC_FPSCR_FEX 0x40000000u
#define PPC_FPSCR_VX 0x20000000u
#define PPC_FPSCR_OX 0x10000000u
#define PPC_FPSCR_UX 0x08000000u
#define PPC_FPSCR_ZX 0x04000000u
#define PPC_FPSCR_XX 0x02000000u
void ppc_fpu_reset(ppc_fpu_t *fpu);
void ppc_fpu_add_single(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
void ppc_fpu_sub_single(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
void ppc_fpu_mul_single(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
void ppc_fpu_div_single(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
void ppc_fpu_add_double(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
void ppc_fpu_sub_double(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
void ppc_fpu_mul_double(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
void ppc_fpu_div_double(ppc_fpu_t *fpu,unsigned fd,unsigned fa,unsigned fb);
unsigned ppc_fpu_compare(ppc_fpu_t *fpu,unsigned fa,unsigned fb,int ordered);
#endif
