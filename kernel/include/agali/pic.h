#ifndef PIC_H
#define PIC_H

void pic_remap(int masterAddr, int slaveAddr);
void pic_enableIRQ(int irqIndex);
void pic_disableIRQ(int irqIndex);
void pic_disable(void);

void pic_sendEOI(int irqIndex, int wasSpurious);

/* 0: No
   1: Yes, by master
   2: Yes, by slave */
int pic_isIRQspurious(int irqIndex);

#endif // PIC_H
