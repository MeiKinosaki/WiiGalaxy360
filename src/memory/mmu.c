#include "wii/mmu.h"

void wii_mmu_init(wii_mmu_t *mmu)
{
    size_t i;
    if (!mmu) return;
    for(i=0;i<WII_MMU_TLB_ENTRIES;i++) mmu->entries[i].valid=false;
    for(i=0;i<16u;i++){mmu->segments[i].vsid=0;mmu->segments[i].ks=false;mmu->segments[i].kp=false;mmu->segments[i].no_execute=false;mmu->segments[i].valid=false;}
    for(i=0;i<4u;i++){mmu->ibat[i].valid=false;mmu->dbat[i].valid=false;}
    mmu->replacement=0; mmu->sdr1=0;
}
bool wii_mmu_tlb_map(wii_mmu_t *m,uint32_t ea,uint32_t pa,uint16_t asid,uint8_t p)
{
 size_t i,slot=WII_MMU_TLB_ENTRIES;
 if(!m||(ea&0xfffu)||(pa&0xfffu)||!p||(p&~7u))return false;
 for(i=0;i<WII_MMU_TLB_ENTRIES;i++){wii_mmu_tlb_entry_t *e=&m->entries[i];if(e->valid&&e->effective_page==ea&&e->asid==asid){slot=i;break;}if(!e->valid&&slot==WII_MMU_TLB_ENTRIES)slot=i;}
 if(slot==WII_MMU_TLB_ENTRIES){slot=m->replacement;m->replacement=(m->replacement+1u)%WII_MMU_TLB_ENTRIES;}
 m->entries[slot].effective_page=ea;m->entries[slot].physical_page=pa;m->entries[slot].asid=asid;m->entries[slot].permissions=p;m->entries[slot].valid=true;return true;
}
bool wii_mmu_tlb_unmap(wii_mmu_t *m,uint32_t ea,uint16_t asid)
{
 size_t i;if(!m||(ea&0xfffu))return false;for(i=0;i<WII_MMU_TLB_ENTRIES;i++){wii_mmu_tlb_entry_t *e=&m->entries[i];if(e->valid&&e->effective_page==ea&&e->asid==asid){e->valid=false;return true;}}return false;
}
bool wii_mmu_translate(const wii_mmu_t *m,uint32_t ea,uint16_t asid,uint8_t access,uint32_t *pa)
{
 size_t i;uint32_t page=ea&~0xfffu,off=ea&0xfffu;if(!m||!pa||(access!=1u&&access!=2u&&access!=4u))return false;
 for(i=0;i<WII_MMU_TLB_ENTRIES;i++){const wii_mmu_tlb_entry_t *e=&m->entries[i];if(e->valid&&e->effective_page==page&&e->asid==asid){if(!(e->permissions&access)||e->physical_page>UINT32_MAX-off)return false;*pa=e->physical_page+off;return true;}}return false;
}
bool wii_mmu_set_sdr1(wii_mmu_t *m,uint32_t v)
{
 uint32_t mask=v&0x1ffu,origin=v&0xffff0000u;if(!m||(origin&0x3fffu)!=0u||mask>0x1ffu)return false;m->sdr1=v;return true;
}
bool wii_mmu_set_segment(wii_mmu_t *m,unsigned n,uint32_t vsid,bool ks,bool kp,bool nx)
{
 if(!m||n>=16u||vsid>0xffffffu)return false;m->segments[n].vsid=vsid;m->segments[n].ks=ks;m->segments[n].kp=kp;m->segments[n].no_execute=nx;m->segments[n].valid=true;return true;
}
bool wii_mmu_set_segment_vsid(wii_mmu_t *m,unsigned n,uint32_t vsid){return wii_mmu_set_segment(m,n,vsid,true,true,false);}
bool wii_mmu_set_bat(wii_mmu_t *m,bool ins,unsigned n,uint32_t ea,uint32_t pa,uint32_t mask,bool sv,bool uv,uint8_t perms)
{
 wii_mmu_bat_t *b;if(!m||n>=4u||!sv&&!uv||!perms||(perms&~7u)||(mask&0x1ffffu)!=0x1ffffu)return false;b=ins?&m->ibat[n]:&m->dbat[n];b->effective_base=ea&~mask;b->physical_base=pa&~mask;b->block_mask=mask;b->permissions=perms;b->supervisor_valid=sv;b->user_valid=uv;b->valid=true;return true;
}
static bool walk_pteg(const wii_bus_t *bus,uint32_t base,uint32_t vsid,uint32_t api,bool secondary,uint32_t offset,uint8_t access,uint32_t *pa)
{
 unsigned slot;for(slot=0;slot<8u;slot++){uint32_t p0,p1,a=base+slot*8u;if(!wii_bus_read32be(bus,a,&p0)||!wii_bus_read32be(bus,a+4u,&p1))return false;
 if(!(p0&0x80000000u)||((p0>>7)&0xffffffu)!=vsid||(((p0>>6)&1u)!=0u)!=secondary||(p0&0x3fu)!=api)continue;
 {uint32_t pp=p1&3u;bool key=(p1&WII_MMU_PTE1_KEY)!=0u;bool r=pp!=0u,w=(pp==1u)||(!key&&pp==2u);if((access==WII_MMU_PERM_WRITE&&!w)||((access==WII_MMU_PERM_READ||access==WII_MMU_PERM_EXEC)&&!r))return false;}
 if(access==WII_MMU_PERM_WRITE)p1|=WII_MMU_PTE1_CHANGED;p1|=WII_MMU_PTE1_REFERENCED;if(!wii_bus_write32be((wii_bus_t *)bus,a+4u,p1))return false;*pa=(p1&0xfffff000u)|offset;return true;
 }return false;
}
bool wii_mmu_translate_bus_ex(const wii_mmu_t *m,wii_bus_t *bus,uint32_t ea,uint16_t asid,uint8_t access,bool user,uint32_t *pa)
{
 unsigned seg=ea>>28,i;uint32_t vsid,pi,api,hash,mask,origin,pteg;const wii_mmu_segment_t *sr;const wii_mmu_bat_t *bats;
 if(!m||!bus||!pa||(access!=1u&&access!=2u&&access!=4u))return false;
 if(wii_mmu_translate(m,ea,asid,access,pa))return true;
 bats=access==WII_MMU_PERM_EXEC?m->ibat:m->dbat;
 for(i=0;i<4u;i++){const wii_mmu_bat_t *b=&bats[i];if(b->valid&&(user?b->user_valid:b->supervisor_valid)&&((ea&~b->block_mask)==b->effective_base)&&(b->permissions&access)){*pa=b->physical_base|(ea&b->block_mask);return true;}}
 sr=&m->segments[seg];if(!sr->valid||(access==WII_MMU_PERM_EXEC&&sr->no_execute))return false;
 if(user?!sr->kp:!sr->ks)return false;
 vsid=sr->vsid;pi=(ea>>12)&0xffffu;api=(ea>>22)&0x3fu;hash=vsid^pi;mask=m->sdr1&0x1ffu;origin=m->sdr1&0xffff0000u;
 pteg=origin|((hash&mask)<<6);if(walk_pteg(bus,pteg,vsid,api,false,ea&0xfffu,access,pa))return true;
 pteg=origin|(((~hash)&mask)<<6);return walk_pteg(bus,pteg,vsid,api,true,ea&0xfffu,access,pa);
}
bool wii_mmu_translate_bus(const wii_mmu_t *m,wii_bus_t *b,uint32_t ea,uint16_t asid,uint8_t a,uint32_t *pa){return wii_mmu_translate_bus_ex(m,b,ea,asid,a,false,pa);}
bool wii_mmu_bus_read8(const wii_mmu_t*m,wii_bus_t*b,uint32_t ea,uint16_t a,uint8_t*v){uint32_t p;return v&&wii_mmu_translate_bus(m,b,ea,a,1,&p)&&wii_bus_read8(b,p,v);}
bool wii_mmu_bus_read16be(const wii_mmu_t*m,wii_bus_t*b,uint32_t ea,uint16_t a,uint16_t*v){uint32_t p;return v&&wii_mmu_translate_bus(m,b,ea,a,1,&p)&&wii_bus_read16be(b,p,v);}
bool wii_mmu_bus_write8(const wii_mmu_t*m,wii_bus_t*b,uint32_t ea,uint16_t a,uint8_t v){uint32_t p;return wii_mmu_translate_bus(m,b,ea,a,2,&p)&&wii_bus_write8(b,p,v);}
bool wii_mmu_bus_write16be(const wii_mmu_t*m,wii_bus_t*b,uint32_t ea,uint16_t a,uint16_t v){uint32_t p;return wii_mmu_translate_bus(m,b,ea,a,2,&p)&&wii_bus_write16be(b,p,v);}
bool wii_mmu_bus_read32be(const wii_mmu_t*m,wii_bus_t*b,uint32_t ea,uint16_t a,uint32_t*v){uint32_t p;return v&&wii_mmu_translate_bus(m,b,ea,a,1,&p)&&wii_bus_read32be(b,p,v);}
bool wii_mmu_bus_write32be(const wii_mmu_t*m,wii_bus_t*b,uint32_t ea,uint16_t a,uint32_t v){uint32_t p;return wii_mmu_translate_bus(m,b,ea,a,2,&p)&&wii_bus_write32be(b,p,v);}
