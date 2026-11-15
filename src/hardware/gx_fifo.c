#include "wii/gx_fifo.h"
void wii_gx_fifo_init(wii_gx_fifo_t*f){if(!f)return;f->head=0;f->tail=0;f->count=0;f->overflows=0;f->underflows=0;}
bool wii_gx_fifo_write(wii_gx_fifo_t*f,uint32_t v){if(!f)return false;if(f->count==WII_GX_FIFO_CAPACITY){f->overflows++;return false;}f->words[f->tail]=v;f->tail=(f->tail+1u)%WII_GX_FIFO_CAPACITY;f->count++;return true;}
bool wii_gx_fifo_read(wii_gx_fifo_t*f,uint32_t*v){if(!f||!v)return false;if(!f->count){f->underflows++;return false;}*v=f->words[f->head];f->head=(f->head+1u)%WII_GX_FIFO_CAPACITY;f->count--;return true;}
size_t wii_gx_fifo_count(const wii_gx_fifo_t*f){return f?f->count:0u;}
bool wii_gx_fifo_empty(const wii_gx_fifo_t*f){return !f||f->count==0u;}
bool wii_gx_fifo_full(const wii_gx_fifo_t*f){return f&&f->count==WII_GX_FIFO_CAPACITY;}
