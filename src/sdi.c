#include "sdi.h"

#include <stdint.h>

#include "systick.h"

// SDI debug data registers
#define SDI_DATA0_ADDRESS ((volatile uint32_t*)0xE0000380)
#define SDI_DATA1_ADDRESS ((volatile uint32_t*)0xE0000384)

void sdi_init(void)
{
    *(SDI_DATA0_ADDRESS) = 0;
}

void sdi_deinit(void)
{
    // nothing to deinit for SDI path
}

void sdi_write(const char* buf, int size)
{
    int offset = 0;
    while ( offset < size )
    {
        int chunk = size - offset;
        if ( chunk > 7 )
        {
            chunk = 7;
        }

        while ( *(SDI_DATA0_ADDRESS) != 0u ) {}

        uint32_t data0 = (uint32_t)chunk;
        for ( int idx = 0; idx < 3 && idx < chunk; idx++ )
        {
            data0 |= ((uint32_t)(uint8_t)buf[offset + idx]) << (8 * (idx + 1));
        }

        uint32_t data1 = 0;
        for ( int idx = 0; idx < 4 && (idx + 3) < chunk; idx++ )
        {
            data1 |= ((uint32_t)(uint8_t)buf[offset + 3 + idx]) << (8 * idx);
        }

        *(SDI_DATA1_ADDRESS) = data1;
        *(SDI_DATA0_ADDRESS) = data0;

        offset += chunk;
    }
}
