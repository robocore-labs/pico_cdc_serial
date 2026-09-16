#include "cdc_serial.h"

#include "pico/time.h"
#include "tusb.h"

static uint32_t cdc_write(serial_t *s, const uint8_t *src, uint32_t len) {
    cdc_serial_t *p = (cdc_serial_t *)s;

    // Nobody attached: report the short write rather than spinning until the
    // timeout on every frame. The port stays usable when a host shows up.
    if (!tud_cdc_n_connected(p->itf)) {
        return 0;
    }

    uint32_t written = 0;
    absolute_time_t deadline = make_timeout_time_ms(CDC_SERIAL_WRITE_TIMEOUT_MS);
    while (written < len) {
        written += tud_cdc_n_write(p->itf, &src[written], len - written);
        tud_cdc_n_write_flush(p->itf);
        if (written < len) {
            if (time_reached(deadline)) {
                break;
            }
            tud_task();  // drain the endpoint so the FIFO frees up
        }
    }
    tud_cdc_n_write_flush(p->itf);
    return written;
}

static uint32_t cdc_read(serial_t *s, uint8_t *dst, uint32_t max_len) {
    cdc_serial_t *p = (cdc_serial_t *)s;
    uint32_t avail = tud_cdc_n_available(p->itf);
    if (avail == 0 || max_len == 0) {
        return 0;
    }
    return tud_cdc_n_read(p->itf, dst, (max_len < avail) ? max_len : avail);
}

static uint32_t cdc_available(serial_t *s) {
    return tud_cdc_n_available(((cdc_serial_t *)s)->itf);
}

static void cdc_task(serial_t *s) {
    (void)s;
    tud_task();  // services the whole device, this interface included
}

static const serial_type_t cdc_serial_type = {
    .write        = cdc_write,
    .read         = cdc_read,
    .available    = cdc_available,
    .task         = cdc_task,
    .set_baudrate = NULL,  // the host sets the line coding; it means nothing here
};

serial_t *cdc_serial_init(cdc_serial_t *p, uint8_t itf) {
    if (!p) return NULL;
    p->base.type = &cdc_serial_type;
    p->itf = itf;
    return &p->base;
}

bool cdc_serial_connected(serial_t *s) {
    return tud_cdc_n_connected(((cdc_serial_t *)s)->itf);
}
