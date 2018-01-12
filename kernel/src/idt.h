#idndef IDT_H
#define IDT_H

#include "ktypes.h"

typedef struct {
   uint16 offset_1;
   uint16 selector;
   uint8 ist;
   uint8 type_attr;
   uint16 offset_2;
   uint32 offset_3;
   uint32 zero;
} idt_descr;



#endif // IDT_H
