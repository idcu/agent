# IDCU Module Isolation

Module isolation library for crash detection and recovery.

## Features

- Module crash detection and logging
- Automatic module restart with configurable limits
- Crash log retention
- Manual module management (restart/unload)
- Callback hooks for crash and restart events

## Usage

```c
#include "idcu/module_isolation/module_isolation.h"

idcu_ModuleIsolator isolator;

// Initialize isolator
idcu_isolator_init(&isolator);

// Register a module for isolation
idcu_isolator_register_module(&isolator, "my_module", module_handle, 3, 5000);

// Start monitoring
idcu_isolator_start_monitor(&isolator);

// If crash occurs, report it
idcu_isolator_report_crash(&isolator, "my_module", "Segmentation fault");

// Cleanup
idcu_isolator_stop_monitor(&isolator);
idcu_isolator_destroy(&isolator);
```

## API Reference

See `include/idcu/module_isolation/module_isolation.h` for complete API documentation.
