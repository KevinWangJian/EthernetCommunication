/*
 * EthComm.c
 *
 *  Created on: 2019Äê10ÔÂ22ÈÕ
 *      Author: admin
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "EthComm.h"
#include "PhyTja110x.h"
#include "PhyDP83822I.h"

#include "Eth_Cfg.h"
#include "Eth_Fec.h"
#include "Eth.h"
#include "EthIf.h"
#include "EthIf_Prv.h"
#include "EthIf_Cbk.h"
#include "EthTrcv_Prv.h"
#include "EthTrcv.h"
#include "EthSM.h"

#include "ComM_Cfg.h"
#include "TcpIp.h"
#include "rba_EthTcp.h"
#include "rba_EthTcp_Prv.h"
#include "rba_EthTcp_Prv_Domain.h"

static int ethernet_timeCount = 0;
static int ethernet_timeCount2 = 0;

typedef  unsigned char  ETH_STATUS;

volatile unsigned char string[]= "DuPower";

zvt_pt_ipmode IpModeFlag     = User_IpMode;
zvt_pt_ipmode PrevIpModeFlag = User_IpMode;

TcpIp_SockAddrInetType Factory_LocalAddress  = {TCPIP_AF_INET, 0xc0a8010F, TCPIP_PORT_ANY};		/* 192.168.1.15 */
TcpIp_SockAddrInetType Factory_RouterAddress = {TCPIP_AF_INET, 0xc0a80101, TCPIP_PORT_ANY};		/* 192.168.1.1  */

TcpIp_SockAddrInetType User_LocalAddress	 = {TCPIP_AF_INET, 0xc0a80063, TCPIP_PORT_ANY};		/* 192.168.0.99 */
TcpIp_SockAddrInetType User_RouterAddress    = {TCPIP_AF_INET, 0xc0a80001, TCPIP_PORT_ANY};		/* 192.168.0.1 */

TcpIp_SockAddrInetType User_RemoteAddress    = {TCPIP_AF_INET, 0xc0a80027, 20007};				/* 192.168.0.39:20007 */
TcpIp_SockAddrInetType Factory_RemoteAddress = {TCPIP_AF_INET, 0xc0a80124, 20007};				/* 192.168.1.26:20007 */


TcpIp_SockAddrInetType CCV_POS_RemoteAddress  = {TCPIP_AF_INET, 0xc0a80047, 20007};			/* CCV_POS:   192.168.0.71:20007 ---> Switch P2 port.*/
TcpIp_SockAddrInetType LEFT_LEM_RemoteAddress = {TCPIP_AF_INET, 0xc0a80014, 50007};			/* Left_LEM:  192.168.0.20:50007 ---> Switch P2 port.*/
TcpIp_SockAddrInetType RIGHT_LEM_RemoteAddess = {TCPIP_AF_INET, 0xc0a80028, 50008};			/* Right_LEM: 192.168.0.40:50008 ---> Switch P2 port.*/
TcpIp_SockAddrInetType Tbox1_RemoteAddress	  = {TCPIP_AF_INET, 0xc0a80032, 1002 };			/* Tbox1:     192.168.0.50:1002  ---> Switch P0 port.*/
TcpIp_SockAddrInetType Maintain_RemoteAddress = {TCPIP_AF_INET, 0xc0a80046, 3002 };			/* Maintain:  192.168.0.70:3002  ---> Switch P3 port.*/
TcpIp_SockAddrInetType AdBoard_RemoteAddress  = {TCPIP_AF_INET, 0xc0a8003c, 2002 };			/* AdBoard:   192.168.0.60:2002  ---> Switch P2 port.*/

Eth_CommInfoDef EthernetCommInfo =
{
	{
		TCPIP_IP_ADDR_UNASSIGNED
	},
	{
		0
	},
	{
		{FALSE, 0, TCPIP_STATUS_INVALID, 0xFFFF, TCPIP_PORT_ANY, {0}, SWITCH_PORT_INVALID},
		{FALSE, 0, TCPIP_STATUS_INVALID, 0xFFFF, TCPIP_PORT_ANY, {0}, SWITCH_PORT_INVALID},
		{FALSE, 0, TCPIP_STATUS_INVALID, 0xFFFF, TCPIP_PORT_ANY, {0}, SWITCH_PORT_INVALID},
		{FALSE, 0, TCPIP_STATUS_INVALID, 0xFFFF, TCPIP_PORT_ANY, {0}, SWITCH_PORT_INVALID},
		{FALSE, 0, TCPIP_STATUS_INVALID, 0xFFFF, TCPIP_PORT_ANY, {0}, SWITCH_PORT_INVALID},
	}
};

/* Local ports which are used by EMS controller. */
TcpIp_PortType TcpClientLocalPort_array[TCPIP_TCPSOCKETMAX]    = {
																	TCPIP_PORT_ANY,
																	TCPIP_PORT_ANY,
																	TCPIP_PORT_ANY,
																	TCPIP_PORT_ANY,
																	TCPIP_PORT_ANY,
																 };

TcpIp_SockAddrInetType RemoteAddress_array[TCPIP_TCPSOCKETMAX] = {
																	{TCPIP_AF_INET, 0xc0a80047, 20007},
																	{TCPIP_AF_INET, 0xc0a80014, 50007},
																	{TCPIP_AF_INET, 0xc0a80028, 50008},
																	{TCPIP_AF_INET, 0xc0a80032, 1002 },
																	{TCPIP_AF_INET, 0xc0a80046, 3002 },
																 };



static SocketRxFrameBufferDef SocketRxFrameBuffer =
{
	{0}, 
    {0},
	{
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
		{FALSE, 0, {0}, 0, 0xFFFF, {0, TCPIP_IPADDR_ANY, TCPIP_PORT_ANY}},
	}
};


FramePbufferInfo CCV_RxFramePbuffer = 
{
{0},
{0},
{0}
};

FramePbufferInfo LEFT_LEM_RxFramePbuffer = 
{
{0},
{0},
{0}
};

FramePbufferInfo RIGHT_LEM_RxFramePbuffer = 
{
{0},
{0},
{0}
};

FramePbufferInfo Maintain_RxFramePbuffer = 
{
{0},
{0},
{0}
};

FramePbufferInfo Tbox1_RxFramePbuffer = 
{
{0},
{0},
{0}
};

static uint8_t  Rx_EthFrameStartWaitingFlag[TCPIP_TCPSOCKETMAX] = {0};
static uint8_t  Rx_EthFrameCurrentCounter[TCPIP_TCPSOCKETMAX]   = {0};
static uint8_t  MoreDataAvailableFlagInRxBufferIndex[TCPIP_TCPSOCKETMAX] = {0};

static uint16_t random_array[5000] = {0};

volatile uint32_t EthComm_1MS_Counter = 0;



static void  		   Ethernet_Init(void);
static Std_ReturnType  Ethernet_TcpClient_CreateSocket(TcpIp_PortType lPortNum, TcpIp_SockAddrInetType *pRemoteAddrPtr);
static Std_ReturnType  Ethernet_TcpClient_BindSocket(TcpIp_ProtocalTypeDef ptType, TcpIp_SocketIdType lSocketIdx);
static Std_ReturnType  Ethernet_TcpClient_ConnectServer(TcpIp_SocketIdType lSocketIdx, TcpIp_SockAddrInetType *pRemoteAddr);
static Std_ReturnType  Ethernet_TcpClient_PollingStatus(TcpIp_PortType uPortNum, TcpIp_ProtocalTypeDef ptType);
static Std_ReturnType  Ethernet_TcpClient_StartTransmit(TcpIp_SocketIdType lSocketIdx, TcpIp_ProtocalTypeDef ptoType, uint8_t *pTxdata, uint32_t size);
static Std_ReturnType  Ethernet_TcpClient_StopTransmit(TcpIp_SocketIdType lSocketIdx, TcpIp_ProtocalTypeDef ptoType);



/*
@brief
@details
@para
@return
*/
uint16_t Ethernet_GetRandom(uint32_t countVal)
{
	uint16_t result;
	int index;

	/* Using input parameter(seed) to initialize Random number generator. */
	srand(countVal);

	for (index = 0; index < (sizeof(random_array)/sizeof(uint16_t)); index++)
	{
		random_array[index] = (uint16_t)(rand()%10000 + 50000);
	}

	index = rand()%(sizeof(random_array)/sizeof(uint16_t));

	result = random_array[index];

	return (result);
}

/*
@brief
@details
@para
@return
*/
void EthIf_Initial(void)
{
	/* Initialize the Ethernet Interface. */
	EthIf_Init(NULL_PTR);

	/* Initialize Ethernet controller(FEC). */
	Ethernet_Init();

	/* Init EthTransceiver module. */
	EthTrcv_Init(NULL_PTR);

	/* initialize the EthSM. */
	EthSM_Init();

	/* Handles the communication mode to FULL COMMUNICATION. */
	EthSM_RequestComMode(ComMConf_ComMChannel_ComMChannel_Eth_Network_Channel, COMM_FULL_COMMUNICATION);
}

