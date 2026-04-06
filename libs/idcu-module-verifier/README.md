# IDCU Module Verifier

Module verification library for DLL/SO hash and signature checking.

## Features

- File hash computation and verification
- Signature verification (placeholder for crypto integration)
- Hash file I/O operations
- Configurable verification types

## Usage

```c
#include "idcu/module_verifier/module_verifier.h"

idcu_ModuleVerifier verifier;

// Initialize verifier
idcu_verifier_init(&verifier);

// Compute hash of a module
uint8_t hash[IDCU_VERIFIER_HASH_SIZE];
idcu_verifier_compute_hash("mymodule.dll", hash, IDCU_VERIFIER_HASH_SIZE);

// Verify module using hash
idcu_verifier_verify_module_with_hash(&verifier, "mymodule.dll", hash);

// Cleanup
idcu_verifier_destroy(&verifier);
```

## API Reference

See `include/idcu/module_verifier/module_verifier.h` for complete API documentation.
