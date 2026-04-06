# IDCU Watchdog Module

Watchdog module for process monitoring and automatic restart.

## Features

- Heartbeat-based process monitoring
- Configurable timeout threshold
- Automatic restart on timeout
- Restart limit and window control
- Cross-platform support (Windows, Linux, macOS)
- Callback hooks for timeout and restart events

## Usage

```c
#include "idcu/watchdog/watchdog.h"

idcu_Watchdog wd;

// Initialize watchdog with 5s timeout, max 5 restarts in 60s window
idcu_watchdog_init(&wd, 5000, 5, 60000);

// Start watchdog
idcu_watchdog_start(&wd);

// Periodically send heartbeat
while (running) {
    idcu_watchdog_heartbeat(&wd);
    // Do work...
    sleep(1);
}

// Stop watchdog
idcu_watchdog_stop(&wd);
idcu_watchdog_destroy(&wd);
```

## API Reference

See `include/idcu/watchdog/watchdog.h` for complete API documentation.