/*
@brief
@details
@para
@return
*/
void Ethernet_TrcvState_MainFunction(void)
{
	uint8 CtrlIndex = 0U;

	if ((PHY_DP83822HF_Prop[0].PhyLinkStatus != Valid_Link_Established) &&
		(PHY_DP83822HF_Prop[1].PhyLinkStatus != Valid_Link_Established) &&
		(TJA1101_AttributeInfo.LinkStatus    != TJA1101_Linkup_Success))
	{
		EthTrcv_Prv_State_e[CtrlIndex]       = ETHTRCV_STATE_INIT;
		EthTrcv_Prv_LinkState[CtrlIndex]     = ETHTRCV_LINK_STATE_DOWN;
		EthSM_TrcvLinkState_aen[CtrlIndex]   = ETHTRCV_LINK_STATE_DOWN;

		EthTrcv_Prv_InitState_e[CtrlIndex]   = ETHTRCV_INITSTATE_UNINIT;
		EthTrcv_Prv_CurrentMode[CtrlIndex]   = ETHTRCV_MODE_DOWN;
		EthTrcv_Prv_RequestedMode[CtrlIndex] = ETHTRCV_MODE_DOWN;

		EthIf_TrcvModeIndication(CtrlIndex, ETHTRCV_MODE_DOWN);
	}
	else
	{
		EthTrcv_Prv_State_e[CtrlIndex]       = ETHTRCV_STATE_ACTIVE;
		EthTrcv_Prv_LinkState[CtrlIndex]     = ETHTRCV_LINK_STATE_ACTIVE;
		EthSM_TrcvLinkState_aen[CtrlIndex]   = ETHTRCV_LINK_STATE_ACTIVE;

		EthTrcv_Prv_InitState_e[CtrlIndex]   = ETHTRCV_INITSTATE_INITFINISH;
		EthTrcv_Prv_CurrentMode[CtrlIndex]   = ETHTRCV_MODE_ACTIVE;
		EthTrcv_Prv_RequestedMode[CtrlIndex] = ETHTRCV_MODE_ACTIVE;

		EthIf_TrcvModeIndication(CtrlIndex, ETHTRCV_MODE_ACTIVE);
	}

	if ((PHY_DP83822HF_Prop[0].DuplexStatus != FullDuplexMode) &&
		(PHY_DP83822HF_Prop[1].DuplexStatus != FullDuplexMode))
	{
		EthTrcv_Prv_DuplexMode[CtrlIndex] = ETHTRCV_DUPLEX_MODE_HALF;
	}
	else
	{
		EthTrcv_Prv_DuplexMode[CtrlIndex] = ETHTRCV_DUPLEX_MODE_FULL;
	}

	if ((PHY_DP83822HF_Prop[0].SpeedStatus != Speed_100MbpsMode) &&
		(PHY_DP83822HF_Prop[1].SpeedStatus != Speed_100MbpsMode))
	{
		EthTrcv_Prv_BaudRate[CtrlIndex] = ETHTRCV_BAUD_RATE_10MBIT;
	}
	else
	{
		EthTrcv_Prv_BaudRate[CtrlIndex] = ETHTRCV_BAUD_RATE_100MBIT;
	}
}

/*
@brief
@details
@para
@return
*/
void Ethernet_IpAddrAssignment_MainFunction(void)
{
	uint8 						   CtrlIndex;
	uint8 						   Index;
	Std_ReturnType                 RetVal;

#if (ENABLE_TCP_CLIENT != 0U)
	Eth_CommInfoDef                *EthCommInfPtr;
#endif

	rba_EthTcp_DynSockTblType_tst  *pDynSockPtr;
	TcpIp_CurrAsgnedAddr_tst       *lCurrAsgnedAddr_pst;
	const TcpIp_AddrAssignment_tst *lAddressAsgned_pst;
	const TcpIp_ConfigType         *TcpIp_CurConfigPtr;
	TcpIp_SockAddrInetType         IpAddr;
	TcpIp_SockAddrInetType         RouterAddr;
	uint8 						   NetMask;

	CtrlIndex 			= 0;
	TcpIp_CurConfigPtr  = &TcpIp_ConfigSet[0];
	lCurrAsgnedAddr_pst = TcpIp_CurConfigPtr->EthLocalAddrConfig_paco[CtrlIndex].CurrAsgnedAddr_pst;
	lAddressAsgned_pst  = TcpIp_CurConfigPtr->EthLocalAddrConfig_paco[CtrlIndex].AddrAsgnment_pcst;

#if (ENABLE_TCP_CLIENT != 0U)
	EthCommInfPtr 		= (&EthernetCommInfo);
#endif

	IpAddr.domain 	    = TCPIP_AF_INET;
	RouterAddr.domain   = TCPIP_AF_INET;

	RetVal = TcpIp_GetIpAddr((TcpIpConf_TcpIpLocalAddr_TcpIpLocalAddr),
							 ((TcpIp_SockAddrType*)(&IpAddr.domain)),
							 (&NetMask),
							 ((TcpIp_SockAddrType*)(&RouterAddr.domain)));

	if (RetVal == E_OK)
	{
        /* TX-PHY0 Connection with Ethernet cable failed! --> Switch P3 port(The left RJ-45 on top view of the EMS control board). */
		if (PHY_DP83822HF_Prop[0].PhyLinkStatus == Link_Not_Established)	
		{
			if ((IpAddr.addr[0] != TCPIP_IPADDR_ZERO) && (RouterAddr.addr[0] != TCPIP_IPADDR_ZERO))
			{
				/* First, Close corresponding sockets. */
				for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
				{
					if (EthCommInfPtr->SocketInfo[Index].SwitchPort == SWITCH_PORT_P3)
					{
						pDynSockPtr = &rba_EthTcp_DynSockTbl_ast[Index];

						if ((pDynSockPtr->State_en > RBA_ETHTCP_CONN_CLOSED_E) && (pDynSockPtr->SockState_en > RBA_ETHTCP_SOCK_CLOSED_E))
						{
							TcpIp_Close(Index, TRUE);

							EthCommInfPtr->SocketInfo[Index].CurrentStat            = TCPIP_ERROR;
                            EthCommInfPtr->SocketInfo[Index].ConnectingCnt          = 0;
                            EthCommInfPtr->SocketInfo[Index].Established            = FALSE;
                            EthCommInfPtr->SocketInfo[Index].LocalPort              = TCPIP_PORT_ANY;
                            EthCommInfPtr->SocketInfo[Index].SocketId               = 0xFFFF;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.addr[0] = TCPIP_IPADDR_ANY;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.port    = TCPIP_PORT_ANY;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.domain  = TCPIP_AF_INET;
						}
					}
				}
			}
		}
        /* TX-PHY0 Connection with Ethernet cable successfully! --> Switch P3 port(The left RJ-45 on top view of the EMS control board). */
		else
		{
			for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
			{
				if (EthCommInfPtr->SocketInfo[Index].SwitchPort == SWITCH_PORT_P3)
				{
					if (EthCommInfPtr->SocketInfo[Index].CurrentStat == TCPIP_ERROR)
					{
						EthCommInfPtr->SocketInfo[Index].CurrentStat = TCPIP_STATUS_INVALID;
						EthCommInfPtr->SocketInfo[Index].SwitchPort  = SWITCH_PORT_INVALID;
					}
				}
			}
		}

        /* TX-PHY1 Connection with Ethernet cable failed! --> Switch P2 port(The right RJ-45 on top view of the EMS control board). */
		if (PHY_DP83822HF_Prop[1].PhyLinkStatus == Link_Not_Established)	
		{
			if ((IpAddr.addr[0] != TCPIP_IPADDR_ZERO) && (RouterAddr.addr[0] != TCPIP_IPADDR_ZERO))
			{
				/* First, Close corresponding sockets. */
				for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
				{
					if (EthCommInfPtr->SocketInfo[Index].SwitchPort == SWITCH_PORT_P2)
					{
						pDynSockPtr = &rba_EthTcp_DynSockTbl_ast[Index];

						if ((pDynSockPtr->State_en > RBA_ETHTCP_CONN_CLOSED_E) && (pDynSockPtr->SockState_en > RBA_ETHTCP_SOCK_CLOSED_E))
						{
							TcpIp_Close(Index, TRUE);

							EthCommInfPtr->SocketInfo[Index].CurrentStat            = TCPIP_ERROR;
                            EthCommInfPtr->SocketInfo[Index].ConnectingCnt          = 0;
                            EthCommInfPtr->SocketInfo[Index].Established            = FALSE;
                            EthCommInfPtr->SocketInfo[Index].LocalPort              = TCPIP_PORT_ANY;
                            EthCommInfPtr->SocketInfo[Index].SocketId               = 0xFFFF;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.addr[0] = TCPIP_IPADDR_ANY;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.port    = TCPIP_PORT_ANY;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.domain  = TCPIP_AF_INET;
						}
					}
				}
			}
		}
        /* TX-PHY1 Connection with Ethernet cable successfully! --> Switch P2 port(The right RJ-45 on top view of the EMS control board).*/
		else
		{
			for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
			{
				if (EthCommInfPtr->SocketInfo[Index].SwitchPort == SWITCH_PORT_P2)
				{
					if (EthCommInfPtr->SocketInfo[Index].CurrentStat == TCPIP_ERROR)
					{
						EthCommInfPtr->SocketInfo[Index].CurrentStat = TCPIP_STATUS_INVALID;
						EthCommInfPtr->SocketInfo[Index].SwitchPort  = SWITCH_PORT_INVALID;
					}
				}
			}
		}

        /* T1-PHY Connection with automotive Ethernet cable failed! --> Switch P0 port. */
        if (TJA1101_AttributeInfo.LinkStatus == TJA1101_Linkup_Failed) 
        {
            if ((IpAddr.addr[0] != TCPIP_IPADDR_ZERO) && (RouterAddr.addr[0] != TCPIP_IPADDR_ZERO))
            {
                for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++) 
                {
                    if (EthCommInfPtr->SocketInfo[Index].SwitchPort == SWITCH_PORT_P0) 
                    {
						pDynSockPtr = &rba_EthTcp_DynSockTbl_ast[Index];

						if ((pDynSockPtr->State_en > RBA_ETHTCP_CONN_CLOSED_E) && (pDynSockPtr->SockState_en > RBA_ETHTCP_SOCK_CLOSED_E))
						{
							TcpIp_Close(Index, TRUE);

							EthCommInfPtr->SocketInfo[Index].CurrentStat            = TCPIP_ERROR;
                            EthCommInfPtr->SocketInfo[Index].ConnectingCnt          = 0;
                            EthCommInfPtr->SocketInfo[Index].Established            = FALSE;
                            EthCommInfPtr->SocketInfo[Index].LocalPort              = TCPIP_PORT_ANY;
                            EthCommInfPtr->SocketInfo[Index].SocketId               = 0xFFFF;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.addr[0] = TCPIP_IPADDR_ANY;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.port    = TCPIP_PORT_ANY;
                            EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.domain  = TCPIP_AF_INET;
						}                        
                    }
                }
            }
        }
        /* T1-PHY Connection with automotive Ethernet cable successfully! --> Switch P0 port. */
        else 
        {
			for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
			{
				if (EthCommInfPtr->SocketInfo[Index].SwitchPort == SWITCH_PORT_P0)
				{
					if (EthCommInfPtr->SocketInfo[Index].CurrentStat == TCPIP_ERROR)
					{
						EthCommInfPtr->SocketInfo[Index].CurrentStat = TCPIP_STATUS_INVALID;
						EthCommInfPtr->SocketInfo[Index].SwitchPort  = SWITCH_PORT_INVALID;
					}
				}
			}            
        }/* End of PHYs link status. */


		if (EthTrcv_Prv_LinkState[CtrlIndex] == ETHTRCV_LINK_STATE_ACTIVE)
		{
			/* The current IP pattern is inconsistent with the previous IP pattern, indicating that the current IP pattern has changed. */
			if (IpModeFlag != PrevIpModeFlag)
			{
				if ((IpAddr.addr[0] != TCPIP_IPADDR_ZERO) && (RouterAddr.addr[0] != TCPIP_IPADDR_ZERO))
				{
					/* First, Close all sockets. */
					for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
					{
						pDynSockPtr = &rba_EthTcp_DynSockTbl_ast[Index];

						if ((pDynSockPtr->State_en > RBA_ETHTCP_CONN_CLOSED_E) && (pDynSockPtr->SockState_en > RBA_ETHTCP_SOCK_CLOSED_E))
						{
							TcpIp_Close(Index, TRUE);
						}
					}

					/* Second, release the current IP address. */
					TcpIp_ReleaseIpAddrAssignment(TcpIpConf_TcpIpLocalAddr_TcpIpLocalAddr);

					/* Third, release the TCP client resources associated with IP address assignment. */
					#if (ENABLE_TCP_CLIENT != 0)
					{
						EthCommInfPtr->IpAssignmentStat = TCPIP_IP_ADDR_UNASSIGNED;

						for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
						{
							EthCommInfPtr->SocketInfo[Index].CurrentStat 			= TCPIP_STATUS_INVALID;
							EthCommInfPtr->SocketInfo[Index].Established 			= FALSE;
							EthCommInfPtr->SocketInfo[Index].ConnectingCnt 			= 0;
							EthCommInfPtr->SocketInfo[Index].LocalPort   			= TCPIP_PORT_ANY;
							EthCommInfPtr->SocketInfo[Index].SocketId    			= 0xFFFF;
							EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.addr[0] = TCPIP_IPADDR_ANY;
							EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.port    = TCPIP_PORT_ANY;
							EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.domain  = TCPIP_AF_INET;
							EthCommInfPtr->SocketInfo[Index].SwitchPort             = SWITCH_PORT_INVALID;
						}
					}
					#endif
				}

				/* Fourth, save the current IP pattern to the previous IP pattern. */
				PrevIpModeFlag = IpModeFlag;
			}
			/* The current IP pattern is consistent with the previous IP pattern, indicating that the current IP pattern has not changed. */
			else
			{
				/* First, the current IP address of the system is empty, indicating that the IP address assignment has not been completed. */
				if ((IpAddr.addr[0] == TCPIP_IPADDR_ZERO) && (RouterAddr.addr[0] == TCPIP_IPADDR_ZERO))
				{
					NetMask = 8 * 3;

					switch (IpModeFlag)
					{
						case Factory_IpMode:
						{
							TcpIp_RequestIpAddrAssignment(TcpIpConf_TcpIpLocalAddr_TcpIpLocalAddr,
														  (lAddressAsgned_pst->AddrAsgnmentMeth_en),
														  ((TcpIp_SockAddrType*)(&Factory_LocalAddress.domain)),
														  NetMask,
														  ((TcpIp_SockAddrType*)(&Factory_RouterAddress.domain)));
						}break;

						case User_IpMode:
						{
							TcpIp_RequestIpAddrAssignment(TcpIpConf_TcpIpLocalAddr_TcpIpLocalAddr,
													 	  (lAddressAsgned_pst->AddrAsgnmentMeth_en),
														  ((TcpIp_SockAddrType*)(&User_LocalAddress.domain)),
														  NetMask,
														  ((TcpIp_SockAddrType*)(&User_RouterAddress.domain)));
						}break;

						default:break;
					}
				}

				if ((lCurrAsgnedAddr_pst->CurrAsgnedAddrMeth_en                  == lAddressAsgned_pst->AddrAsgnmentMeth_en) && \
					(lCurrAsgnedAddr_pst->IpAddrParams_un.IPv4Par_st.IpAddr_u32  != TCPIP_IPADDR_ZERO) && \
					(lCurrAsgnedAddr_pst->IpAddrParams_un.IPv4Par_st.DftRtr_u32  != TCPIP_IPADDR_ZERO) && \
					(lCurrAsgnedAddr_pst->IpAddrParams_un.IPv4Par_st.NetMask_u32 != TCPIP_IPADDR_ZERO))
				{
					#if (ENABLE_TCP_CLIENT != 0)
					{
						EthCommInfPtr->IpAssignmentStat = TCPIP_IP_ADDR_ASSIGNED;

						for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
						{
							if (EthCommInfPtr->SocketInfo[Index].CurrentStat == TCPIP_STATUS_INVALID)
							{
								EthCommInfPtr->SocketInfo[Index].CurrentStat 			= TCPIP_REQ_CREATE_SOCKET;
								EthCommInfPtr->SocketInfo[Index].Established 			= FALSE;
								EthCommInfPtr->SocketInfo[Index].ConnectingCnt 			= 0;
								EthCommInfPtr->SocketInfo[Index].LocalPort   			= TCPIP_PORT_ANY;
								EthCommInfPtr->SocketInfo[Index].SocketId    			= 0xFFFF;
								EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.addr[0] = TCPIP_IPADDR_ANY;
								EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.port    = TCPIP_PORT_ANY;
								EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.domain  = TCPIP_AF_INET;
								EthCommInfPtr->SocketInfo[Index].SwitchPort             = SWITCH_PORT_INVALID;
							}
						}
					}
					#endif
				}
			}
		}
		else
		{
			if ((IpAddr.addr[0] != TCPIP_IPADDR_ZERO) && (RouterAddr.addr[0] != TCPIP_IPADDR_ZERO))
			{
				/* First, Close all sockets. */
				for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
				{
					pDynSockPtr = &rba_EthTcp_DynSockTbl_ast[Index];

					if ((pDynSockPtr->State_en > RBA_ETHTCP_CONN_CLOSED_E) && (pDynSockPtr->SockState_en > RBA_ETHTCP_SOCK_CLOSED_E))
					{
						TcpIp_Close(Index, TRUE);
					}
				}

				/* Second, release the current IP address. */
				TcpIp_ReleaseIpAddrAssignment(TcpIpConf_TcpIpLocalAddr_TcpIpLocalAddr);

				#if (ENABLE_TCP_CLIENT != 0)
				{
					/* Third, release the TCP client resources associated with IP address assignment. */
					EthCommInfPtr->IpAssignmentStat  = TCPIP_IP_ADDR_UNASSIGNED;
					EthCommInfPtr->SocketPollTimeCnt = 0;

					for (Index = 0; Index < TCPIP_TCPSOCKETMAX; Index++)
					{
						EthCommInfPtr->SocketInfo[Index].CurrentStat 			= TCPIP_STATUS_INVALID;
						EthCommInfPtr->SocketInfo[Index].Established 			= FALSE;
						EthCommInfPtr->SocketInfo[Index].ConnectingCnt 			= 0;
						EthCommInfPtr->SocketInfo[Index].LocalPort   			= TCPIP_PORT_ANY;
						EthCommInfPtr->SocketInfo[Index].SocketId    			= 0xFFFF;
						EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.addr[0] = TCPIP_IPADDR_ANY;
						EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.port    = TCPIP_PORT_ANY;
						EthCommInfPtr->SocketInfo[Index].RemoteSockAddr.domain  = TCPIP_AF_INET;
						EthCommInfPtr->SocketInfo[Index].SwitchPort             = SWITCH_PORT_INVALID;
					}
				}
				#endif
			}
		}
	}
}

