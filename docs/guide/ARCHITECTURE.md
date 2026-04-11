# Architecture Guide

This document describes the architecture of IDCU Agent.

## Overview

IDCU Agent is built using a **microkernel architecture** with modular design. The system consists of:

1. **Core Infrastructure** - foundational libraries
2. **Microkernel** - module lifecycle management
3. **Business Modules** - feature implementations

## System Architecture Diagram

```
┌─────────────────────────────────────────────────────────┐
│                     Business Modules                     │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌───────┐ │
│  │ Core     │  │ HTTP     │  │ Metrics  │  │ ...   │ │
│  │ Module   │  │ API      │  │ Module   │  │       │ │
│  └──────────┘  └──────────┘  └──────────┘  └───────┘ │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│                     Microkernel                          │
│  ┌─────────────────────────────────────────────────┐  │
│  │  Module Lifecycle  |  Message Bus  |  Config   │  │
│  └─────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│                  Core Infrastructure                    │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌───────────┐ │
│  | Common  | |  Log    | |  JSON   | |  Network  | │
│  └─────────┘ └─────────┘ └─────────┘ └───────────┘ │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌───────────┐ │
│  |  YAML   | |  Config | | Storage | | Coroutine | │
│  └─────────┘ └─────────┘ └─────────┘ └───────────┘ │
└─────────────────────────────────────────────────────────┘
```

## Core Components

### 1. Microkernel Architecture

The microkernel is responsible for:

- **Module Lifecycle Management**: Loading, initializing, starting, stopping, and unloading modules
- **Dependency Resolution**: Managing module dependencies and ensuring correct startup order
- **Message Routing**: Facilitating communication between modules via the message bus
- **Configuration Management**: Providing centralized configuration access

### 2. Module System

```c
// Module definition
typedef struct idcu_ModuleDef {
    const char* name;
    const char* version;
    
    // Lifecycle callbacks
    idcu_ErrorCode (*init)(void);
    idcu_ErrorCode (*start)(void);
    idcu_ErrorCode (*stop)(void);
    idcu_ErrorCode (*destroy)(void);
    
    // Dependencies
    const char** dependencies;
    size_t dependency_count;
} idcu_ModuleDef;
```

### 3. Message Bus

Publish-subscribe system for inter-module communication:

- **Topics**: Message categorization
- **Subscribers**: Message consumers
- **Publishers**: Message producers
- **Priorities**: Message priority levels (LOW, NORMAL, HIGH, CRITICAL)

## Library Architecture

### Library Organization

```
libs/
├── idcu-common/          # Common utilities and data structures
├── idcu-log/             # Logging system
├── idcu-json/            # JSON parser/serializer
├── idcu-yaml/            # YAML parser/serializer
├── idcu-network/         # TCP/UDP network library
├── idcu-http-server/     # HTTP server framework
├── idcu-http-client/     # HTTP client library
├── idcu-config/          # Configuration management
├── idcu-storage/         # Key-value storage
├── idcu-metrics/         # Metrics collection
├── idcu-healthcheck/     # Health checking
├── idcu-module-system/   # Module system
├── idcu-coroutine/       # Coroutine scheduler
├── idcu-msgbus/          # Message bus
└── idcu-microkernel/     # Microkernel
```

### Key Design Principles

1. **Modularity**: Each library is independent with clear interfaces
2. **Minimal Dependencies**: Libraries depend only on lower-level libraries
3. **Thread Safety**: All public APIs are thread-safe
4. **Error Handling**: Consistent error code system
5. **Performance**: Optimized for both speed and memory

## Data Flow

### Module Communication Flow

```
Module A                     Message Bus                   Module B
   |                              |                              |
   |-- publish(topic, data) ---->|                              |
   |                              |-- dispatch() -------------->|
   |                              |                              |-- handle_msg()
   |                              |                              |
```

### Request-Response Pattern

While the message bus is primarily publish-subscribe, request-response can be implemented using:

1. Correlation IDs
2. Reply-to topics
3. Async patterns with callbacks

## Concurrency Model

The system uses a hybrid concurrency approach:

1. **Coroutines**: For cooperative multitasking within modules
2. **Threads**: For module isolation and I/O operations
3. **Message Passing**: For safe inter-thread communication

### Thread Safety Guarantees

- All public APIs are thread-safe
- Internal state is protected by mutexes
- Message passing is lock-free where possible

## Configuration

Configuration is managed through `idcu-config` library with support for:

- INI format (default)
- JSON format
- YAML format
- Environment variable overrides
- Command-line overrides
- Hot reloading
- Change notifications

## Monitoring & Observability

### Metrics (idcu-metrics)

- Counter: Incrementing values
- Gauge: Snapshot values
- Histogram: Distribution statistics
- Prometheus export support

### Health Checks (idcu-healthcheck)

- HTTP endpoint checks
- TCP port checks
- Disk space checks
- Memory usage checks
- Custom checkers

### Logging (idcu-log)

- Multiple log levels (DEBUG, INFO, WARN, ERROR, FATAL)
- Console and file output
- Color support
- Structured logging

## Extension Points

The system is designed for extensibility:

1. **Custom Modules**: Implement `idcu_ModuleDef` interface
2. **Custom Metrics**: Extend metrics registry
3. **Custom Health Checks**: Implement custom checkers
4. **Custom Storage Backends**: Implement storage interface
5. **Custom Serialization**: Add new config format parsers

## Deployment Architecture

### Process Model

```
idcu-agent (main process)
├── Thread: Microkernel (main loop)
├── Thread: Module A (worker)
├── Thread: Module B (worker)
├── Thread: HTTP Server
└── Thread: Metrics Collector
```

### Container Deployment

See [Dockerfile](../../Dockerfile) for containerization setup.

## Security Considerations

1. **Input Validation**: All inputs are validated
2. **Memory Safety**: No buffer overflows, proper bounds checking
3. **Authentication/Authorization**: Pluggable auth system
4. **Audit Logging**: Critical operations are logged
