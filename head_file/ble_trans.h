/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_srv_bracelet Shoe Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Shoe Service module.
 *
 * @details This module implements the Shoe Service with the Shoe Level characteristic.
 *          During initialization it adds the Shoe Service and Shoe Level characteristic
 *          to the BLE stack databracelete. Optionally it can also add a Report Reference descriptor
 *          to the Shoe Level characteristic (used when including the Shoe Service in
 *          the HID service).
 *
 *          If specified, the module will support notification of the Shoe Level characteristic
 *          through the ble_bracelet_bracelet_data_update() function.
 *          If an event handler is supplied by the application, the Shoe Service will
 *          generate Shoe Service events to the application.
 *
 * @note The application must propagate BLE stack events to the Shoe Service module by calling
 *       ble_bracelet_on_ble_evt() from the from the @ref ble_stack_handler callback.
 */

#ifndef BLE_TRANS_H__
#define BLE_TRANS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief Shoe Service event type. */
typedef enum
{
	BLE_EVT_NONE = 0
} ble_evt_type_t;

/**@brief Shoe Service event. */
typedef struct
{
    ble_evt_type_t evt_type;                                  /**< Type of event. */
} ble_s_evt_t;

// Forward declaration of the ble_bracelet_t type. 
typedef struct ble_s ble_t;

/**@brief Shoe Service event handler type. */
typedef void (*ble_s_evt_handler_t) (ble_t * p_ble, ble_s_evt_t * p_evt, uint8_t* p_buff, uint8_t size);

/**@brief Shoe Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_s_evt_handler_t             evt_handler;                    /**< Event handler to be called for handling events in the Shoe Service. */
    bool                          support_notification;           /**< TRUE if notification of Shoe Level measurement is supported. */
    ble_srv_cccd_security_mode_t  data_char_attr_md;     /**< Initial security data for bracelet characteristics attribute */
} ble_init_t;

/**@brief Shoe Service structure. This contains various status information for the service. */
typedef struct ble_s
{
    ble_s_evt_handler_t             evt_handler;                    /**< Event handler to be called for handling events in the Shoe Service. */
    uint16_t                      service_handle;                 /**< Handle of Shoe Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      receive_data_handles;  /**< Handles related to the Shoe Level characteristic. */
    ble_gatts_char_handles_t      send_data_handles;     /**< Handles related to the Shoe Level characteristic. */
		uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    bool                          is_notification_supported;      /**< TRUE if notification of Shoe Level is supported. */
} ble_t;

/**@brief Function for initializing the Shoe Service.
 *
 * @param[out]  p_bracelet  Shoe Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_bracelet_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_init(ble_t * p_ble, const ble_init_t * p_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Shoe Service.
 *
 * @note For the requirements in the BRACELET specification to be fulfilled,
 *       ble_bracelet_bracelet_data_update() must be called upon reconnection if the
 *       bracelet data has changed while the service has been disconnected from a bonded
 *       client.
 *
 * @param[in]   p_bracelet      Shoe Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_on_ble_evt(ble_t * p_ble, ble_evt_t * p_ble_evt);

/**@brief Function for updating the bracelet data.
 *
 * @details The application calls this function after having performed a bracelet measurement. If
 *          notification has been enabled, the bracelet data characteristic is sent to the client.
 *
 * @note For the requirements in the BRACELET specification to be fulfilled,
 *       this function must be called upon reconnection if the bracelet data has changed
 *       while the service has been disconnected from a bonded client.
 *
 * @param[in]   p_bracelet     Shoe Service structure.
 * @param[in]   bracelet_data  New bracelet measurement value (in percent of full capacity).
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_data_update(ble_t * p_ble, uint8_t* data, uint16_t size);


uint32_t ble_hrs_heart_rate_measurement_send(ble_t * p_ble,uint8_t * data, uint16_t data_size);

#endif // BLE_TRANS_H__

/** @} */