/*
@brief
@details
@para
@return
*/
void Ethernet_WaitRxFrameCompleteMainFunction(void)
{
	uint8_t socketIdx;
	uint8_t bufferIdx;
	SocketRxFrameBufferDef *pRxframeBuf;

	pRxframeBuf = (&SocketRxFrameBuffer);

	for (socketIdx = 0; socketIdx < TCPIP_TCPSOCKETMAX; socketIdx++)
	{
		if (Rx_EthFrameStartWaitingFlag[socketIdx] == 1)
		{
			Rx_EthFrameCurrentCounter[socketIdx]++;

			if (Rx_EthFrameCurrentCounter[socketIdx] >= 20)	/* Timeout: 1ms * 20 = 20ms */
			{
				bufferIdx = MoreDataAvailableFlagInRxBufferIndex[socketIdx];

				pRxframeBuf->RxSockFrame[bufferIdx].RxMoreDataStatus = FALSE;

				Rx_EthFrameCurrentCounter[socketIdx]   = 0;
				Rx_EthFrameStartWaitingFlag[socketIdx] = 0;

				MoreDataAvailableFlagInRxBufferIndex[socketIdx] = 0;
			}
		}
	}
}

/*
@brief
@details
@para
@return
*/
int Socket_WriteRxFrameBuffer(TcpIp_SocketIdType sockIdx, TcpIp_SockAddrType* pRemoteAddrPtr, uint8_t* pRxBuf, uint16_t rxLength)
{
	SocketRxFrameBufferDef *pRxframeBuf;
	uint8_t *pRxDataPtr;

	if ((sockIdx >= TCPIP_TCPSOCKETMAX) || (rxLength > RBA_ETHTCP_DFL_MSS) || (rxLength == 0))
	{
		return (0);
	}

	pRxframeBuf = (&SocketRxFrameBuffer);

	/* Is there any effective received TCP frame in the specified RX buffer which have not been read out by user. */
	if (pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].FrameValid == TRUE)
	{
		TcpIp_TcpReceived(sockIdx, rxLength);

		/* Yes, do not write TCP frame to the RX buffer, return to wait for the TCP frame being read out. */
		return (0);
	}
	else
	{
		if (rxLength < RBA_ETHTCP_DFL_MSS)
		{
			Rx_EthFrameStartWaitingFlag[sockIdx] = 0;
			Rx_EthFrameCurrentCounter[sockIdx] = 0;
			MoreDataAvailableFlagInRxBufferIndex[sockIdx] = 0;
		}
		else if (rxLength == RBA_ETHTCP_DFL_MSS)
		{
			if (Rx_EthFrameStartWaitingFlag[sockIdx] == 0)
			{
				Rx_EthFrameStartWaitingFlag[sockIdx] = 1;
			}
			else
			{
				if (Rx_EthFrameCurrentCounter[sockIdx] != 0)
				{
					Rx_EthFrameCurrentCounter[sockIdx]--;
				}
			}

			pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].RxMoreDataStatus = TRUE;

			MoreDataAvailableFlagInRxBufferIndex[sockIdx] = pRxframeBuf->RxWritePtr;
		}

		/* No, write TCP frame to the RX buffer. */
		pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].RxSockIdx = sockIdx;

		pRxDataPtr = (&(pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].RxDataBuf[0]));

		memcpy((uint8_t*)pRxDataPtr, (uint8*)pRxBuf, rxLength);

		pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].RxFrameSize = rxLength;

		/* Saving remote sender address information. */
		pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].RemoteNodeAddr.domain  = ((TcpIp_SockAddrInetType*)pRemoteAddrPtr)->domain;
		pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].RemoteNodeAddr.addr[0] = ((TcpIp_SockAddrInetType*)pRemoteAddrPtr)->addr[0];
		pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].RemoteNodeAddr.port    = ((TcpIp_SockAddrInetType*)pRemoteAddrPtr)->port;

		/* Modify received frame validation mark flag to TRUE. */
		pRxframeBuf->RxSockFrame[pRxframeBuf->RxWritePtr].FrameValid = TRUE;

		pRxframeBuf->RxWritePtr++;

		if (pRxframeBuf->RxWritePtr >= ETH_RX_FRAME_BUF_SIZE)
		{
			pRxframeBuf->RxWritePtr = 0;
		}

		TcpIp_TcpReceived(sockIdx, rxLength);

		return (1);
	}
}

