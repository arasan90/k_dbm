/**
 * @brief Mock for DevJSON protocol header file
 * @addtogroup k_devjson_protocol
 * @{
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* Include -------------------------------------------------------------------*/
#include "fff.h"
#include "k_dbm.h"

/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
DECLARE_FAKE_VALUE_FUNC(int, k_dbm_init, const k_dbm_config_t *)
DECLARE_FAKE_VALUE_FUNC(int, k_dbm_insert, const char *, const char *, k_dbm_storage_t)
DECLARE_FAKE_VALUE_FUNC(int, k_dbm_get, const char *, char *, size_t)
DECLARE_FAKE_VALUE_FUNC(int, k_dbm_delete, const char *)

#ifdef __cplusplus
}
#endif
/* @} */