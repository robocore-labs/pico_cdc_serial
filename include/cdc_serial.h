/*
 * A TinyUSB CDC interface presented as a serial_t.
 *
 * A composite USB device's CDC ports are byte pipes like any other, so
 * anything written against serial_t works over one: a zenoh link
 * (pico_zenoh), a passthrough to a UART, a console. The port number is the
 * interface index in the application's own USB descriptors -- which is why
 * this takes one rather than naming ports itself.
 *
 * This is the one library here that includes tusb.h, and deliberately so:
 * it exists to be the place USB stops. The application still owns
 * tusb_config.h, the descriptors and tusb_init(); this only moves bytes.
 *
 *     static cdc_serial_t zenoh_cdc;
 *     serial_t *link = cdc_serial_init(&zenoh_cdc, CDC_IDX_ZENOH);
 *
 * task() is tud_task(), so a driver blocking on this port keeps the whole
 * USB device serviced while it waits -- including the port it is waiting on.
 */

#ifndef CDC_SERIAL_H
#define CDC_SERIAL_H

#include "pico_serial.h"

// How long write() keeps pushing at a stalled host before giving up and
// reporting a short write. A host that has opened the port but stopped
// reading must not wedge the firmware.
#ifndef CDC_SERIAL_WRITE_TIMEOUT_MS
#define CDC_SERIAL_WRITE_TIMEOUT_MS 200
#endif

typedef struct {
    serial_t base;      // must be first
    uint8_t  itf;       // CDC interface index in the app's descriptors
} cdc_serial_t;

/*
 * Present CDC interface `itf` as a serial port. USB does not have to be up
 * yet -- reads return nothing and writes report short until a host attaches.
 * Returns NULL if p is NULL.
 */
serial_t *cdc_serial_init(cdc_serial_t *p, uint8_t itf);

// True once a host has the port open. A serial_t has no notion of this, so
// it is here for callers that want to skip work when nobody is listening.
bool cdc_serial_connected(serial_t *s);

#endif // CDC_SERIAL_H
