#include <agali/apic.h>
#include <agali/acpi.h>

static volatile uint32* LAPICbase;
static volatile uint32* IOAPICbases[8];

static int coreCount;
static int IOAPICcount;
