#include <agali/pic.h>
#include <agali/portio.h>

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI		0x20

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIC_READ_IRR     0x0A    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR     0x0B    /* OCW3 irq service next CMD read */

static inline uint16 __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

void pic_remap(int masterAddr, int slaveAddr)
{
    unsigned char a1, a2;

	a1 = inb(PIC1_DATA);
	a2 = inb(PIC2_DATA);

	outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, masterAddr);
	io_wait();
	outb(PIC2_DATA, slaveAddr);
	io_wait();
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);
}

void pic_enableIRQ(int irqIndex)
{
    uint16 port;
    uint8 value;

    if(irqIndex < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irqIndex -= 8;
    }
    value = inb(port) & ~(1 << irqIndex);
    outb(port, value);
}

void pic_disableIRQ(int irqIndex)
{
    uint16 port;
    uint8 value;

    if(irqIndex < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irqIndex -= 8;
    }
    value = inb(port) | (1 << irqIndex);
    outb(port, value);
}

void pic_disable(void)
{
    outb(PIC2_DATA, 0xFF);
    outb(PIC1_DATA, 0xFF);
}

void pic_sendEOI(int irq, int wasSpurious)
{
    switch (wasSpurious)
    {
    case 0:
        if(irq >= 8) {
            outb(PIC2_COMMAND, PIC_EOI);
        }
        outb(PIC1_COMMAND,PIC_EOI);
	break;
	case 2:
        outb(PIC1_COMMAND,PIC_EOI);
    break;
    }
}

int pic_isIRQspurious(int irqIndex)
{
    if (irqIndex == 7) {
        return (__pic_get_irq_reg(PIC_READ_ISR) & 0x08) == 0;
    } else if (irqIndex == 15) {
        return (__pic_get_irq_reg(PIC_READ_ISR) & 0x80) == 0 ? 2 : 0;
    }
    return 0;
}
