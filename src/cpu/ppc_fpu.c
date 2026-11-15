#include "ppc/ppc_fpu.h"
#include <math.h>
#include <stddef.h>
void ppc_fpu_reset(ppc_fpu_t *f){unsigned i;if(!f)return;for(i=0;i<32u;i++)f->fpr[i]=0.0;f->fpscr=0;}
static void flags(ppc_fpu_t *f,double x){if(!f)return;if(isnan(x)){f->fpscr|=PPC_FPSCR_VX|PPC_FPSCR_FX;}if(isinf(x)){f->fpscr|=PPC_FPSCR_OX|PPC_FPSCR_FX;}if(x==0.0)f->fpscr|=PPC_FPSCR_XX;}
void ppc_fpu_add_single(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){volatile float x;if(!f||d>=32||a>=32||b>=32)return;x=(float)f->fpr[a]+(float)f->fpr[b];f->fpr[d]=(double)x;flags(f,x);}
void ppc_fpu_sub_single(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){volatile float x;if(!f||d>=32||a>=32||b>=32)return;x=(float)f->fpr[a]-(float)f->fpr[b];f->fpr[d]=(double)x;flags(f,x);}
void ppc_fpu_mul_single(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){volatile float x;if(!f||d>=32||a>=32||b>=32)return;x=(float)f->fpr[a]*(float)f->fpr[b];f->fpr[d]=(double)x;flags(f,x);}
void ppc_fpu_div_single(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){volatile float x;if(!f||d>=32||a>=32||b>=32)return;if(f->fpr[b]==0.0)f->fpscr|=PPC_FPSCR_ZX|PPC_FPSCR_FX;x=(float)f->fpr[a]/(float)f->fpr[b];f->fpr[d]=(double)x;flags(f,x);}
void ppc_fpu_add_double(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){if(!f||d>=32||a>=32||b>=32)return;f->fpr[d]=f->fpr[a]+f->fpr[b];flags(f,f->fpr[d]);}
void ppc_fpu_sub_double(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){if(!f||d>=32||a>=32||b>=32)return;f->fpr[d]=f->fpr[a]-f->fpr[b];flags(f,f->fpr[d]);}
void ppc_fpu_mul_double(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){if(!f||d>=32||a>=32||b>=32)return;f->fpr[d]=f->fpr[a]*f->fpr[b];flags(f,f->fpr[d]);}
void ppc_fpu_div_double(ppc_fpu_t*f,unsigned d,unsigned a,unsigned b){if(!f||d>=32||a>=32||b>=32)return;if(f->fpr[b]==0.0)f->fpscr|=PPC_FPSCR_ZX|PPC_FPSCR_FX;f->fpr[d]=f->fpr[a]/f->fpr[b];flags(f,f->fpr[d]);}
unsigned ppc_fpu_compare(ppc_fpu_t*f,unsigned a,unsigned b,int ordered){double x,y;if(!f||a>=32||b>=32)return 0;if(isnan(f->fpr[a])||isnan(f->fpr[b])){if(ordered)f->fpscr|=PPC_FPSCR_VX|PPC_FPSCR_FX;return 0;}x=f->fpr[a];y=f->fpr[b];return x<y?8u:(x>y?4u:2u);}
