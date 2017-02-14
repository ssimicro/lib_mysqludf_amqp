#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_LIBBSD == 1
#include <bsd/stdlib.h>
#endif /* HAVE_LIBBSD */

#define SSI_UUID_BUF_LEN 41

/*
 * simple implementation of version 4 UUID generation
 */
void
ssiuuidgen(char *buffer)
{
    int i;
    int pos;

    memset(buffer, '\0', SSI_UUID_BUF_LEN);
    for (i = 0, pos = 0; i < 16 && pos < SSI_UUID_BUF_LEN - 2; i++) {

        uint8_t r = (uint8_t) arc4random();

        /* set some special bits */
        if (i == 6) {
            r = (uint8_t) ((r & 0x0F) | 0x40); /* set version number */
        } else if (i == 8) {
            r = (uint8_t) ((r & 0x3F) | 0x80); /* set reserved to b01 */
        }

        /* insert '-' where needed */
        if (pos == 8 || pos == 13 || pos == 18 || pos == 23) {
            pos += snprintf(buffer + pos, SSI_UUID_BUF_LEN - pos, "-");
        }

        /* print hex characters */
        pos += snprintf(buffer + pos, SSI_UUID_BUF_LEN - pos, "%2.2x", r);
    }
}
