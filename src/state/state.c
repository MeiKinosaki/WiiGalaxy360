#include "wii/state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATE_HEADER_SIZE 24u
#define STATE_CPU_SIZE 172u
#define STATE_VERSION 2u

static const uint8_t state_magic[8] = {'W','G','3','6','0','S','T','1'};

static void put32(uint8_t *p, uint32_t v)
{
    p[0]=(uint8_t)(v>>24); p[1]=(uint8_t)(v>>16);
    p[2]=(uint8_t)(v>>8); p[3]=(uint8_t)v;
}
static uint32_t get32(const uint8_t *p)
{
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|
           ((uint32_t)p[2]<<8)|(uint32_t)p[3];
}
static uint32_t hash_bytes(uint32_t h, const uint8_t *p, size_t n)
{
    size_t i;
    for (i=0;i<n;++i) { h ^= p[i]; h *= 16777619u; }
    return h;
}
static void encode_cpu(uint8_t *p, const ppc_cpu_t *cpu)
{
    unsigned i;
    for (i=0;i<32u;++i,p+=4) put32(p,cpu->gpr[i]);
    put32(p,cpu->pc); p+=4; put32(p,cpu->lr); p+=4;
    put32(p,cpu->ctr); p+=4; put32(p,cpu->cr); p+=4;
    put32(p,cpu->alu.xer_ca); p+=4;
    put32(p,cpu->srr0); p+=4; put32(p,cpu->srr1); p+=4;
    put32(p,cpu->dar); p+=4; put32(p,cpu->dsisr); p+=4;
    put32(p,cpu->exception_vector); p+=4;
    put32(p,cpu->exception_pending);
}
static void decode_cpu(ppc_cpu_t *cpu, const uint8_t *p)
{
    unsigned i;
    for (i=0;i<32u;++i,p+=4) cpu->gpr[i]=get32(p);
    cpu->pc=get32(p); p+=4; cpu->lr=get32(p); p+=4;
    cpu->ctr=get32(p); p+=4; cpu->cr=get32(p); p+=4;
    cpu->alu.xer_ca=get32(p); p+=4;
    cpu->srr0=get32(p); p+=4; cpu->srr1=get32(p); p+=4;
    cpu->dar=get32(p); p+=4; cpu->dsisr=get32(p); p+=4;
    cpu->exception_vector=get32(p); p+=4;
    cpu->exception_pending=get32(p);
}
static bool valid_args(const char *path, const ppc_cpu_t *cpu,
                       const uint8_t *m1, size_t n1,
                       const uint8_t *m2, size_t n2)
{
    return path != NULL && cpu != NULL && m1 != NULL && m2 != NULL &&
           n1 <= UINT32_MAX && n2 <= UINT32_MAX;
}

bool wii_state_save(const char *path, const ppc_cpu_t *cpu,
                    const uint8_t *mem1, size_t mem1_size,
                    const uint8_t *mem2, size_t mem2_size)
{
    uint8_t header[STATE_HEADER_SIZE], registers[STATE_CPU_SIZE];
    uint32_t hash=2166136261u;
    FILE *f;
    if (!valid_args(path,cpu,mem1,mem1_size,mem2,mem2_size)) return false;
    encode_cpu(registers,cpu);
    hash=hash_bytes(hash,registers,sizeof registers);
    hash=hash_bytes(hash,mem1,mem1_size);
    hash=hash_bytes(hash,mem2,mem2_size);
    memcpy(header,state_magic,sizeof state_magic);
    put32(header+8,STATE_VERSION);
    put32(header+12,(uint32_t)mem1_size);
    put32(header+16,(uint32_t)mem2_size);
    put32(header+20,hash);
    f=fopen(path,"wb");
    if (f==NULL) return false;
    if (fwrite(header,1,sizeof header,f)!=sizeof header ||
        fwrite(registers,1,sizeof registers,f)!=sizeof registers ||
        fwrite(mem1,1,mem1_size,f)!=mem1_size ||
        fwrite(mem2,1,mem2_size,f)!=mem2_size) {
        (void)fclose(f); return false;
    }
    return fclose(f)==0;
}

bool wii_state_load(const char *path, ppc_cpu_t *cpu,
                    uint8_t *mem1, size_t mem1_size,
                    uint8_t *mem2, size_t mem2_size)
{
    uint8_t header[STATE_HEADER_SIZE], registers[STATE_CPU_SIZE];
    uint8_t *temp1=NULL,*temp2=NULL;
    uint32_t expected,hash=2166136261u;
    FILE *f=NULL;
    bool ok=false;
    if (!valid_args(path,cpu,mem1,mem1_size,mem2,mem2_size)) return false;
    f=fopen(path,"rb");
    if (f==NULL) return false;
    if (fread(header,1,sizeof header,f)!=sizeof header ||
        memcmp(header,state_magic,sizeof state_magic)!=0 ||
        get32(header+8)!=STATE_VERSION ||
        get32(header+12)!=(uint32_t)mem1_size ||
        get32(header+16)!=(uint32_t)mem2_size) goto done;
    expected=get32(header+20);
    temp1=(uint8_t *)malloc(mem1_size ? mem1_size : 1u);
    temp2=(uint8_t *)malloc(mem2_size ? mem2_size : 1u);
    if (temp1==NULL || temp2==NULL) goto done;
    if (fread(registers,1,sizeof registers,f)!=sizeof registers ||
        fread(temp1,1,mem1_size,f)!=mem1_size ||
        fread(temp2,1,mem2_size,f)!=mem2_size) goto done;
    if (fgetc(f)!=EOF) goto done;
    hash=hash_bytes(hash,registers,sizeof registers);
    hash=hash_bytes(hash,temp1,mem1_size);
    hash=hash_bytes(hash,temp2,mem2_size);
    if (hash!=expected) goto done;
    decode_cpu(cpu,registers);
    memcpy(mem1,temp1,mem1_size); memcpy(mem2,temp2,mem2_size);
    ok=true;
done:
    if (f!=NULL) (void)fclose(f);
    free(temp1); free(temp2);
    return ok;
}
