#ifndef APIC_H
#define APIC_H

void apic_init(void);
void apic_enable(void);

int apic_getCoreCount(void);
int apic_getIOAPICcount(void);

void apic_startTimer(int freq);

#endif // APIC_H
