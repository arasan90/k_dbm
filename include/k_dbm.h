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
	K_DBM_STORAGE_RAM,	//!< Use RAM for storage
	K_DBM_STORAGE_NVM	//!< Use Non-Volatile Memory for storage
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
 * @param storage The storage type to use for the insertion
 *
 * @return Returns 0 on success, -1 on failure
 */
typedef int (*k_dbm_insert_t)(const char *key, const char *value, k_dbm_storage_t storage);

/**
 * @brief Function pointer type for retrieving a value by key from the database
 *
 * @param key The key to retrieve
 * @param value Pointer to store the retrieved value
 *
 * @return Returns 0 on success, -1 on failure
 */
typedef int (*k_dbm_get_t)(const char *key, char **value);

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
int k_dbm_init(const k_dbm_config_t *config);

#ifdef __cplusplus
}
#endif
/* @} */