#include <assert.h>
#include <math.h>
#include "ppc/ppc_fpu.h"
int main(void){ppc_fpu_t f;ppc_fpu_reset(&f);f.fpr[1]=1.5;f.fpr[2]=2.25;ppc_fpu_add_single(&f,3,1,2);assert(f.fpr[3]==3.75);ppc_fpu_mul_double(&f,4,1,2);assert(f.fpr[4]==3.375);ppc_fpu_div_double(&f,5,2,1);assert(f.fpr[5]==1.5);assert(ppc_fpu_compare(&f,1,2,1)==8u);f.fpr[6]=NAN;assert(ppc_fpu_compare(&f,6,2,1)==0u);assert((f.fpscr&PPC_FPSCR_VX)!=0u);f.fpr[7]=0;ppc_fpu_div_single(&f,8,1,7);assert((f.fpscr&PPC_FPSCR_ZX)!=0u);return 0;}
