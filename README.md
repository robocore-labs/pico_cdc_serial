# pico_cdc_serial

A TinyUSB CDC interface presented as a [`serial_t`](../pico_serial).

A composite device's CDC ports are byte pipes like any other, so anything
written against `serial_t` works over one — a zenoh link
([`pico_zenoh`](../pico_zenoh)), a passthrough to a UART, a console.

```cmake
add_subdirectory(path/to/link-libraries/pico_serial      pico_serial)
add_subdirectory(path/to/link-libraries/pico_cdc_serial  pico_cdc_serial)
target_link_libraries(my_firmware pico_cdc_serial)
```

```c
#include "cdc_serial.h"

static cdc_serial_t zenoh_cdc;
serial_t *link = cdc_serial_init(&zenoh_cdc, CDC_IDX_ZENOH);
```

`CDC_IDX_ZENOH` is yours: the index is an interface number in *your* USB
descriptors, which is why this library takes one rather than naming ports
itself.

## Where USB stops

This is the one library in this repo that includes `tusb.h`, and
deliberately so — it exists to be the place USB stops. Everything above it
sees a `serial_t`, so a zenoh port, a motor driver or a console can be
written, tested and reused without a USB stack anywhere in sight.

The application still owns the parts TinyUSB requires it to own:
`tusb_config.h`, the descriptors, `tusb_init()`, and calling `tud_task()`
from its main loop. Point the build at the first of those:

```cmake
set(CDC_SERIAL_TUSB_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR})  # holds tusb_config.h
```

It defaults to the top-level source directory, which is where the firmwares
in this family keep it.

## Behaviour worth knowing

**`task()` is `tud_task()`.** So a driver that blocks on this port keeps the
whole USB device serviced while it waits — including the port it is waiting
on, which is what makes a blocking read over CDC work at all.

**Writes are bounded.** `write()` pushes until the bytes are accepted or
`CDC_SERIAL_WRITE_TIMEOUT_MS` (200 ms) passes, then reports a short write. A
host that opens the port and stops reading stalls one frame, not the
firmware. With no host attached at all, `write()` returns 0 immediately.

**`set_baudrate()` is not implemented** and returns `false`: a CDC port has
no baud rate. The host's line-coding request is a separate thing — if your
firmware forwards it to a real UART, do that in `tud_cdc_line_coding_cb()`,
which is application glue.

**`cdc_serial_connected()`** is outside the `serial_t` interface, for
callers that want to skip work when nobody is listening.
