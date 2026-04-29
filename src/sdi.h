#ifndef SDI_H
#define SDI_H

#include <stdint.h>

void sdi_init(void);
void sdi_deinit(void);
void sdi_write(const char* buf, int size);

#endif // SDI_H
