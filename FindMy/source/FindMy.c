/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
  Filename:       simpleBLEPeripheral.c
  Revised:        
  Revision:       

  Description:    This file contains the Simple BLE Peripheral sample application
                  

**************************************************************************************************/
/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "rf_phy_driver.h"
#include "global_config.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "sbpProfile_ota.h"
#include "ota_app_service.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "pwrmgr.h"
#include "gpio.h"
#include "simpleBLEPeripheral.h"
#include "ll.h"
#include "ll_hw_drv.h"
#include "ll_def.h"
#include "hci_tl.h"
#include "flash.h"
/*********************************************************************
 * MACROS
 */
//#define LOG(...)  
/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD					5000

#define DEVINFO_SYSTEM_ID_LEN					8
#define DEVINFO_SYSTEM_ID						0
 
#define DEFAULT_DISCOVERABLE_MODE					GAP_ADTYPE_FLAGS_GENERAL
// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL		8//32//80
// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL		16//48//800
// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY			0
// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT			300//1000
// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST			TRUE
// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL			6
#define INVALID_CONNHANDLE						0xFFFF
// Default passcode
#define DEFAULT_PASSCODE						0//19655
// Length of bd addr as a string
#define B_ADDR_STR_LEN							15
#define RESOLVING_LIST_ENTRY_NUM				10
// Offset of advertData&scanRspData
#define	RSP_OFFSET_MAC							4

/*********************************************************************
 * build define
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
perStatsByChan_t g_perStatsByChanTest;

/*********************************************************************
 * EXTERNAL VARIABLES
 */
volatile uint8_t g_current_advType = LL_ADV_CONNECTABLE_UNDIRECTED_EVT;


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
uint8	simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static	gaprole_States_t	gapProfileState	=	GAPROLE_INIT;


uint8	dev_mac_data[MAC_DATA_LEN]	=	{0x00,0x00,0x00,0x00,0x00,0x00};
static uint8 public_key[] = {
	0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xef,
	0xfe,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11
};

void set_addr_from_key(uint8 *dev_mac_data, uint8 *public_key)
{
	dev_mac_data[0] = public_key[0] | 0xc0;
	dev_mac_data[1] = public_key[1];
	dev_mac_data[2] = public_key[2];
	dev_mac_data[3] = public_key[3];
	dev_mac_data[4] = public_key[4];
	dev_mac_data[5] = public_key[5];
};

void set_payload_from_key(uint8_t *payload, uint8_t *public_key) {
	/* copy last 22 bytes */
	osal_memcpy(&payload[7], &public_key[6], 22);
	/* append two bits of public key */
	payload[29] = public_key[0] >> 6;
}