/*
@brief
@details
@para
@return
*/
static int Socket_ReadRxFrameBuffer(TcpIp_SocketIdType* 	pSockIdx,
		                     	 	TcpIp_SockAddrInetType* pRemoteAddrPtr,
									uint16_t*               pRxLength,
									uint8_t*                pRxIndex)
{
	SocketRxFrameBufferDef *pRxframeBuf;

	pRxframeBuf = (&SocketRxFrameBuffer);

	/* Is there any effective received TCP frame in the specified RX buffer? */
	if (pRxframeBuf->RxSockFrame[pRxframeBuf->RxReadPtr].FrameValid == FALSE)
	{
		/* No, return to wait. */
		return (0);
	}
	else
	{
		/* Yes, read out the received TCP frame in RX buffer. */
		if (pRxframeBuf->RxSockFrame[pRxframeBuf->RxReadPtr].RxFrameSize != 0)
		{
			/* Read out the remote node address information. */
			pRemoteAddrPtr->addr[0] = (pRxframeBuf->RxSockFrame[pRxframeBuf->RxReadPtr].RemoteNodeAddr).addr[0];
			pRemoteAddrPtr->domain  = (pRxframeBuf->RxSockFrame[pRxframeBuf->RxReadPtr].RemoteNodeAddr).domain;
			pRemoteAddrPtr->port    = (pRxframeBuf->RxSockFrame[pRxframeBuf->RxReadPtr].RemoteNodeAddr).port;

			/* Return the received socket frame buffer read index value. */
			*pRxIndex  = pRxframeBuf->RxReadPtr;
			*pRxLength = pRxframeBuf->RxSockFrame[pRxframeBuf->RxReadPtr].RxFrameSize;
			*pSockIdx  = pRxframeBuf->RxSockFrame[pRxframeBuf->RxReadPtr].RxSockIdx;

			pRxframeBuf->RxReadPtr++;

			if (pRxframeBuf->RxReadPtr >= ETH_RX_FRAME_BUF_SIZE)
			{
				pRxframeBuf->RxReadPtr = 0;
			}

			return (1);
		}
		else
		{
			return (0);
		}
	}
}

/*
@brief
@details
@para
@return
*/
static int Ethernet_TcpIp_Receive(TcpIp_SocketIdType*     ulSocketIdx,
		                          TcpIp_SockAddrInetType* pRemoteNodeAddrPtr,
								  uint16_t*               rxLength,
								  uint8_t*                ptrRxIdx)
{
	int result;
	TcpIp_SocketIdType RxSocketId;
	uint16_t RxLength;

	result = Socket_ReadRxFrameBuffer(&RxSocketId, pRemoteNodeAddrPtr, &RxLength, ptrRxIdx);

	if (result != 0U)
	{
		if ((RxSocketId <= TCPIP_TCPSOCKETMAX) && (RxLength <= RBA_ETHTCP_DFL_MSS))
		{
			*rxLength    = RxLength;
			*ulSocketIdx = RxSocketId;

			result = 1;
		}
		else
		{
			*rxLength = 0;
			*ulSocketIdx = 0xFFFF;

			result = 0;
		}
	}
	else
	{
		*rxLength = 0;
		*ulSocketIdx = 0xFFFF;

		result = 0;
	}

	return (result);
}

/*
@brief
@details
@para
@return
*/
int Ethernet_ReceiveData_MainFunction(void) 
{
    int 					result;
    int 					retVal;
    TcpIp_SocketIdType      rxSocketId;
    TcpIp_SockAddrInetType  rxRemoteAddr;
    uint16_t 				rxDataSize;
    uint8_t                 rxBufIndex;
    uint8_t                 rxDataValid;
    SocketRxFrameBufferDef  *pRxframeBuf;

    retVal 		= -1;
    pRxframeBuf = (&SocketRxFrameBuffer);

    result = Ethernet_TcpIp_Receive(&rxSocketId, &rxRemoteAddr, &rxDataSize, &rxBufIndex);

    if (result == 1) 
    {
    	if (Rx_EthFrameStartWaitingFlag[rxSocketId] == 1)
    	{
    		retVal = -1;

    		pRxframeBuf->RxReadPtr = rxBufIndex;

    		return (retVal);
    	}

    	if ((rxRemoteAddr.domain == CCV_POS_RemoteAddress.domain) && (rxRemoteAddr.addr[0] == CCV_POS_RemoteAddress.addr[0]) && (rxRemoteAddr.port == CCV_POS_RemoteAddress.port))
        {
            if (CCV_RxFramePbuffer.pbufWriteIndex < P_BUFFER_SIZE)
            {
            	if (CCV_RxFramePbuffer.rxFrameBuff[CCV_RxFramePbuffer.pbufWriteIndex].effectiveFlag == FALSE)
            	{
					CCV_RxFramePbuffer.rxFrameBuff[CCV_RxFramePbuffer.pbufWriteIndex].dataSize 		= rxDataSize;
					CCV_RxFramePbuffer.rxFrameBuff[CCV_RxFramePbuffer.pbufWriteIndex].sockBuffIdx   = rxBufIndex;
					CCV_RxFramePbuffer.rxFrameBuff[CCV_RxFramePbuffer.pbufWriteIndex].pBuff    		= pRxframeBuf->RxSockFrame[rxBufIndex].RxDataBuf;
					CCV_RxFramePbuffer.rxFrameBuff[CCV_RxFramePbuffer.pbufWriteIndex].rxMoreDataFlg = pRxframeBuf->RxSockFrame[rxBufIndex].RxMoreDataStatus;
					CCV_RxFramePbuffer.rxFrameBuff[CCV_RxFramePbuffer.pbufWriteIndex].effectiveFlag = TRUE;

					CCV_RxFramePbuffer.pbufWriteIndex++;

					if (CCV_RxFramePbuffer.pbufWriteIndex >= P_BUFFER_SIZE)
					{
						CCV_RxFramePbuffer.pbufWriteIndex = 0;
					}

					rxDataValid = 1;
            	}
            	else
            	{
            		rxDataValid = 0;
            	}
            }
            else
            {
            	rxDataValid = 0;
            }
        }
    	else if ((rxRemoteAddr.domain == LEFT_LEM_RemoteAddress.domain) && (rxRemoteAddr.addr[0] == LEFT_LEM_RemoteAddress.addr[0]) && (rxRemoteAddr.port == LEFT_LEM_RemoteAddress.port))
        {
            if (LEFT_LEM_RxFramePbuffer.pbufWriteIndex < P_BUFFER_SIZE)
            {
            	if (LEFT_LEM_RxFramePbuffer.rxFrameBuff[LEFT_LEM_RxFramePbuffer.pbufWriteIndex].effectiveFlag == FALSE)
            	{
					LEFT_LEM_RxFramePbuffer.rxFrameBuff[LEFT_LEM_RxFramePbuffer.pbufWriteIndex].dataSize 	  = rxDataSize;
					LEFT_LEM_RxFramePbuffer.rxFrameBuff[LEFT_LEM_RxFramePbuffer.pbufWriteIndex].sockBuffIdx   = rxBufIndex;
					LEFT_LEM_RxFramePbuffer.rxFrameBuff[LEFT_LEM_RxFramePbuffer.pbufWriteIndex].pBuff    	  = pRxframeBuf->RxSockFrame[rxBufIndex].RxDataBuf;
					LEFT_LEM_RxFramePbuffer.rxFrameBuff[LEFT_LEM_RxFramePbuffer.pbufWriteIndex].rxMoreDataFlg = pRxframeBuf->RxSockFrame[rxBufIndex].RxMoreDataStatus;
					LEFT_LEM_RxFramePbuffer.rxFrameBuff[LEFT_LEM_RxFramePbuffer.pbufWriteIndex].effectiveFlag = TRUE;

					LEFT_LEM_RxFramePbuffer.pbufWriteIndex++;

					if (LEFT_LEM_RxFramePbuffer.pbufWriteIndex >= P_BUFFER_SIZE)
					{
						LEFT_LEM_RxFramePbuffer.pbufWriteIndex = 0;
					}

					rxDataValid = 1;
            	}
            	else
            	{
            		rxDataValid = 0;
            	}
            }
            else
            {
            	rxDataValid = 0;
            }
        }
    	else if ((rxRemoteAddr.domain == RIGHT_LEM_RemoteAddess.domain) && (rxRemoteAddr.addr[0] == RIGHT_LEM_RemoteAddess.addr[0]) && (rxRemoteAddr.port == RIGHT_LEM_RemoteAddess.port))
        {
            if (RIGHT_LEM_RxFramePbuffer.pbufWriteIndex < P_BUFFER_SIZE)
            {
            	if (RIGHT_LEM_RxFramePbuffer.rxFrameBuff[RIGHT_LEM_RxFramePbuffer.pbufWriteIndex].effectiveFlag == FALSE)
            	{
					RIGHT_LEM_RxFramePbuffer.rxFrameBuff[RIGHT_LEM_RxFramePbuffer.pbufWriteIndex].dataSize 		= rxDataSize;
					RIGHT_LEM_RxFramePbuffer.rxFrameBuff[RIGHT_LEM_RxFramePbuffer.pbufWriteIndex].sockBuffIdx   = rxBufIndex;
					RIGHT_LEM_RxFramePbuffer.rxFrameBuff[RIGHT_LEM_RxFramePbuffer.pbufWriteIndex].pBuff    		= pRxframeBuf->RxSockFrame[rxBufIndex].RxDataBuf;
					RIGHT_LEM_RxFramePbuffer.rxFrameBuff[RIGHT_LEM_RxFramePbuffer.pbufWriteIndex].rxMoreDataFlg = pRxframeBuf->RxSockFrame[rxBufIndex].RxMoreDataStatus;
					RIGHT_LEM_RxFramePbuffer.rxFrameBuff[RIGHT_LEM_RxFramePbuffer.pbufWriteIndex].effectiveFlag = TRUE;

					RIGHT_LEM_RxFramePbuffer.pbufWriteIndex++;

					if (RIGHT_LEM_RxFramePbuffer.pbufWriteIndex >= P_BUFFER_SIZE)
					{
						RIGHT_LEM_RxFramePbuffer.pbufWriteIndex = 0;
					}

					rxDataValid = 1;
            	}
            	else
            	{
            		rxDataValid = 0;
            	}
            }
            else
            {
            	rxDataValid = 0;
            }
        }
    	else if ((rxRemoteAddr.domain == Maintain_RemoteAddress.domain) && (rxRemoteAddr.addr[0] == Maintain_RemoteAddress.addr[0]) && (rxRemoteAddr.port == Maintain_RemoteAddress.port))
        {
            if (Maintain_RxFramePbuffer.pbufWriteIndex < P_BUFFER_SIZE)
            {
            	if (Maintain_RxFramePbuffer.rxFrameBuff[Maintain_RxFramePbuffer.pbufWriteIndex].effectiveFlag == FALSE)
            	{
					Maintain_RxFramePbuffer.rxFrameBuff[Maintain_RxFramePbuffer.pbufWriteIndex].dataSize 	   = rxDataSize;
					Maintain_RxFramePbuffer.rxFrameBuff[Maintain_RxFramePbuffer.pbufWriteIndex].sockBuffIdx    = rxBufIndex;
					Maintain_RxFramePbuffer.rxFrameBuff[Maintain_RxFramePbuffer.pbufWriteIndex].pBuff   	   = pRxframeBuf->RxSockFrame[rxBufIndex].RxDataBuf;
					Maintain_RxFramePbuffer.rxFrameBuff[Maintain_RxFramePbuffer.pbufWriteIndex].rxMoreDataFlg  = pRxframeBuf->RxSockFrame[rxBufIndex].RxMoreDataStatus;
					Maintain_RxFramePbuffer.rxFrameBuff[Maintain_RxFramePbuffer.pbufWriteIndex].effectiveFlag  = TRUE;

					Maintain_RxFramePbuffer.pbufWriteIndex++;

					if (Maintain_RxFramePbuffer.pbufWriteIndex >= P_BUFFER_SIZE)
					{
						Maintain_RxFramePbuffer.pbufWriteIndex = 0;
					}

					rxDataValid = 1;
            	}
            	else
            	{
            		rxDataValid = 0;
            	}
            }
            else
            {
            	rxDataValid = 0;
            }
        }
    	else if ((rxRemoteAddr.domain == Tbox1_RemoteAddress.domain) && (rxRemoteAddr.addr[0] == Tbox1_RemoteAddress.addr[0]) && (rxRemoteAddr.port == Tbox1_RemoteAddress.port))
        {
            if (Tbox1_RxFramePbuffer.pbufWriteIndex < P_BUFFER_SIZE)
            {
            	if (Tbox1_RxFramePbuffer.rxFrameBuff[Tbox1_RxFramePbuffer.pbufWriteIndex].effectiveFlag == FALSE)
            	{
					Tbox1_RxFramePbuffer.rxFrameBuff[Tbox1_RxFramePbuffer.pbufWriteIndex].dataSize		 = rxDataSize;
					Tbox1_RxFramePbuffer.rxFrameBuff[Tbox1_RxFramePbuffer.pbufWriteIndex].sockBuffIdx    = rxBufIndex;
					Tbox1_RxFramePbuffer.rxFrameBuff[Tbox1_RxFramePbuffer.pbufWriteIndex].pBuff    		 = pRxframeBuf->RxSockFrame[rxBufIndex].RxDataBuf;
					Tbox1_RxFramePbuffer.rxFrameBuff[Tbox1_RxFramePbuffer.pbufWriteIndex].rxMoreDataFlg  = pRxframeBuf->RxSockFrame[rxBufIndex].RxMoreDataStatus;
					Tbox1_RxFramePbuffer.rxFrameBuff[Tbox1_RxFramePbuffer.pbufWriteIndex].effectiveFlag  = TRUE;

					Tbox1_RxFramePbuffer.pbufWriteIndex++;

					if (Tbox1_RxFramePbuffer.pbufWriteIndex >= P_BUFFER_SIZE)
					{
						Tbox1_RxFramePbuffer.pbufWriteIndex = 0;
					}

					rxDataValid = 1;
            	}
            	else
            	{
            		rxDataValid = 0;
            	}
            }
            else
            {
            	rxDataValid = 0;
            }
        }
        else
        {
        	rxDataValid = 0;
        }

        if (rxDataValid == 0)
        {
        	pRxframeBuf->RxSockFrame[rxBufIndex].FrameValid  	  = FALSE;
        	pRxframeBuf->RxSockFrame[rxBufIndex].RxFrameSize 	  = 0;
        	pRxframeBuf->RxSockFrame[rxBufIndex].RxSockIdx   	  = 0xFFFF;
        	pRxframeBuf->RxSockFrame[rxBufIndex].RxMoreDataStatus = 0;
        	memset((uint8_t*)(&(pRxframeBuf->RxSockFrame[rxBufIndex].RxDataBuf[0])),   0, sizeof(pRxframeBuf->RxSockFrame[rxBufIndex].RxDataBuf));
        	memset((uint8_t*)(&(pRxframeBuf->RxSockFrame[rxBufIndex].RemoteNodeAddr)), 0, sizeof(TcpIp_SockAddrInetType));
        }

        retVal = 0;
    }

    return (retVal);
}

