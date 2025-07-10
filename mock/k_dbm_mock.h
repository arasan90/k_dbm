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

#ifdef __cplusplus
}
#endif
/* @} */