/** Advertisement payload */
static uint8 advertData[] = 
{
	0x1e, /* Length (30) */
	0xff, /* Manufacturer Specific Data (type 0xff) */
	0x4c, 0x00, /* Company ID (Apple) */
	0x12, 0x19, /* Offline Finding type and length */
	0x00, /* State */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, /* First two bits */
	0x00, /* Hint (0x00) */
};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "FindMy";
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void simpleProfileChangeCB( uint8 paramID );
static void peripheralStateReadRssiCB( int8 rssi  );

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
    peripheralStateNotificationCB,  // Profile State Change Callbacks
    peripheralStateReadRssiCB       // When a valid RSSI is read from controller (not used by application)
};
#if (DEF_GAPBOND_MGR_ENABLE==1)
// GAP Bond Manager Callbacks, add 2017-11-15
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
	NULL,                     // Passcode callback (not used by application)
	NULL                      // Pairing / Bonding state Callback (not used by application)
};
#endif
// Simple GATT Profile Callbacks
static simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
{
    simpleProfileChangeCB    // Charactersitic value change callback
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
    simpleBLEPeripheral_TaskID = task_id;
//	uint8	i	=	0;

    // Setup the GAP
    VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
    // Setup the GAP Peripheral Role Profile
    {
        // device starts advertising upon initialization
        uint8 initial_advertising_enable	=	FALSE;

        uint8 enable_update_request			=	DEFAULT_ENABLE_UPDATE_REQUEST;
        uint8 advChnMap		=		GAP_ADVCHAN_37;// | GAP_ADVCHAN_38 | GAP_ADVCHAN_39; 
        
        // By setting this to zero, the device will go into the waiting state after
        // being discoverable for 30.72 second, and will not being advertising again
        // until the enabler is set back to TRUE
        uint16 gapRole_AdvertOffTime	=	0;
    
        uint16 desired_min_interval		=	DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        uint16 desired_max_interval		=	DEFAULT_DESIRED_MAX_CONN_INTERVAL;
        uint16 desired_slave_latency	=	DEFAULT_DESIRED_SLAVE_LATENCY;
        uint16 desired_conn_timeout		=	DEFAULT_DESIRED_CONN_TIMEOUT;
		
        uint8 peerPublicAddr[] = {
			0x01,
			0x02,
			0x03,
			0x04,
			0x05,
			0x06
		};

		{
			set_addr_from_key(dev_mac_data, public_key);
			set_payload_from_key(advertData, public_key);
		}

//        uint8 advType =g_current_advType;// LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT;//LL_ADV_SCANNABLE_UNDIRECTED_EVT;//LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT;//;    // it seems a  bug to set GAP_ADTYPE_ADV_NONCONN_IND = 0x03
				uint8 advType =LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT;// LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT;//LL_ADV_SCANNABLE_UNDIRECTED_EVT;//LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT;//;    // it seems a  bug to set GAP_ADTYPE_ADV_NONCONN_IND = 0x03
		
        GAPRole_SetParameter( GAPROLE_ADV_EVENT_TYPE,	sizeof( uint8 ),		&advType );
				GAPRole_SetParameter( GAPROLE_ADV_DIRECT_ADDR,	sizeof(peerPublicAddr), peerPublicAddr);
        // set adv channel map
        GAPRole_SetParameter( GAPROLE_ADV_CHANNEL_MAP,	sizeof( uint8 ),		&advChnMap);        
        // Set the GAP Role Parameters
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED,	sizeof( uint8 ),		&initial_advertising_enable );
        GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME,	sizeof( uint16 ),		&gapRole_AdvertOffTime 	);
        GAPRole_SetParameter( GAPROLE_ADVERT_DATA,		sizeof( advertData ),	advertData	);
        GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE,	sizeof( uint8  ),	&enable_update_request	);
        GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL,	sizeof( uint16 ),	&desired_min_interval	);
        GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL,	sizeof( uint16 ),	&desired_max_interval	);
        GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY,		sizeof( uint16 ),	&desired_slave_latency	);
        GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER,	sizeof( uint16 ),	&desired_conn_timeout	);
    }

	// Set the GAP Characteristics
	GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

    // Set advertising interval
    {
        uint16 advInt = //8000; //1600;//2400;//1600;//1600;//800;//1600;   // actual time = advInt * 625us

        GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
        GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
        GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
        GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
    }
#if (DEF_GAPBOND_MGR_ENABLE==1)    
    // Setup the GAP Bond Manager, add 2017-11-15
    {
		uint32 passkey = DEFAULT_PASSCODE;
		uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
		uint8 mitm = TRUE;
		uint8 ioCap = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;
		uint8 bonding = TRUE;
		GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
		GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
		GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
		GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
		GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
    }
#endif    
    // Initialize GATT attributes
	GGS_AddService( GATT_ALL_SERVICES );            	// 	GAP
	GATTServApp_AddService( GATT_ALL_SERVICES );    	// 	GATT attributes
//	DevInfo_AddService();                           	// 	Device Information Service
	SimpleProfile_AddService( GATT_ALL_SERVICES );  	// 	Simple GATT Profile
	uint8	OTA_Passward_AscII[8]	=	{'R','G','B','L','i','g','h','t'};
	//ota_app_AddService_UseKey(8,OTA_Passward_AscII);
	ota_app_AddService();

    #if (0)
    {
		uint8_t mtuSet = 247;
		llInitFeatureSet2MPHY(TRUE);
		llInitFeatureSetDLE(TRUE);
		ATT_SetMTUSizeMax(mtuSet);
//		LOG("[2Mbps | DLE | MTU %d] \n",mtuSet);
    }
    #else
	    ATT_SetMTUSizeMax(23);
	    llInitFeatureSet2MPHY(FALSE);
	    llInitFeatureSetDLE(FALSE);
    #endif
    // Setup a delayed profile startup
    osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );
    // for receive HCI complete message
    GAP_RegisterForHCIMsgs(simpleBLEPeripheral_TaskID);
		LL_PLUS_PerStats_Init(&g_perStatsByChanTest);
		LOG("=====SimpleBLEPeripheral_Init Done=======\n");
	
	