/*
@brief
@details
@para
@return
*/
static Eth_RxStatusType Ethernet_ModulePollingRxData(uint8_t* rxDataPtr, uint16_t* rxDataSize, FramePbufferInfo* pModulePbuffer)
{
	Eth_RxStatusType 		ethRxStat;
	SocketRxFrameBufferDef 	*pRxframeBuf;
    uint8_t  				bufferRing;
    uint8_t  				rxBufferIndx;
    uint8_t  				rxDataEffectiveFlg;
    uint8_t  				*pDataBuffer;
    uint16_t 				rxDataLength;
    uint8_t  				rxMoreDataFlag;

    pRxframeBuf = (&SocketRxFrameBuffer);

    bufferRing = pModulePbuffer->pbufReadIndex;

	if (bufferRing >= P_BUFFER_SIZE)
	{
		ethRxStat = ETH_NOT_RECEIVED;

		return (ethRxStat);
	}

	rxDataEffectiveFlg = pModulePbuffer->rxFrameBuff[bufferRing].effectiveFlag;

	if (rxDataEffectiveFlg == TRUE)
	{
		pModulePbuffer->rxFrameBuff[bufferRing].effectiveFlag = FALSE;

		pDataBuffer  = pModulePbuffer->rxFrameBuff[bufferRing].pBuff;
		rxDataLength = pModulePbuffer->rxFrameBuff[bufferRing].dataSize;

		memcpy(rxDataPtr,  pDataBuffer, rxDataLength);

		*rxDataSize = pModulePbuffer->rxFrameBuff[bufferRing].dataSize;
		pModulePbuffer->rxFrameBuff[bufferRing].dataSize = 0;

		rxMoreDataFlag = pModulePbuffer->rxFrameBuff[bufferRing].rxMoreDataFlg;
		pModulePbuffer->rxFrameBuff[bufferRing].rxMoreDataFlg = 0;

		rxBufferIndx = pModulePbuffer->rxFrameBuff[bufferRing].sockBuffIdx;
		pModulePbuffer->rxFrameBuff[bufferRing].sockBuffIdx = 0;

		pRxframeBuf->RxSockFrame[rxBufferIndx].FrameValid  = FALSE;
		pRxframeBuf->RxSockFrame[rxBufferIndx].RxFrameSize = 0;
		pRxframeBuf->RxSockFrame[rxBufferIndx].RxSockIdx   = 0xFFFF;
		pRxframeBuf->RxSockFrame[rxBufferIndx].RxMoreDataStatus = 0;
		memset((uint8_t*)(&(pRxframeBuf->RxSockFrame[rxBufferIndx].RemoteNodeAddr)), 0, sizeof(TcpIp_SockAddrInetType));
		memset((uint8_t*)(&(pRxframeBuf->RxSockFrame[rxBufferIndx].RxDataBuf[0])),   0, rxDataLength);

		bufferRing++;

		if (bufferRing >= P_BUFFER_SIZE)
		{
			bufferRing = 0;
		}

		pModulePbuffer->pbufReadIndex = bufferRing;

		if (rxMoreDataFlag != 0)
		{
			ethRxStat = ETH_RECEIVED_MORE_DATA_AVAILABLE;
		}
		else
		{
			ethRxStat = ETH_RECEIVED;
		}
	}
	else
	{
		ethRxStat = ETH_NOT_RECEIVED;
	}

	return (ethRxStat);
}

/*
@brief
@details
@para
@return
*/
int Ethernet_CCV_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength) 
{
	static uint16_t  offset = 0;
	Eth_RxStatusType ethRxState;
	uint8_t 		 *pRxDataBufferBase;
	uint16_t 		 rxdataSize;
	int 			 result;

	pRxDataBufferBase = pRxDataBuf + offset;

	ethRxState = Ethernet_ModulePollingRxData(pRxDataBufferBase, &rxdataSize, &CCV_RxFramePbuffer);

	if (ethRxState == ETH_NOT_RECEIVED)
	{
		offset = 0;

		result = -1;
	}
	else if (ethRxState == ETH_RECEIVED_MORE_DATA_AVAILABLE)
	{
		*pDataLength = rxdataSize + offset;

		offset += rxdataSize;

		result = -2;
	}
	else if (ethRxState == ETH_RECEIVED)
	{
		*pDataLength = rxdataSize + offset;

		offset = 0;

		result = 0;
	}
	else
	{
		offset = 0;

		result = -1;
	}

	return (result);
}

