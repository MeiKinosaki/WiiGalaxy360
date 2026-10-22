#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "wii/state.h"

int main(void)
{
    ppc_cpu_t cpu, loaded;
    uint8_t mem1[128], mem2[64], old1[128], old2[64];
    FILE *f;
    int c;
    memset(mem1,0x31,sizeof mem1); memset(mem2,0x72,sizeof mem2);
    ppc_cpu_reset(&cpu,0x80003100u);
    cpu.gpr[3]=0x12345678u; cpu.lr=0x80003200u; cpu.ctr=9u;
    cpu.cr=0xdeadbeefu; cpu.alu.xer_ca=1u;
    assert(wii_state_save("test_state.bin",&cpu,mem1,sizeof mem1,mem2,sizeof mem2));
    memset(&loaded,0,sizeof loaded); memset(mem1,0,sizeof mem1); memset(mem2,0,sizeof mem2);
    assert(wii_state_load("test_state.bin",&loaded,mem1,sizeof mem1,mem2,sizeof mem2));
    assert(loaded.pc==0x80003100u && loaded.gpr[3]==0x12345678u);
    assert(loaded.lr==0x80003200u && loaded.ctr==9u && loaded.cr==0xdeadbeefu);
    assert(loaded.alu.xer_ca==1u && mem1[0]==0x31u && mem2[63]==0x72u);
    memcpy(old1,mem1,sizeof mem1); memcpy(old2,mem2,sizeof mem2);
    f=fopen("test_state.bin","r+b"); assert(f!=NULL);
    assert(fseek(f,24+148+3,SEEK_SET)==0);
    c=fgetc(f); assert(c!=EOF);
    assert(fseek(f,-1,SEEK_CUR)==0); assert(fputc(c^0xff,f)!=EOF); assert(fclose(f)==0);
    loaded.pc=0xabcdef00u;
    assert(!wii_state_load("test_state.bin",&loaded,mem1,sizeof mem1,mem2,sizeof mem2));
    assert(loaded.pc==0xabcdef00u);
    assert(memcmp(mem1,old1,sizeof mem1)==0 && memcmp(mem2,old2,sizeof mem2)==0);
    assert(remove("test_state.bin")==0);
    puts("Wii save-state tests: PASS");
    return 0;
}
