#include <assert.h>
#include "wii/gx_fifo.h"
int main(void){wii_gx_fifo_t f;uint32_t v;unsigned i;wii_gx_fifo_init(&f);assert(wii_gx_fifo_empty(&f));assert(!wii_gx_fifo_read(&f,&v));assert(f.underflows==1);for(i=0;i<WII_GX_FIFO_CAPACITY;i++)assert(wii_gx_fifo_write(&f,0x1000u+i));assert(wii_gx_fifo_full(&f));assert(!wii_gx_fifo_write(&f,0));assert(f.overflows==1);for(i=0;i<WII_GX_FIFO_CAPACITY;i++){assert(wii_gx_fifo_read(&f,&v));assert(v==0x1000u+i);}assert(wii_gx_fifo_empty(&f));return 0;}
