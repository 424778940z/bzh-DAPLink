#include "device_id.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ch32v30x.h"

#if !defined(get16bits)
  #define get16bits(d) ((((uint32_t)(((const uint8_t*)(d))[1])) << 8) + (uint32_t)(((const uint8_t*)(d))[0]))
#endif
uint32_t SuperFastHash(const uint8_t* data, int len)
{
    uint32_t hash = len, tmp;
    uint32_t rem;

    if ( len <= 0 || data == NULL )
        return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for ( ; len > 0; len-- )
    {
        hash += get16bits(data);
        tmp = (get16bits(data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    /* Handle end cases */
    switch ( rem )
    {
    case 3:
        hash += get16bits(data);
        hash ^= hash << 16;
        hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
        hash += hash >> 11;
        break;
    case 2:
        hash += get16bits(data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1:
        hash += (signed char)*data;
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

uint8_t device_id_get_hashed(uint8_t* buf, uint8_t buf_len)
{
    uint32_t IC_UID[3];
    IC_UID[0] = *(uint32_t*)0x1FFFF7E8;
    IC_UID[1] = *(uint32_t*)0x1FFFF7EC;
    IC_UID[2] = *(uint32_t*)0x1FFFF7F0;

    uint32_t IC_UID_HASH = SuperFastHash((const uint8_t*)IC_UID, sizeof(IC_UID));

    uint8_t uid_buf[sizeof(uint32_t) * 2 + 1] = {'\0'};

    if ( buf_len >= sizeof(uid_buf) )
    {
        sprintf((char*)uid_buf, "%08lX", IC_UID_HASH);
        memcpy(buf, uid_buf, sizeof(uid_buf));
        return sizeof(uid_buf);
    }

    return 0;
}