/*
@brief
@details
@para
@return
*/
int Ethernet_LeftLEM_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength) 
{
	static uint16_t  offset = 0;
	Eth_RxStatusType ethRxState;
	uint8_t 		 *pRxDataBufferBase;
	uint16_t 		 rxdataSize;
	int 			 result;

	pRxDataBufferBase = pRxDataBuf + offset;

	ethRxState = Ethernet_ModulePollingRxData(pRxDataBufferBase, &rxdataSize, &LEFT_LEM_RxFramePbuffer);

	if (ethRxState == ETH_NOT_RECEIVED)
	{
		offset = 0;

		result = -1;
	}
	else if (ethRxState == ETH_RECEIVED_MORE_DATA_AVAILABLE)
	{
		*pDataLength = rxdataSize + offset;

		offset += rxdataSize;

		result = -2;
	}
	else if (ethRxState == ETH_RECEIVED)
	{
		*pDataLength = rxdataSize + offset;

		offset = 0;

		result = 0;
	}
	else
	{
		offset = 0;

		result = -1;
	}

	return (result);
}

/*
@brief
@details
@para
@return
*/
int Ethernet_RightLEM_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength) 
{
	static uint16_t  offset = 0;
	Eth_RxStatusType ethRxState;
	uint8_t 		 *pRxDataBufferBase;
	uint16_t 		 rxdataSize;
	int 			 result;

	pRxDataBufferBase = pRxDataBuf + offset;

	ethRxState = Ethernet_ModulePollingRxData(pRxDataBufferBase, &rxdataSize, &RIGHT_LEM_RxFramePbuffer);

	if (ethRxState == ETH_NOT_RECEIVED)
	{
		offset = 0;

		result = -1;
	}
	else if (ethRxState == ETH_RECEIVED_MORE_DATA_AVAILABLE)
	{
		*pDataLength = rxdataSize + offset;

		offset += rxdataSize;

		result = -2;
	}
	else if (ethRxState == ETH_RECEIVED)
	{
		*pDataLength = rxdataSize + offset;

		offset = 0;

		result = 0;
	}
	else
	{
		offset = 0;

		result = -1;
	}

	return (result);
}

/*
@brief
@details
@para
@return
*/
int Ethernet_Maintain_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength) 
{
	static uint16_t  offset = 0;
	Eth_RxStatusType ethRxState;
	uint8_t 		 *pRxDataBufferBase;
	uint16_t 		 rxdataSize;
	int 			 result;

	pRxDataBufferBase = pRxDataBuf + offset;

	ethRxState = Ethernet_ModulePollingRxData(pRxDataBufferBase, &rxdataSize, &Maintain_RxFramePbuffer);

	if (ethRxState == ETH_NOT_RECEIVED)
	{
		offset = 0;

		result = -1;
	}
	else if (ethRxState == ETH_RECEIVED_MORE_DATA_AVAILABLE)
	{
		*pDataLength = rxdataSize + offset;

		offset += rxdataSize;

		result = -2;
	}
	else if (ethRxState == ETH_RECEIVED)
	{
		*pDataLength = rxdataSize + offset;

		offset = 0;

		result = 0;
	}
	else
	{
		offset = 0;

		result = -1;
	}

	return (result);
}

/*
@brief
@details
@para
@return
*/
int Ethernet_Tbox1_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength) 
{
	static uint16_t  offset = 0;
	Eth_RxStatusType ethRxState;
	uint8_t 		 *pRxDataBufferBase;
	uint16_t 		 rxdataSize;
	int 			 result;

	pRxDataBufferBase = pRxDataBuf + offset;

	ethRxState = Ethernet_ModulePollingRxData(pRxDataBufferBase, &rxdataSize, &Tbox1_RxFramePbuffer);

	if (ethRxState == ETH_NOT_RECEIVED)
	{
		offset = 0;

		result = -1;
	}
	else if (ethRxState == ETH_RECEIVED_MORE_DATA_AVAILABLE)
	{
		*pDataLength = rxdataSize + offset;

		offset += rxdataSize;

		result = -2;
	}
	else if (ethRxState == ETH_RECEIVED)
	{
		*pDataLength = rxdataSize + offset;

		offset = 0;

		result = 0;
	}
	else
	{
		offset = 0;

		result = -1;
	}

	return (result);
}

/*
@brief
@details
@para
@return
*/
static void Ethernet_Init(void)
{
	Std_ReturnType retVal;
	uint8_t dummyCnt;

	/*Set SIU_FECCR register manually to select MII-Lite. API is avalable in MCAL4.3*/
	REG_WRITE32(SIU_FECCR_REG_ADDR, (uint32)0x00000001UL);

	/* Initializes the Ethernet Driver. */
	Eth_Init(&EthConfigSet_0);

	/* Initializes the Ethernet Driver. */
	retVal = Eth_ControllerInit(CTRL_INDEX, CFG_INDEX);

	if (retVal == E_OK)
	{
		/* Enables the given controller. */
		retVal = Eth_SetControllerMode(CTRL_INDEX, ETH_MODE_ACTIVE);

		if (retVal == E_OK)
		{
			dummyCnt = 0;
			/* Execute several empty operation to enable system can set breakpoint here when in debug mode. */
			while (dummyCnt < 3)
			{
                dummyCnt++;
			}
		}
		else
		{
			dummyCnt = 0;
			/* Execute several empty operation to enable system can set breakpoint here when in debug mode. */
			while (dummyCnt < 3)
			{
                dummyCnt++;
			}
		}
	}
	else
	{
		dummyCnt = 0;
		/* Execute several empty operation to enable system can set breakpoint here when in debug mode. */
		while (dummyCnt < 3)
		{
            dummyCnt++;
		}
	}
}

/*
@brief
@details
@para
@return
*/
#if (ENABLE_TCP_CLIENT != 0)
int Ethernet_TcpClient_Send(TcpIp_PortType uLocalPort, TcpIp_SockAddrInetType *pRemoteAddrPtr, uint8_t *pTxDatabuf, uint16_t txLength)
{
	Eth_CommInfoDef               *EthCommInfPtr;
	uint8_t                       CountIndex;
	int                           result;
    Std_ReturnType                fucRetVal;
    TcpIp_SocketIdType            specSocketId;

    fucRetVal = TCPIP_E_STATUS_ERROR;
    specSocketId = 0xFFFF;

	if ((pTxDatabuf == NULL_PTR) || (txLength > RBA_ETHTCP_DFL_MSS))
	{
        fucRetVal = TCPIP_E_STATUS_ERROR;
        
		return (fucRetVal);
	}

	EthCommInfPtr = (&EthernetCommInfo);

	if (EthCommInfPtr->IpAssignmentStat != TCPIP_IP_ADDR_ASSIGNED)
	{
        fucRetVal = TCPIP_E_STATUS_ERROR;
        
		return (fucRetVal);
	}

	for (CountIndex = 0; CountIndex < TCPIP_TCPSOCKETMAX; CountIndex++)
	{
		/* The specified local port must match with the Ethernet communication info local port. */
		if (uLocalPort == EthCommInfPtr->SocketInfo[CountIndex].LocalPort)
		{
			/* The specified remote address parameters must match with the Ethernet communication info remote socket address para. */
			if ((pRemoteAddrPtr->port    == EthCommInfPtr->SocketInfo[CountIndex].RemoteSockAddr.port)    &&
				(pRemoteAddrPtr->addr[0] == EthCommInfPtr->SocketInfo[CountIndex].RemoteSockAddr.addr[0]) &&
				(pRemoteAddrPtr->domain  == EthCommInfPtr->SocketInfo[CountIndex].RemoteSockAddr.domain))
			{
				/* Get the corresponding socket id value. */
                specSocketId = EthCommInfPtr->SocketInfo[CountIndex].SocketId;

				break;
			}
		}
	}

    if (specSocketId >= TCPIP_TCPSOCKETMAX)
    {
        fucRetVal = TCPIP_E_STATUS_ERROR;

		return (fucRetVal);
    }

	result = 0;

	while (result == 0)
	{
        switch (EthCommInfPtr->SocketInfo[specSocketId].CurrentStat)
		{
			case TCPIP_POLL_CONNECTSTATUS:
			{
                fucRetVal = Ethernet_TcpClient_StartTransmit(specSocketId, TCPIP_TCP_CLIENT, pTxDatabuf, txLength);

                if (fucRetVal == E_OK) 
                {
                    fucRetVal = TCPIP_E_STATUS_OK;
                }
                else 
                {
                    fucRetVal = TCPIP_E_STATUS_ERROR;
                }

				result = -1;
			}break;

			default: result = -1; break;
		}
	}

	return (fucRetVal);
}
#endif

/*
@brief
@details
@para
@return
*/
int Ethernet_TcpClient_Transmit(TcpIp_PortType u16localPort, TcpIp_SockAddrInetType *remoteAddrPtr, uint8_t *txDatabuffer, uint16_t txDataSize) 
{
    int      funcCallRes;
    uint16_t lengthOffset;
    uint16_t count;
    int      result;
    uint16_t multipleMSS;
    uint16_t remainSize;
    uint8_t  dummyCount;

    if (txDataSize < RBA_ETHTCP_DFL_MSS) 
    {
        lengthOffset = 0;

        funcCallRes = Ethernet_TcpClient_Send(u16localPort, remoteAddrPtr, (txDatabuffer + lengthOffset), txDataSize);
        
        if (funcCallRes == TCPIP_E_STATUS_ERROR) 
        {
            result = -1;
        }
        else 
        {
            result = 0;
        }        

        return (result);
    }
    else 
    {
        multipleMSS = txDataSize / RBA_ETHTCP_DFL_MSS;
        remainSize  = txDataSize % RBA_ETHTCP_DFL_MSS;

        lengthOffset = 0;
        result       = 0;

        for (count = 0; count < multipleMSS; count++) 
        {
            funcCallRes = Ethernet_TcpClient_Send(u16localPort, remoteAddrPtr, (txDatabuffer + lengthOffset), RBA_ETHTCP_DFL_MSS);

            if (funcCallRes == TCPIP_E_STATUS_ERROR) 
            {
                result = -1;
                
                dummyCount = 0;

                while (dummyCount < 3)dummyCount++;

                break;
            }
            else 
            {
                result = 0;
                
                lengthOffset += RBA_ETHTCP_DFL_MSS;

                dummyCount = 0;

                while (dummyCount < 1)dummyCount++;
            }
        }

        if (result == 0) 
        {
            funcCallRes = Ethernet_TcpClient_Send(u16localPort, remoteAddrPtr, (txDatabuffer + lengthOffset), remainSize);
            
            if (funcCallRes == TCPIP_E_STATUS_ERROR) 
            {
                result = -1;

                dummyCount = 0;

                while (dummyCount < 3)dummyCount++;
            }
            else 
            {
                result = 0;

                dummyCount = 0;

                while (dummyCount < 3)dummyCount++;
            }
        }

        return (result);
    }
}

