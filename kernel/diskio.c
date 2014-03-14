#include "diskio.h"
#include "string.h"
#include "SDI.h"
#include "syscalls.h"

DSTATUS disk_initialize(BYTE drv)
{
    return 0;
}

DSTATUS disk_status(BYTE drv)
{
    return 0;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
    s32 Retry = 10;

    while (1) {
        if (sdio_ReadSectors(sector, count, buff)) {
            break;
        }

        Retry--;

        if (Retry < 0) {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

// Write Sector(s)
DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{
    u8 *buffer = (u8 *) heap_alloc_aligned(0, count * 512, 0x40);

    memcpy(buffer, (void *) buff, count * 512);

    _ahbMemFlush(0xA);

    if (sdio_WriteSectors(sector, count, buffer) < 0) {
        return RES_ERROR;
    }

    heap_free(0, buffer);

    return RES_OK;
}
