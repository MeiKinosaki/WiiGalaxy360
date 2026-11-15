#ifndef WII_GX_FIFO_H
#define WII_GX_FIFO_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define WII_GX_FIFO_CAPACITY 256u
typedef struct { uint32_t words[WII_GX_FIFO_CAPACITY]; size_t head,tail,count; uint32_t overflows,underflows; } wii_gx_fifo_t;
void wii_gx_fifo_init(wii_gx_fifo_t *fifo);
bool wii_gx_fifo_write(wii_gx_fifo_t *fifo,uint32_t word);
bool wii_gx_fifo_read(wii_gx_fifo_t *fifo,uint32_t *word);
size_t wii_gx_fifo_count(const wii_gx_fifo_t *fifo);
bool wii_gx_fifo_empty(const wii_gx_fifo_t *fifo);
bool wii_gx_fifo_full(const wii_gx_fifo_t *fifo);
#endif
