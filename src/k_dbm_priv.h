/**
 * @brief Database Manager protocol private header file
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

#include "k_dbm.h"

/* Macro ---------------------------------------------------------------------*/
#ifndef K_DBM_DB_SIZE
#error "DB size must be defined at compile time"
#endif
#ifndef K_DBM_VALUE_MAX_LENGTH
#error "Max value length must be defined at compile time"
#endif

/* Typedef -------------------------------------------------------------------*/

/**
 * @brief DB entry structure
 */
typedef struct
{
	const char	   *key;							//!< DB entry key
	char			value[K_DBM_VALUE_MAX_LENGTH];	//!< DB entry value
	k_dbm_storage_t storage;						//!< DB entry actual storage
} k_dbm_entry_t;

/**
 *@brief DB structure
 */
typedef struct
{
	k_dbm_entry_t entries_a[K_DBM_DB_SIZE];	 //!< DB entries
	size_t		  db_size;					 //!< Max entries size
	size_t		  db_count;					 //!< Number of entries currently in DB
} k_dbm_db_t;

/**
 * @brief DB manager context
 */
typedef struct
{
	k_dbm_config_t config;	//!< DBM configuration
	k_dbm_db_t	   db;		//!< DB
} k_dbm_context_t;

/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/**
 * @brief Return the first free entry in DB
 *
 * @return -1 if no entry is free, first free entry otherwise
 */
int k_dbm_find_first_empty_entry(void);

/**
 * @brief Find an entry by key in DB
 *
 * @param key_p Key to search for
 *
 * @return Index of the entry if found, -1 otherwise
 */
int k_dbm_find_entry(const char *key_p);

#ifdef __cplusplus
}
#endif
/* @} */