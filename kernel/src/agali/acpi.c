#include <agali/acpi.h>
#include <agali/paging.h>
#include <string.h>

typedef struct __attribute__((packed)) {
    uint8 signature[8]; // "RSD PTR "
    uint8 checksum;
    uint8 OEMID[6];
    uint8 revision;
    uint32 rsdtAddr;
    uint32 length;  // The sizeof(RSDP)
    uint64 xsdtAddr;
    uint8 extChecksum;
    uint8 _res[3];
} RSDP;

static RSDP rootSysDescrPtr;

static int checksum(RSDP* rspd)
{
    int i, sum = 0, exSum = 0;
    int len = rspd->length;
    uint8* rspdPtr = (uint8*)rspd;

    len = (len > sizeof(RSDP) ? sizeof(RSDP) : len);
    for (i = 0; i < 20; ++i) {
        sum += rspdPtr[i];
        exSum += rspdPtr[i];
    }
    for (; i < len; ++i) {
        exSum += rspdPtr[i];
    }

    return (sum & 0xFF) + (exSum & 0xFF);
}

static int checksum2(SDTheader* hdr)
{
    int i, len, sum = 0;

    len = hdr->length;
    for (i = 0; i < len; ++i) {
        sum += ((uint8*)hdr)[i];
    }

    return sum & 0xFF;
}

BOOL acpi_init(void)
{
    uint64 i;

    for (i = 0x500; i < 0x100000; i += 16) {
        if (strncmp((char*)i, "RSD PTR ", 8) == 0) {
            if (checksum((RSDP*)i) == 0) {
                rootSysDescrPtr = *((RSDP*)i);
                return TRUE;
            }
        }
    }
    return FALSE;
}

void* acpi_findTable(const char* headerSig)
{
    SDTheader* root;
    volatile SDTheader* current = NULL;

    root = paging_alloc(1, TRUE, TRUE);
    paging_map(root, (void*)rootSysDescrPtr.xsdtAddr, TRUE, TRUE);
    current = paging_alloc(1, TRUE, TRUE);

    if (strncmp((char*)root->signature, "XSDT", 4) == 0 && checksum2(root) == 0) {
        uint64* descrBase = (uint64*)(root + 1);
        uint32 i, descrAmount = (root->length - sizeof(SDTheader))/sizeof(uint64);

        for (i = 0; i < descrAmount; ++i) {
            paging_map((void*)current, (void*)descrBase[i], TRUE, TRUE);
            current = (void*)(((intptr)current & ~0xFFFUL) | (descrBase[i] & 0xFFFUL));

            if (strncmp((char*)current->signature, headerSig, 4) == 0 && checksum2((void*)current) == 0) {
                break;
            }

            paging_unmap((void*)current, FALSE);
        }
        if (i >= descrAmount) {
            paging_unmap((void*)current, i == 0);
            current = NULL;
        }
    }

    paging_unmap(root, FALSE);

    return (void*)current;
}
