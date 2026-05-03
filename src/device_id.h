#ifndef DEVICE_ID_H
#define DEVICE_ID_H

#include <stdint.h>

uint8_t device_id_get_hashed(uint8_t* buf, uint8_t buf_len);

#endif // DEVICE_ID_H