//		pwroff_cfg_t cfg[1] = {
//			P11, POL_FALLING, 1
//		};
//		hal_pwrmgr_enter_standby(cfg,1);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        uint8 *pMsg;

        if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL )
        {
            simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & SBP_START_DEVICE_EVT )
    {
        // Start the Device
        VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );
    #if (DEF_GAPBOND_MGR_ENABLE==1)
        // Start Bond Manager, 2017-11-15
        VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );
    #endif
        HCI_LE_ReadResolvingListSizeCmd();

        return ( events ^ SBP_START_DEVICE_EVT );
    }
    // enable adv
    if ( events & SBP_RESET_ADV_EVT )
    {
        uint8 initial_advertising_enable = TRUE;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
        return ( events ^ SBP_RESET_ADV_EVT );
    } 

	if(events & SBP_DEALDATA){
		LOG("\ndeal app datas in events!!!\n");
		return(events ^ SBP_DEALDATA);
	}
    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
	hciEvt_CmdComplete_t *pHciMsg;

	switch ( pMsg->event ){  
		case HCI_GAP_EVENT_EVENT:{
			switch( pMsg->status ){
				case HCI_COMMAND_COMPLETE_EVENT_CODE:
					pHciMsg = (hciEvt_CmdComplete_t *)pMsg;
					LOG("==> HCI_COMMAND_COMPLETE_EVENT_CODE: %x\n", pHciMsg->cmdOpcode);
					//safeToDealloc = gapProcessHCICmdCompleteEvt( (hciEvt_CmdComplete_t *)pMsg );
				break;
				
				default:
					//safeToDealloc = FALSE;  // Send to app
				break;
			}
		}
	}
}
/*********************************************************************
 * @fn      peripheralStateReadRssiCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateReadRssiCB( int8  rssi )
{
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
 static void peripheralStateNotificationCB( gaprole_States_t newState )
{
    switch ( newState )
    {
        case GAPROLE_STARTED:
        {
            #if(0) 
            uint8 systemId[DEVINFO_SYSTEM_ID_LEN];
            // use 6 bytes of device address for 8 bytes of system ID value
            systemId[0] = ownAddress[0];
            systemId[1] = ownAddress[1];
            systemId[2] = ownAddress[2];
        
            // set middle bytes to zero
            systemId[4] = 0x00;
            systemId[3] = 0x00;
        
            // shift three bytes up
            systemId[7] = ownAddress[5];
            systemId[6] = ownAddress[4];
            systemId[5] = ownAddress[3];
        
            DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
            #endif    
			extern uint8 ownPublicAddr[LL_DEVICE_ADDR_LEN];
			// update MAC.Attention to the sequence of MAC address!!!
			for(uint8 i = 0;i < LL_DEVICE_ADDR_LEN;i++){
				ownPublicAddr[i]	=	dev_mac_data[LL_DEVICE_ADDR_LEN - 1 - i];
			}

            LOG("Gaprole_start:\n");
			osal_set_event(simpleBLEPeripheral_TaskID, SBP_RESET_ADV_EVT);                                     
        }
        break;
        
        case GAPROLE_ADVERTISING:
		{
			LOG("Gaprole_adversting:\n");
		}   
        break;
        
        case GAPROLE_CONNECTED:
            HCI_PPLUS_ConnEventDoneNoticeCmd(simpleBLEPeripheral_TaskID, NULL); 
            LOG("Gaprole_Connected:\n");
        break;
        
        case GAPROLE_CONNECTED_ADV:
	    break;      
        case GAPROLE_WAITING:
        	LOG("Gaprole_Disconnection:\n");
        break;
        
        case GAPROLE_WAITING_AFTER_TIMEOUT:
        	LOG("Gaprole_waitting_after_timerout:\n");
        break;
        
        case GAPROLE_ERROR:
        break;
        
        default:
        break;        
    }  
    gapProfileState = newState;
	LOG("[GAP ROLE %d]\n",newState);
     
    VOID gapProfileState;     
}


/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB( uint8 paramID )
{
    
	switch( paramID )
	{
		case SIMPLEPROFILE_CHAR1:
 
		break;

		default:
			// not process other attribute change
		break;
	}
}


/*********************************************************************
*********************************************************************/
