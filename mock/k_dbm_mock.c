/**
 * @file k_devjson_protocol_mock.c
 * @ingroup k_devjson_protocol
 * @{
 */

/* Include -------------------------------------------------------------------*/
#include "k_dbm_mock.h"

/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Definition -------------------------------------------------------*/
DEFINE_FAKE_VALUE_FUNC(int, k_dbm_init, const k_dbm_config_t *)
DEFINE_FAKE_VALUE_FUNC(int, k_dbm_insert, const char *, const char *, k_dbm_storage_t)
DEFINE_FAKE_VALUE_FUNC(int, k_dbm_get, const char *, char *, size_t)
DEFINE_FAKE_VALUE_FUNC(int, k_dbm_delete, const char *)
