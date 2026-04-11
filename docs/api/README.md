# IDCU Agent Core Library API Documentation

This directory contains API documentation for the core libraries of the IDCU Agent project.

## Library Overview

### Core Infrastructure Libraries

- [idcu-common](common.md) - Common utilities and data structures
- [idcu-log](log.md) - Logging system
- [idcu-module-system](module-system.md) - Module system
- [idcu-coroutine](coroutine.md) - Coroutine scheduler
- [idcu-msgbus](msgbus.md) - Message bus
- [idcu-microkernel](microkernel.md) - Microkernel

### Data Processing Libraries

- [idcu-json](json.md) - JSON parser and serializer
- [idcu-yaml](yaml.md) - YAML parser and serializer
- [idcu-config](config.md) - Configuration management

### Network Libraries

- [idcu-network](network.md) - Network programming (TCP/UDP)
- [idcu-http-server](http-server.md) - HTTP server framework
- [idcu-http-client](http-client.md) - HTTP client framework

### Monitoring Libraries

- [idcu-metrics](metrics.md) - Metrics collection
- [idcu-healthcheck](healthcheck.md) - Health checking
- [idcu-alert](alert.md) - Alert management

## Usage Patterns

### Error Handling

All functions return `idcu_ErrorCode` to indicate success or failure:

```c
int ret = some_function();
if (ret != IDCU_ERR_OK) {
    // Handle error
    fprintf(stderr, "Error: %s\n", idcu_error_message(ret));
    return ret;
}
```

### Resource Management

Most components follow an init/destroy pattern:

```c
SomeType* obj = NULL;
ret = some_type_init(&obj, config);
if (ret != IDCU_ERR_OK) {
    // Handle error
    return ret;
}

// Use the object
some_type_operation(obj, ...);

// Cleanup
some_type_destroy(obj);
```

## License

This project is licensed under the MIT License.
