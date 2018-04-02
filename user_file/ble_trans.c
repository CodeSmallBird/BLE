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

#include "ble_trans.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ble       Shoe Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_t * p_ble, ble_evt_t * p_ble_evt)
{
    p_ble->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_ble       Shoe Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_t * p_ble, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ble->conn_handle = BLE_CONN_HANDLE_INVALID;
}



/**@brief Function for handling the Write event.
 *
 * @param[in]   p_ble       Shoe Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_t * p_ble, ble_evt_t * p_ble_evt)
{
    if (p_ble->is_notification_supported)
    {
        ble_gatts_evt_write_t* p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
        uint8_t*               p_buff      = p_evt_write->data;
				uint8_t                p_len       = p_evt_write->len;
				
        if ((p_evt_write->handle == p_ble->receive_data_handles.cccd_handle) && (p_evt_write->len == 2))
        {
            // CCCD written, call application event handler
						if(p_buff[0] == 0x001 || p_buff[1] == 0x002)
               p_ble->is_notification_supported = true;
						else
               p_ble->is_notification_supported = false;
				}
				if((p_evt_write->handle == p_ble->receive_data_handles.value_handle) && (p_ble->evt_handler != NULL))
        {
            // CCCD written, call application event handler
            if (p_ble->evt_handler != NULL)
            {
								ble_s_evt_t evt;
								p_ble->evt_handler(p_ble, &evt, p_buff, p_len);
            }
        }
    }
}


void ble_on_ble_evt(ble_t * p_ble, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ble, p_ble_evt);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ble, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_ble, p_ble_evt);
            break;
            
        default:
            break;
    }
}


/**@brief Function for adding the Shoe Level characteristic.
 *
 * @param[in]   p_ble        Shoe Service structure.
 * @param[in]   p_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t data_receive_char_add(ble_t * p_ble, const ble_init_t * p_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    
    memset(&char_md, 0, sizeof(char_md));
    
		char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
        
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_DERONG_MOTO_RECEIVE_CHAR);
	
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_init->data_char_attr_md.read_perm;
    attr_md.write_perm = p_init->data_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
        
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 20;
    
    return sd_ble_gatts_characteristic_add(p_ble->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_ble->receive_data_handles);
}

/**@brief Function for adding the Shoe Level characteristic.
 *
 * @param[in]   p_ble        Shoe Service structure.
 * @param[in]   p_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t data_send_char_add(ble_t * p_ble, const ble_init_t * p_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    
    memset(&char_md, 0, sizeof(char_md));
    
		char_md.char_props.read   = 1;
		char_md.char_props.notify = (p_ble->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
       
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_DERONG_MOTO_SEND_CHAR);
	
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_init->data_char_attr_md.read_perm;
    attr_md.write_perm = p_init->data_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
        
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 20;
    
    return sd_ble_gatts_characteristic_add(p_ble->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_ble->send_data_handles);
}


uint32_t ble_init(ble_t * p_ble, const ble_init_t * p_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_ble->evt_handler               = p_init->evt_handler;
    p_ble->conn_handle               = BLE_CONN_HANDLE_INVALID;
    p_ble->is_notification_supported = p_init->support_notification;
    
    // Add service
		BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_DERONG_MOTO_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_ble->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add data characteristic
    err_code = data_receive_char_add(p_ble, p_init);
		if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }
		
		return data_send_char_add(p_ble, p_init);
}


uint32_t ble_data_update(ble_t * p_ble, uint8_t* data, uint16_t size)
{
    uint32_t err_code = NRF_SUCCESS;

		// Send value if connected and notifying
		if ((p_ble->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ble->is_notification_supported)
		{
				ble_gatts_hvx_params_t hvx_params;
				
				memset(&hvx_params, 0, sizeof(hvx_params));
				
				hvx_params.handle   = p_ble->send_data_handles.value_handle;
				hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
				hvx_params.offset   = 0;
				hvx_params.p_len    = &size;
				hvx_params.p_data   = data;
				
				err_code = sd_ble_gatts_hvx(p_ble->conn_handle, &hvx_params);
		}
		else
		{
				err_code = NRF_ERROR_INVALID_STATE;
		}
    return err_code;
}



uint32_t ble_hrs_heart_rate_measurement_send(ble_t * p_ble,uint8_t * data, uint16_t data_size)
{
    uint32_t err_code;
    
    // Send value if connected and notifying
    if (p_ble->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint8_t                encoded_hrm[20];
//        uint16_t               len;
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params;
        
//        len     = hrm_encode(p_hrs, heart_rate, encoded_hrm);
        hvx_len = data_size;
        memcpy(encoded_hrm,data,data_size);
        memset(&hvx_params, 0, sizeof(hvx_params));
        
        hvx_params.handle   = p_ble->send_data_handles.value_handle;
        hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset   = 0;
        hvx_params.p_len    = &hvx_len;
        hvx_params.p_data   = encoded_hrm;
        
        err_code = sd_ble_gatts_hvx(p_ble->conn_handle, &hvx_params);
        if ((err_code == NRF_SUCCESS) && (hvx_len != data_size))
        {
           err_code = NRF_ERROR_DATA_SIZE;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


