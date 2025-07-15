/**
 * @file k_dbm.c
 * @ingroup k_dbm
 * @{
 */

/* Include -------------------------------------------------------------------*/
#include "k_dbm.h"

#include <stdio.h>
#include <string.h>

#include "k_dbm_priv.h"

/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
k_dbm_context_t k_dbm_context = {0};

/* Function Definition -------------------------------------------------------*/
int k_dbm_init(const k_dbm_config_t *config_p)
{
	int ret_code = -1;
	if (config_p)
	{
		if (config_p->k_dbm_lock_mutex_f && config_p->k_dbm_unlock_mutex_f && config_p->k_dbm_insert_f && config_p->k_dbm_get_f && config_p->k_dbm_delete_f)
		{
			k_dbm_context.config	 = *config_p;
			k_dbm_context.db.db_size = K_DBM_DB_SIZE;
			ret_code				 = 0;
		}
	}
	return ret_code;
}

int k_dbm_insert(const char *key_p, const char *value_p, k_dbm_storage_t storage)
{
	int ret_code = -1;
	if (key_p && value_p && strlen(value_p) < K_DBM_VALUE_MAX_LENGTH && K_DBM_STORAGE_NONE != storage)
	{
		k_dbm_context.config.k_dbm_lock_mutex_f(K_DBM_LOCK_MUTEX_INFINITE_TIMEOUT);
		const int first_free_entry = k_dbm_find_first_empty_entry();
		int		  db_index		   = k_dbm_find_entry(key_p);
		if (-1 != db_index || -1 != first_free_entry)
		{
			/* Key is already present in DB, or we have space to insert a new key in it */
			int save_success = 0;
			switch (storage)
			{
				case K_DBM_STORAGE_NVM:
					save_success = k_dbm_context.config.k_dbm_insert_f(key_p, value_p);
					/* Fallthrough */
				case K_DBM_STORAGE_RAM:
					if (0 == save_success)
					{
						if (-1 != db_index)
						{
							/* Update existing entry */
							k_dbm_context.db.entries_a[db_index].key = key_p;
							strcpy(k_dbm_context.db.entries_a[db_index].value, value_p);
							ret_code = 0;
						}
						else
						{
							/* Insert new entry */
							k_dbm_context.db.entries_a[first_free_entry].key = key_p;
							strcpy(k_dbm_context.db.entries_a[first_free_entry].value, value_p);
							k_dbm_context.db.entries_a[first_free_entry].storage = storage;
							ret_code											 = 0;
						}
					}
					break;
				default:
					break;
			}
		}
		k_dbm_context.config.k_dbm_unlock_mutex_f();
	}
	return ret_code;
}

int k_dbm_get(const char *key_p, char *value_buffer_p, size_t value_buffer_size)
{
	int ret_code	 = -1;
	int is_key_found = 0;
	if (key_p && value_buffer_p)
	{
		k_dbm_context.config.k_dbm_lock_mutex_f(K_DBM_LOCK_MUTEX_INFINITE_TIMEOUT);
		for (size_t i = 0; i < k_dbm_context.db.db_size; i++)
		{
			if (k_dbm_context.db.entries_a[i].key && 0 == strcmp(k_dbm_context.db.entries_a[i].key, key_p))
			{
				is_key_found = 1;
				if (value_buffer_size > strlen(k_dbm_context.db.entries_a[i].value))
				{
					strcpy(value_buffer_p, k_dbm_context.db.entries_a[i].value);
					ret_code = 0;
				}
				break;
			}
		}
		if (!is_key_found)
		{
			if (0 == k_dbm_context.config.k_dbm_get_f(key_p, value_buffer_p, value_buffer_size))
			{
				ret_code			 = 0;  // Key found in NVM
				int first_free_entry = k_dbm_find_first_empty_entry();
				if (-1 != first_free_entry)
				{
					/* Cache the value */
					k_dbm_context.db.entries_a[first_free_entry].key = key_p;
					strcpy(k_dbm_context.db.entries_a[first_free_entry].value, value_buffer_p);
					k_dbm_context.db.entries_a[first_free_entry].storage = K_DBM_STORAGE_NVM;
				}
			}
		}
		k_dbm_context.config.k_dbm_unlock_mutex_f();
	}
	return ret_code;
}

int k_dbm_delete(const char *key_p)
{
	int ret_code = -1;
	if (key_p)
	{
		k_dbm_context.config.k_dbm_lock_mutex_f(K_DBM_LOCK_MUTEX_INFINITE_TIMEOUT);
		for (size_t i = 0; i < k_dbm_context.db.db_size; i++)
		{
			if (k_dbm_context.db.entries_a[i].key && 0 == strcmp(k_dbm_context.db.entries_a[i].key, key_p))
			{
				int is_deleted = 1;
				switch (k_dbm_context.db.entries_a[i].storage)
				{
					case K_DBM_STORAGE_NVM:
						if (0 != k_dbm_context.config.k_dbm_delete_f(key_p))
						{
							is_deleted = 0;	 // Deletion from NVM failed
						}
					/* Fallthrough */
					case K_DBM_STORAGE_RAM:
						if (is_deleted)
						{
							k_dbm_context.db.entries_a[i].storage = K_DBM_STORAGE_NONE;
							k_dbm_context.db.entries_a[i].key	  = NULL;
							memset(k_dbm_context.db.entries_a[i].value, 0, sizeof(k_dbm_context.db.entries_a[i].value));
							ret_code = 0;
						}
						break;
					default:
						break;
				}
				break;
			}
		}
		k_dbm_context.config.k_dbm_unlock_mutex_f();
	}
	return ret_code;
}

size_t k_dbm_get_free_space(void)
{
	size_t free_count = 0;
	for (size_t i = 0; i < k_dbm_context.db.db_size; i++)
	{
		if (K_DBM_STORAGE_NONE == k_dbm_context.db.entries_a[i].storage)
		{
			free_count++;
		}
	}
	return free_count;
}

int k_dbm_find_first_empty_entry(void)
{
	int ret_code = -1;
	for (size_t i = 0; i < k_dbm_context.db.db_size; i++)
	{
		if (K_DBM_STORAGE_NONE == k_dbm_context.db.entries_a[i].storage)
		{
			ret_code = (int)i;
			break;
		}
	}
	return ret_code;
}

int k_dbm_find_entry(const char *key_p)
{
	int index = -1;
	for (size_t i = 0; i < k_dbm_context.db.db_size; i++)
	{
		if (k_dbm_context.db.entries_a[i].key && 0 == strcmp(k_dbm_context.db.entries_a[i].key, key_p))
		{
			index = (int)i;
			break;
		}
	}
	return index;
}