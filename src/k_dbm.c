/**
 * @file k_dbm.c
 * @ingroup k_dbm
 * @{
 */

/* Include -------------------------------------------------------------------*/
#include "k_dbm.h"

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
	if (key_p && value_p && K_DBM_STORAGE_NONE != storage && strlen(key_p) < K_DBM_KEY_MAX_LENGTH)
	{
		k_dbm_context.config.k_dbm_lock_mutex_f(K_DBM_LOCK_MUTEX_INFINITE_TIMEOUT);
		const int first_free_entry = k_dbm_find_first_empty_entry();
		if (-1 != first_free_entry)
		{
			int save_success = 0;
			switch (storage)
			{
				case K_DBM_STORAGE_NVM:
					save_success = k_dbm_context.config.k_dbm_insert_f(key_p, value_p);
					/* Fallthrough */
				case K_DBM_STORAGE_RAM:
					if (0 == save_success)
					{
						k_dbm_context.db.entries_a[first_free_entry].key = key_p;
						strcpy(k_dbm_context.db.entries_a[first_free_entry].value, value_p);
						k_dbm_context.db.entries_a[first_free_entry].storage = storage;
						ret_code											 = 0;
					}
					break;
				default:
					ret_code = -1;
					break;
			}
		}
		k_dbm_context.config.k_dbm_unlock_mutex_f();
	}
	return ret_code;
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