/*
@brief
@details
@para
@return
*/
void Ethernet_TcpSocket_MainFunction(void)
{
    uint16_t countIdx;
    int statusVal;
    Std_ReturnType ret;
    Eth_CommInfoDef *EthCommInfPtr;

	EthCommInfPtr = (&EthernetCommInfo);

	if (EthCommInfPtr->IpAssignmentStat != TCPIP_IP_ADDR_ASSIGNED)
	{
		return;
	}

	EthCommInfPtr->SocketPollTimeCnt++;

	if (EthCommInfPtr->SocketPollTimeCnt >= 5U)
	{
		EthCommInfPtr->SocketPollTimeCnt = 0;

		for (countIdx = 0; countIdx < TCPIP_TCPSOCKETMAX; countIdx++)
		{
			statusVal = 0;

			while (statusVal == 0)
			{
				switch (EthCommInfPtr->SocketInfo[countIdx].CurrentStat)
				{
					case TCPIP_REQ_CREATE_SOCKET:
					{
						TcpClientLocalPort_array[countIdx] = Ethernet_GetRandom(EthComm_1MS_Counter);

						ret = Ethernet_TcpClient_CreateSocket(TcpClientLocalPort_array[countIdx], &RemoteAddress_array[countIdx]);

						if (ret == E_OK)
						{
							statusVal = 0;
						}
						else
						{
							statusVal = -1;
						}
					}break;

					case TCPIP_REQ_BIND_SOCKET:
					{
						ret = Ethernet_TcpClient_BindSocket(TCPIP_TCP_CLIENT, countIdx);

						if (ret == E_OK)
						{
							statusVal = 0;
						}
						else
						{
							statusVal = -1;
						}
					}break;

					case TCPIP_REQ_CONNECTING:
					{
						ret = Ethernet_TcpClient_ConnectServer(countIdx, &RemoteAddress_array[countIdx]);

						if (ret == E_OK)
						{
							statusVal = 0;
						}
						else
						{
							statusVal = -1;
						}
					}break;

					case TCPIP_POLL_CONNECTSTATUS:
					{
						Ethernet_TcpClient_PollingStatus(TcpClientLocalPort_array[countIdx], TCPIP_TCP_CLIENT);

						statusVal = -1;
					}break;

					case TCPIP_CONNECT_SERVER_FAILED:
					{
						Ethernet_TcpClient_StopTransmit(countIdx, TCPIP_TCP_CLIENT);

						statusVal = -1;
					}break;

					default: statusVal = -1; break;
				}
			}
		}
	}
}

/*
@brief
@details
@para
@return
*/
#if (ENABLE_TCP_CLIENT != 0)
static Std_ReturnType Ethernet_TcpClient_CreateSocket(TcpIp_PortType lPortNum, TcpIp_SockAddrInetType *pRemoteAddrPtr)
{
	uint8_t             effectiveSta;
	Std_ReturnType      RetVal;
	Eth_CommInfoDef     *EthCommInfPtr;
	TcpIp_SocketIdType  CreateSocketIdx;
	Std_ReturnType      result;
	rba_EthTcp_DynSockTblType_tst   *pDynSockPtr;

	EthCommInfPtr = (&EthernetCommInfo);
    
	result = E_NOT_OK;

	RetVal = TcpIp_GetSocket(TCPIP_AF_INET, TCPIP_IPPROTO_TCP, &CreateSocketIdx);

	if (RetVal == E_OK)
	{
		if (CreateSocketIdx < TCPIP_TCPSOCKETMAX)
		{
			pDynSockPtr = &rba_EthTcp_DynSockTbl_ast[CreateSocketIdx];

			if (lPortNum == TcpClientLocalPort_array[CreateSocketIdx])
			{
				/* Assign Switch port value by remote server address and port info. */
				if ((pRemoteAddrPtr->addr[0] == CCV_POS_RemoteAddress.addr[0]) && (pRemoteAddrPtr->port == CCV_POS_RemoteAddress.port))
				{
					EthCommInfPtr->SocketInfo[CreateSocketIdx].SwitchPort = SWITCH_PORT_P2;
					effectiveSta = 1;
				}
				else if ((pRemoteAddrPtr->addr[0] == LEFT_LEM_RemoteAddress.addr[0]) && (pRemoteAddrPtr->port == LEFT_LEM_RemoteAddress.port))
				{
					EthCommInfPtr->SocketInfo[CreateSocketIdx].SwitchPort = SWITCH_PORT_P2;
					effectiveSta = 1;
				}
				else if ((pRemoteAddrPtr->addr[0] == RIGHT_LEM_RemoteAddess.addr[0]) && (pRemoteAddrPtr->port == RIGHT_LEM_RemoteAddess.port))
				{
					EthCommInfPtr->SocketInfo[CreateSocketIdx].SwitchPort = SWITCH_PORT_P2;
					effectiveSta = 1;
				}
				else if ((pRemoteAddrPtr->addr[0] == Maintain_RemoteAddress.addr[0]) && (pRemoteAddrPtr->port == Maintain_RemoteAddress.port))
				{
					EthCommInfPtr->SocketInfo[CreateSocketIdx].SwitchPort = SWITCH_PORT_P3;
					effectiveSta = 1;
				}
				else if ((pRemoteAddrPtr->addr[0] == Tbox1_RemoteAddress.addr[0]) && (pRemoteAddrPtr->port == Tbox1_RemoteAddress.port))
				{
					EthCommInfPtr->SocketInfo[CreateSocketIdx].SwitchPort = SWITCH_PORT_P0;
					effectiveSta = 1;
				}
				else
				{
					EthCommInfPtr->SocketInfo[CreateSocketIdx].SwitchPort = SWITCH_PORT_INVALID;
					effectiveSta = 0;

					pDynSockPtr->SockState_en 	= RBA_ETHTCP_SOCK_CLOSED_E;
					pDynSockPtr->DomainType_u32 = TCPIP_AF_NONE;

					if (rba_EthTcp_SockConnCntr_u16 != 0)
					{
						rba_EthTcp_SockConnCntr_u16--;
					}
				}

				if (effectiveSta == 1U)
				{
					EthCommInfPtr->SocketInfo[CreateSocketIdx].LocalPort              = lPortNum;
					EthCommInfPtr->SocketInfo[CreateSocketIdx].CurrentStat            = TCPIP_REQ_BIND_SOCKET;
					EthCommInfPtr->SocketInfo[CreateSocketIdx].SocketId     		  = CreateSocketIdx;
					EthCommInfPtr->SocketInfo[CreateSocketIdx].ConnectingCnt		  = 0;
					EthCommInfPtr->SocketInfo[CreateSocketIdx].RemoteSockAddr.domain  = pRemoteAddrPtr->domain;
					EthCommInfPtr->SocketInfo[CreateSocketIdx].RemoteSockAddr.port    = pRemoteAddrPtr->port;
					EthCommInfPtr->SocketInfo[CreateSocketIdx].RemoteSockAddr.addr[0] = pRemoteAddrPtr->addr[0];

				    result = E_OK;
				}
			}
			else
			{
				pDynSockPtr->SockState_en 	= RBA_ETHTCP_SOCK_CLOSED_E;
				pDynSockPtr->DomainType_u32 = TCPIP_AF_NONE;

				if (rba_EthTcp_SockConnCntr_u16 != 0)
				{
					rba_EthTcp_SockConnCntr_u16--;
				}
			}
		}
	}

	return (result);
}
#endif

/*
@brief
@details
@para
@return
*/
#if (ENABLE_TCP_CLIENT != 0)
static Std_ReturnType Ethernet_TcpClient_BindSocket(TcpIp_ProtocalTypeDef ptType, TcpIp_SocketIdType lSocketIdx)
{
	Std_ReturnType      retVal;
	TcpIp_SocketIdType  SpecSocketId;
	TcpIp_PortType      SpecLocalPort;
	Eth_CommInfoDef     *EthCommInfPtr;
	Std_ReturnType      result;

	if (ptType != TCPIP_TCP_CLIENT)
	{
		result = E_NOT_OK;

		return (result);
	}

	EthCommInfPtr = (&EthernetCommInfo);
	SpecSocketId  = EthCommInfPtr->SocketInfo[lSocketIdx].SocketId;
	SpecLocalPort = EthCommInfPtr->SocketInfo[lSocketIdx].LocalPort;

	/* Bind the new created socket ID with local port number. */
	/* The same local IP address is bound to different port Numbers to build the Socket. */
	retVal = TcpIp_Bind(SpecSocketId, TcpIpConf_TcpIpLocalAddr_TcpIpLocalAddr, &SpecLocalPort);

	if (retVal == E_OK)
	{
		EthCommInfPtr->SocketInfo[lSocketIdx].CurrentStat = TCPIP_REQ_CONNECTING;

		result = E_OK;
	}
	else
	{
        /* If binding socket failed,user should close the previous created socket.
           Then the current status should be set with TCPIP_CONNECT_SERVER_FAILED. */
		EthCommInfPtr->SocketInfo[lSocketIdx].CurrentStat = TCPIP_CONNECT_SERVER_FAILED;

		result = E_NOT_OK;
	}

	return (result);
}
#endif

/*
@brief
@details
@para
@return
*/
#if (ENABLE_TCP_CLIENT != 0)
static Std_ReturnType Ethernet_TcpClient_ConnectServer(TcpIp_SocketIdType lSocketIdx, TcpIp_SockAddrInetType *pRemoteAddr)
{
	Std_ReturnType      RetVal;
	TcpIp_SocketIdType  SpecSocketId;
	Eth_CommInfoDef     *EthCommInfPtr;
	Std_ReturnType      result;

	EthCommInfPtr = (&EthernetCommInfo);
	SpecSocketId  = EthCommInfPtr->SocketInfo[lSocketIdx].SocketId;

	RetVal = TcpIp_TcpConnect(SpecSocketId, ((TcpIp_SockAddrType*)(&(pRemoteAddr->domain))));

	/* If Tcp connection have been accepted,socket status will be OPENED for communication */
	if (RetVal == E_OK)
	{
        /* The connection to server has been accepted, but not indicate successfully. */
		EthCommInfPtr->SocketInfo[SpecSocketId].CurrentStat = TCPIP_POLL_CONNECTSTATUS;		

		result = E_OK;
	}
	/* If remote address is not effective,connection will not be accepted. */
	else
	{
		EthCommInfPtr->SocketInfo[SpecSocketId].CurrentStat = TCPIP_CONNECT_SERVER_FAILED;

		result = E_NOT_OK;
	}

	return (result);
}
#endif

