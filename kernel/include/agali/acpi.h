#ifndef ACPI_H
#define ACPI_H

#include "ktypes.h"

#define ACPI_ASID_SYSMEM 0x0
#define ACPI_ASID_SYSIO  0x1
#define ACPI_ASID_PCI    0x2
#define ACPI_ASID_EMBCON 0x3
#define ACPI_ASID_SMBUS  0x4
#define ACPI_ASID_PCC    0xA
#define ACPI_ASID_FFH    0x7F

#define ACPI_ACSZ_UNDEF  0
#define ACPI_ACSZ_BYTE   1
#define ACPI_ACSZ_WORD   2
#define ACPI_ACSZ_DWORD  3
#define ACPI_ACSZ_QWORD  4

typedef struct __attribute__((packed)) {
    uint8 addrSpaceID;
    uint8 regBitWidth;
    uint8 regBitOffset;
    uint8 accessSize;
    uint64 address;
} GAS;

typedef struct __attribute__((packed)) {
    uint8 signature[4];
    uint32 length;
    uint8 revision;
    uint8 checksum;
    uint8 OEM_ID[6];
    uint8 OEM_tableID[8];
    uint32 OEMrevision;
    uint32 creatorID;
    uint32 creatorRevision;
} SDTheader;

BOOL acpi_init(void);

void* acpi_findTable(const char* headerSig);

#endif // ACPI_H
