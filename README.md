# k_dbm - Key-Value Database Manager

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/arasan90/k_dbm)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-99-blue.svg)](https://en.wikipedia.org/wiki/C99)

A lightweight, thread-safe key-value database manager written in C, designed for embedded systems and applications requiring both volatile (RAM) and non-volatile memory (NVM) storage capabilities.

## Features

- **Dual Storage Support**: Store data in either RAM or Non-Volatile Memory (NVM)
- **Thread-Safe**: Built-in mutex support for concurrent access
- **Configurable**: Customizable database size and value length limits
- **Lightweight**: Minimal memory footprint suitable for embedded systems
- **Zero Dependencies**: No external libraries required for core functionality
- **Caching**: Automatic caching of NVM entries in RAM for faster access
- **Mock Support**: Includes mock implementation for testing

## Architecture

The k_dbm library implements a hybrid storage architecture:

- **RAM Storage**: Fast access, volatile data
- **NVM Storage**: Persistent data with automatic RAM caching
- **Thread Safety**: Configurable mutex operations for multi-threaded environments

## Quick Start

### 1. Configuration

Define the database parameters at compile time:

```c
#define K_DBM_DB_SIZE 100              // Maximum number of entries
#define K_DBM_VALUE_MAX_LENGTH 256     // Maximum value length in bytes
```

### 2. Implementation

Implement the required callback functions:

```c
#include "k_dbm.h"

// Mutex operations
int my_mutex_lock(int timeout_ms) {
    // Your mutex lock implementation
    return 0; // Success
}

void my_mutex_unlock(void) {
    // Your mutex unlock implementation
}

// NVM operations
int my_nvm_insert(const char *key, const char *value) {
    // Your NVM insert implementation
    return 0; // Success
}

int my_nvm_get(const char *key, char *value, size_t value_buffer_size) {
    // Your NVM get implementation
    return 0; // Success
}

int my_nvm_delete(const char *key) {
    // Your NVM delete implementation
    return 0; // Success
}
```

### 3. Initialization

```c
k_dbm_config_t config = {
    .k_dbm_lock_mutex_f = my_mutex_lock,
    .k_dbm_unlock_mutex_f = my_mutex_unlock,
    .k_dbm_insert_f = my_nvm_insert,
    .k_dbm_get_f = my_nvm_get,
    .k_dbm_delete_f = my_nvm_delete
};

if (k_dbm_init(&config) == 0) {
    // Database manager initialized successfully
}
```

### 4. Usage

```c
// Insert data
k_dbm_insert("temperature", "25.5", K_DBM_STORAGE_RAM);
k_dbm_insert("device_id", "ESP32_001", K_DBM_STORAGE_NVM);

// Retrieve data
char value[K_DBM_VALUE_MAX_LENGTH];
if (k_dbm_get("temperature", value, sizeof(value)) == 0) {
    printf("Temperature: %s°C\n", value);
}

// Delete data
k_dbm_delete("temperature");

// Check free space
size_t free_entries = k_dbm_get_free_space();
printf("Free entries: %zu\n", free_entries);
```

## API Reference

### Core Functions

#### `k_dbm_init(const k_dbm_config_t *config_p)`
Initializes the database manager with the provided configuration.

**Parameters:**
- `config_p`: Pointer to configuration structure containing callback functions

**Returns:**
- `0` on success
- `-1` on failure (NULL config or missing callbacks)

#### `k_dbm_insert(const char *key_p, const char *value_p, k_dbm_storage_t storage)`
Inserts or updates a key-value pair.

**Parameters:**
- `key_p`: Entry key (null-terminated string)
- `value_p`: Entry value (null-terminated string)
- `storage`: Storage type (`K_DBM_STORAGE_RAM` or `K_DBM_STORAGE_NVM`)

**Returns:**
- `0` on success
- `-1` on failure

#### `k_dbm_get(const char *key_p, char *value_buffer_p, size_t value_buffer_size)`
Retrieves a value by key.

**Parameters:**
- `key_p`: Key to search for
- `value_buffer_p`: Buffer to store the retrieved value
- `value_buffer_size`: Size of the value buffer

**Returns:**
- `0` on success
- `-1` on failure (key not found or buffer too small)

#### `k_dbm_delete(const char *key_p)`
Deletes a key-value pair.

**Parameters:**
- `key_p`: Key to delete

**Returns:**
- `0` on success
- `-1` on failure

#### `k_dbm_get_free_space(void)`
Returns the number of free entries in the database.

**Returns:** Number of available entries

### Storage Types

```c
typedef enum {
    K_DBM_STORAGE_NONE,  // Internal use only
    K_DBM_STORAGE_RAM,   // Volatile storage
    K_DBM_STORAGE_NVM    // Non-volatile storage (also cached in RAM)
} k_dbm_storage_t;
```

## Building

### Using CMake

For development (includes tests):

```bash
mkdir build && cd build
cmake .. -DK_DBM_DEV=ON -DK_DBM_DB_SIZE=100 -DK_DBM_VALUE_MAX_LENGTH=256
make
```

### Integration

Include the library in your project using the provided CMake module:

```cmake
include(path/to/k_dbm.cmake)

k_dbm_get_sources(K_DBM_SOURCES)
k_dbm_get_public_headers(K_DBM_PUBLIC_HEADERS)

add_library(my_app ${K_DBM_SOURCES} main.c)
target_include_directories(my_app PRIVATE ${K_DBM_PUBLIC_HEADERS})
target_compile_definitions(my_app PRIVATE
    K_DBM_DB_SIZE=100
    K_DBM_VALUE_MAX_LENGTH=256
)
```

## Testing

The project includes comprehensive unit tests using Google Test framework:

```bash
# Build and run tests
cd build
make
ctest --verbose
```

Tests cover:
- Initialization scenarios
- Insert/Update operations
- Retrieval operations
- Delete operations
- Thread safety
- Storage type handling
- Error conditions

## Mock Library

For testing applications that use k_dbm, a mock library is provided using the [FFF (Fake Function Framework)](https://github.com/meekrosoft/fff):

```c
#include "k_dbm_mock.h"

// Use mocked functions in your tests
FAKE_VALUE_FUNC(int, k_dbm_init, const k_dbm_config_t*);
FAKE_VALUE_FUNC(int, k_dbm_insert, const char*, const char*, k_dbm_storage_t);
```

## Configuration Options

### Compile-Time Definitions

| Definition | Description | Required |
|------------|-------------|----------|
| `K_DBM_DB_SIZE` | Maximum number of database entries | Yes |
| `K_DBM_VALUE_MAX_LENGTH` | Maximum length of values in bytes | Yes |

### Runtime Configuration

The `k_dbm_config_t` structure must provide:

- `k_dbm_lock_mutex_f`: Mutex locking function
- `k_dbm_unlock_mutex_f`: Mutex unlocking function
- `k_dbm_insert_f`: NVM insert function
- `k_dbm_get_f`: NVM get function
- `k_dbm_delete_f`: NVM delete function

## Thread Safety

k_dbm is designed to be thread-safe when proper mutex implementations are provided. All operations are protected by the configured mutex functions.

## Limitations

- Keys must be persistent string pointers (they are not copied internally)
- Maximum database size is fixed at compile time
- No dynamic memory allocation
- Single database instance per application

## Contributing

1. Open a new issue
2. Create a branch
3. Add tests
4. Ensure all tests pass
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.