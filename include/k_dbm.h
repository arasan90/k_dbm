/**
 * @brief Database Manager protocol header file
 * @addtogroup k_dbm
 * @{
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* Include -------------------------------------------------------------------*/
#include <stddef.h>

/* Macro ---------------------------------------------------------------------*/
#define K_DBM_LOCK_MUTEX_INFINITE_TIMEOUT (-1)

/* Typedef -------------------------------------------------------------------*/
/**
 * @brief Enum for storage types used in the database manager
 *
 * This enum defines the types of storage that can be used by the database manager.
 * - K_DBM_STORAGE_RAM: Use RAM for storage
 * - K_DBM_STORAGE_NVM: Use Non-Volatile Memory for storage
 */
typedef enum
{
	K_DBM_STORAGE_NONE,	 //!< Internal use
	K_DBM_STORAGE_NVM,	 //!< Use Non-Volatile Memory for storage. Entries in NVM are also cached in RAM
	K_DBM_STORAGE_RAM,	 //!< Use RAM for storage
} k_dbm_storage_t;

/**
 * @brief Function pointer type for locking a mutex with a timeout
 *
 * @param timeout_ms Timeout in milliseconds, use K_DBM_LOCK_MUTEX_INFINITE_TIMEOUT for infinite wait
 *
 * @return Returns 0 on success, -1 on failure
 */
typedef int (*k_dbm_lock_mutex_t)(int timeout_ms);

/**
 * @brief Function pointer type for unlocking a mutex
 *
 * @return Returns 0 on success, -1 on failure
 */
typedef void (*k_dbm_unlock_mutex_t)(void);

/**
 * @brief Function pointer type for inserting a key-value pair into the database
 *
 * @param key The key to insert
 * @param value The value associated with the key
 *
 * @return Returns 0 on success, -1 on failure
 */
typedef int (*k_dbm_insert_t)(const char *key, const char *value);

/**
 * @brief Function pointer type for retrieving a value by key from the database
 *
 * @param key The key to retrieve
 * @param value Pointer to store the retrieved value
 * @param value_buffer_size Size of the buffer to store the value
 *
 * @return Returns 0 on success, -1 on failure
 */
typedef int (*k_dbm_get_t)(const char *key, char *value, size_t value_buffer_size);

/**
 * @brief Function pointer type for deleting a key-value pair from the database
 *
 * @param key The key to delete
 *
 * @return Returns 0 on success, -1 on failure
 */
typedef int (*k_dbm_delete_t)(const char *key);

/**
 * @brief Configuration structure for the database manager
 *
 * This structure contains function pointers for mutex operations and database operations.
 */
typedef struct
{
	k_dbm_lock_mutex_t	 k_dbm_lock_mutex_f;	//!< Function pointer for locking a mutex
	k_dbm_unlock_mutex_t k_dbm_unlock_mutex_f;	//!< Function pointer for unlocking a mutex
	k_dbm_insert_t		 k_dbm_insert_f;		//!< Function pointer for inserting a key-value pair
	k_dbm_get_t			 k_dbm_get_f;			//!< Function pointer for retrieving a value by key
	k_dbm_delete_t		 k_dbm_delete_f;		//!< Function pointer for deleting a key-value pair
} k_dbm_config_t;

/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/**
 * @brief Initializes the Database Manager with the provided configuration
 *
 * This function initializes the Database Manager by setting up the required function pointers
 * for mutex operations and database operations. It validates that all required function
 * pointers are provided in the configuration structure.
 *
 * @param config_p Pointer to a k_dbm_config_t structure containing the following function pointers:
 *                 - k_dbm_lock_mutex_f: Function for mutex locking
 *                 - k_dbm_unlock_mutex_f: Function for mutex unlocking
 *                 - k_dbm_insert_f: Function for inserting key-value pairs
 *                 - k_dbm_get_f: Function for retrieving values by key
 *                 - k_dbm_delete_f: Function for deleting key-value pairs
 *
 * @note Configuration will be copied
 * @return Returns 0 on successful initialization
 *         Returns -1 if:
 *         - config_p is NULL
 *         - Any of the required function pointers in config_p is NULL
 *
 * @note All function pointers in the configuration structure must be valid (non-NULL)
 *       for successful initialization.
 */

int k_dbm_init(const k_dbm_config_t *config_p);

/**
 * @brief Insert a key-value pair entry into the DB
 *
 * @param key_p Entry key
 * @param value_p Entry value
 * @param storage Storage where the pair will be saved
 * @return 0 in case of success, -1 otherwise
 */
int k_dbm_insert(const char *key_p, const char *value_p, k_dbm_storage_t storage);

/**
 * @brief Get a value by key from the database
 *
 * This function retrieves a value associated with a given key from the database.
 *
 * @param key_p Pointer to the key for which the value is to be retrieved
 * @param value_buffer_p Pointer to a variable where the retrieved value will be stored
 * @param value_buffer_size Size of the buffer to store the value
 *
 * @return Returns 0 on success, -1 otherwise
 */
int k_dbm_get(const char *key_p, char *value_buffer_p, size_t value_buffer_size);

/**
 * @brief Delete a key-value pair from the database
 *
 * This function deletes a key-value pair from the database based on the provided key.
 *
 * @param key_p Pointer to the key of the entry to be deleted
 *
 * @return Returns 0 on success, -1 otherwise
 */
int k_dbm_delete(const char *key_p);

#ifdef __cplusplus
}
#endif
/* @} */