/*
@brief
@details
@para
@return
*/
#if (ENABLE_TCP_CLIENT != 0)
static Std_ReturnType Ethernet_TcpClient_PollingStatus(TcpIp_PortType uPortNum, TcpIp_ProtocalTypeDef ptType)
{
	Eth_CommInfoDef                 *EthCommInfPtr;
	rba_EthTcp_DynSockTblType_tst   *pDynSockPtr;
	unsigned char                   EstabStatus;
	uint8_t                         indexCnt;
	Std_ReturnType                  result;

	result = E_NOT_OK;

	if (ptType != TCPIP_TCP_CLIENT)
	{
		result = E_NOT_OK;

		return (result);
	}

	EthCommInfPtr = (&EthernetCommInfo);

	for (indexCnt = 0; indexCnt < TCPIP_TCPSOCKETMAX; indexCnt++)
	{
		pDynSockPtr = &rba_EthTcp_DynSockTbl_ast[indexCnt];

		if (uPortNum == (pDynSockPtr->LocalPort_u16))
		{
			if ((pDynSockPtr->RemotePort_u16 != TCPIP_PORT_ANY) && (pDynSockPtr->RemoteIPAddr_un.IPv4Addr_u32 != TCPIP_IPADDR_ANY))
			{
				/* Sockets state is OPENED and TCP state should be more than CLOSED. */
				if ((pDynSockPtr->SockState_en == RBA_ETHTCP_SOCK_OPENED_E) && (pDynSockPtr->State_en > RBA_ETHTCP_CONN_CLOSED_E))
				{
					if (pDynSockPtr->State_en == RBA_ETHTCP_CONN_ESTABLISHED_E)
					{
						EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt = 0;

						if (((EthCommInfPtr->SocketInfo[indexCnt].SwitchPort == SWITCH_PORT_P0) && (TJA1101_AttributeInfo.LinkStatus == TJA1101_Linkup_Success))    || \
							((EthCommInfPtr->SocketInfo[indexCnt].SwitchPort == SWITCH_PORT_P2) && (PHY_DP83822HF_Prop[1].PhyLinkStatus == Valid_Link_Established)) || \
							((EthCommInfPtr->SocketInfo[indexCnt].SwitchPort == SWITCH_PORT_P3) && (PHY_DP83822HF_Prop[0].PhyLinkStatus == Valid_Link_Established)))
						{
							EthCommInfPtr->SocketInfo[indexCnt].Established = TRUE;
							EthCommInfPtr->SocketInfo[indexCnt].CurrentStat = TCPIP_POLL_CONNECTSTATUS;

							result = E_OK;
						}
						else
						{
							EthCommInfPtr->SocketInfo[indexCnt].Established = FALSE;
							EthCommInfPtr->SocketInfo[indexCnt].CurrentStat = TCPIP_CONNECT_SERVER_FAILED;

							result = E_NOT_OK;
						}
					}
					else if (pDynSockPtr->State_en >= RBA_ETHTCP_CONN_CLOSE_WAIT_E)
					{
						if (EthCommInfPtr->SocketInfo[indexCnt].CurrentStat == TCPIP_POLL_CONNECTSTATUS)
						{
							EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt = 0;

							EstabStatus = EthCommInfPtr->SocketInfo[indexCnt].Established;

							if (EstabStatus == TRUE)
							{
								EthCommInfPtr->SocketInfo[indexCnt].Established = FALSE;
								EthCommInfPtr->SocketInfo[indexCnt].CurrentStat = TCPIP_CONNECT_SERVER_FAILED;

								result = E_NOT_OK;
							}
						}
					}
					else if (pDynSockPtr->State_en == RBA_ETHTCP_CONN_SYN_SENT_E)
					{
						if (EthCommInfPtr->SocketInfo[indexCnt].CurrentStat == TCPIP_POLL_CONNECTSTATUS)
						{
							if ((pDynSockPtr->IniRcvdSeqNum_u32 == 0UL) && (pDynSockPtr->RcvdSeqNum_u32 == 0UL) && (pDynSockPtr->RcvdAckNum_u32 == 0UL))
							{
								EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt++;

								/* More than RETRY_TIME_SIZE attempts to connection to the remote server failed. */
								if (EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt >= RETRY_TIME_SIZE)
								{
									EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt = 0;
									EthCommInfPtr->SocketInfo[indexCnt].CurrentStat   = TCPIP_CONNECT_SERVER_FAILED;

									result = E_NOT_OK;
								}
							}
						}
					}
				}
				else
				{
//					EstabStatus = EthCommInfPtr->SocketInfo[indexCnt].Established;
//
//					if (EstabStatus == FALSE)
//					{
//						EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt++;
//
//						/* More than RETRY_TIME_SIZE attempts to connection to the remote server failed. */
//						if (EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt >= RETRY_TIME_SIZE)
//						{
//							EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt = 0;
//							EthCommInfPtr->SocketInfo[indexCnt].CurrentStat   = TCPIP_CONNECT_SERVER_FAILED;
//
//							result = E_NOT_OK;
//						}
//					}
				}
			}

			break;
		}
		else
		{
			if ((pDynSockPtr->SockState_en == RBA_ETHTCP_SOCK_CLOSED_E) && (pDynSockPtr->State_en == RBA_ETHTCP_CONN_CLOSED_E))
			{
				if (EthCommInfPtr->SocketInfo[indexCnt].CurrentStat == TCPIP_POLL_CONNECTSTATUS)
				{
					EstabStatus = EthCommInfPtr->SocketInfo[indexCnt].Established;

					if (EstabStatus == TRUE)
					{
						EthCommInfPtr->SocketInfo[indexCnt].Established = FALSE;
						EthCommInfPtr->SocketInfo[indexCnt].CurrentStat = TCPIP_CONNECT_SERVER_FAILED;

						result = E_NOT_OK;
					}
					else
					{
						EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt++;

						/* More than RETRY_TIME_SIZE attempts to connection to the remote server failed. */
						if (EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt >= RETRY_TIME_SIZE)
						{
							EthCommInfPtr->SocketInfo[indexCnt].ConnectingCnt = 0;
							EthCommInfPtr->SocketInfo[indexCnt].CurrentStat   = TCPIP_CONNECT_SERVER_FAILED;

							result = E_NOT_OK;
						}
					}
				}
			}
		}
	}

	return (result);
}
#endif

/*
@brief
@details
@para
@return
*/
#if (ENABLE_TCP_CLIENT != 0)
static Std_ReturnType Ethernet_TcpClient_StartTransmit(TcpIp_SocketIdType lSocketIdx, TcpIp_ProtocalTypeDef ptoType, uint8_t *pTxdata, uint32_t size)
{
	Eth_CommInfoDef *EthCommInfPtr;
	Std_ReturnType  retVal;
	Std_ReturnType  result;

	if (ptoType != TCPIP_TCP_CLIENT)
	{
		result = E_NOT_OK;

		return (result);
	}

	EthCommInfPtr = (&EthernetCommInfo);

	if (EthCommInfPtr->SocketInfo[lSocketIdx].Established == TRUE)
	{
		retVal = TcpIp_TcpTransmit(EthCommInfPtr->SocketInfo[lSocketIdx].SocketId, pTxdata, size, FALSE);

		if (retVal == E_OK)
		{
			result = E_OK;
		}
		else
		{
			result = E_NOT_OK;
		}
	}
    else 
    {
        result = E_NOT_OK;
    }

	return (result);
}
#endif

/*
@brief
@details
@para
@return
*/
#if (ENABLE_TCP_CLIENT != 0)
static Std_ReturnType Ethernet_TcpClient_StopTransmit(TcpIp_SocketIdType lSocketIdx, TcpIp_ProtocalTypeDef ptoType)
{
	TcpIp_SocketIdType            SpecSocketId;
	Eth_CommInfoDef               *EthCommInfPtr;
	rba_EthTcp_DynSockTblType_tst *pDynSockPtr;
	Std_ReturnType                retVal;
	Std_ReturnType                result;

	result = E_NOT_OK;

	if (ptoType != TCPIP_TCP_CLIENT)
	{
		result = E_NOT_OK;

		return (result);
	}

	EthCommInfPtr = (&EthernetCommInfo);
    SpecSocketId  = (EthCommInfPtr->SocketInfo[lSocketIdx].SocketId);
    pDynSockPtr   = (&rba_EthTcp_DynSockTbl_ast[SpecSocketId]);

	if ((pDynSockPtr->State_en >= RBA_ETHTCP_CONN_CLOSED_E) && (pDynSockPtr->SockState_en > RBA_ETHTCP_SOCK_CLOSED_E))
	{
		retVal = TcpIp_Close(SpecSocketId, TRUE);

		if (retVal == E_OK)
		{
			result = E_OK;
		}
		else
		{
			result = E_NOT_OK;
		}
	}
	else if ((pDynSockPtr->State_en == RBA_ETHTCP_CONN_CLOSED_E) && (pDynSockPtr->SockState_en == RBA_ETHTCP_SOCK_CLOSED_E))
	{
		EthCommInfPtr->SocketInfo[SpecSocketId].Established 			= FALSE;
		EthCommInfPtr->SocketInfo[SpecSocketId].ConnectingCnt			= 0U;
		EthCommInfPtr->SocketInfo[SpecSocketId].SocketId    			= 0xFFFFU;
		EthCommInfPtr->SocketInfo[SpecSocketId].LocalPort   			= TCPIP_PORT_ANY;
		EthCommInfPtr->SocketInfo[SpecSocketId].CurrentStat 			= TCPIP_REQ_CREATE_SOCKET;
		EthCommInfPtr->SocketInfo[SpecSocketId].RemoteSockAddr.addr[0] 	= TCPIP_IPADDR_ANY;
		EthCommInfPtr->SocketInfo[SpecSocketId].RemoteSockAddr.port    	= TCPIP_PORT_ANY;
		EthCommInfPtr->SocketInfo[SpecSocketId].RemoteSockAddr.domain  	= TCPIP_AF_INET;
		EthCommInfPtr->SocketInfo[SpecSocketId].SwitchPort				= SWITCH_PORT_INVALID;

		TcpClientLocalPort_array[SpecSocketId] = TCPIP_PORT_ANY;

		result = E_OK;
	}

	return (result);
}
#endif






