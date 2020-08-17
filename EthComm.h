/*
 * EthComm.h
 *
 *  Created on: 2019Äê10ÔÂ22ÈÕ
 *      Author: admin
 */

#ifndef USERS_ETHCOMM_H_
#define USERS_ETHCOMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "Mcal.h"
#include "typedefs.h"
#include "Std_Types.h"
#include "Reg_eSys.h"
#include "Eth_GeneralTypes.h"
#include "Platform_Types.h"

#include "TcpIp.h"
#include "TcpIp_Cfg.h"
#include "TcpIp_GeneralTypes.h"

#define  CTRL_INDEX								(0U)
#define  CFG_INDEX								(0U)

#define  LENGTH_MAC_ADD							(6U)
#define  ETH_RX_FRAME_BUF_SIZE					(20U)
#define  ETH_MAX_PAYLOAD_SIZE					(1500U)
#define  P_BUFFER_SIZE                          (4U)

#define  RETRY_TIME_SIZE						(10U)

#define  SIU_FECCR_REG_ADDR						(SIU_BASEADDR + 0x9E0)


#define  ENABLE_TCP_CLIENT						(1U)
#define  ENABLE_TCP_SERVER						(0U)


#define  TCPIP_E_STATUS_OK						(0x00)
#define  TCPIP_E_STATUS_ERROR					(0xFF)
#define  TCPIP_E_STATUS_PROTOTYPE				(0x10)
#define  TCPIP_E_STATUS_CONNECTION_SUCCESS		(0x11)
#define  TCPIP_E_STATUS_CONNECTION_FAILED		(0x12)


typedef  uint16									TcpIp_PortType;
typedef  uint8									TcpIp_SocketIndexType;


typedef enum
{
	User_IpMode = 0,
	Factory_IpMode
}zvt_pt_ipmode;


typedef enum
{
	TCPIP_STATUS_INVALID 		= 0,
	TCPIP_REQ_CREATE_SOCKET  	= 1,
	TCPIP_REQ_BIND_SOCKET 	 	= 2,
	TCPIP_REQ_CONNECTING 		= 3,
	TCPIP_POLL_CONNECTSTATUS	= 4,
	TCPIP_CONNECT_SERVER_OK 	= 5,
	TCPIP_CONNECT_SERVER_FAILED = 6,
	TCPIP_REQ_LISTENING 		= 7,
	TCPIP_TCP_LISTEN_SUCCESS	= 8,
	TCPIP_TCP_LISTEN_FAILED		= 9,
	TCPIP_ERROR					= 10
}TcpIp_CurStatusDef;


typedef enum
{
	TCPIP_IP_ADDR_UNASSIGNED 	= 0,
	TCPIP_IP_ADDR_ASSIGNED 		= 1
}TcpIp_IpAddrAssignmentStatusDef;


typedef enum
{
	TCPIP_INVALID_PROTOCALTYPE 	= 0,
	TCPIP_UDP 					= 1,
	TCPIP_TCP_CLIENT 			= 2,
	TCPIP_TCP_SERVER 			= 3
}TcpIp_ProtocalTypeDef;


typedef enum
{
	SWITCH_PORT_INVALID         = 0,
	SWITCH_PORT_P0				= 1,
	SWITCH_PORT_P1				= 2,
	SWITCH_PORT_P2				= 3,
	SWITCH_PORT_P3				= 4,
	SWITCH_PORT_P4				= 5
}TcpIp_SwitchPortDef;


typedef struct
{
	uint8_t							FrameValid;
	uint8_t							RxMoreDataStatus;
	uint8_t           				RxDataBuf[RBA_ETHTCP_DFL_MSS];
	uint16_t 						RxFrameSize;
	TcpIp_SocketIdType 				RxSockIdx;
	TcpIp_SockAddrInetType  		RemoteNodeAddr;
}SocketRxFrameDef;


typedef struct
{
	uint8_t 						RxWritePtr;
	uint8_t 						RxReadPtr;
	SocketRxFrameDef 				RxSockFrame[ETH_RX_FRAME_BUF_SIZE];
}SocketRxFrameBufferDef;


typedef struct 
{
    uint8_t                         effectiveFlag;
    uint8_t                         sockBuffIdx;
    uint16_t                        dataSize;
    uint8_t                         *pBuff;
    uint8_t                         rxMoreDataFlg;
}FramePbuff_t;

typedef struct 
{
    uint8_t                         pbufWriteIndex;
    uint8_t                         pbufReadIndex;
    FramePbuff_t                    rxFrameBuff[P_BUFFER_SIZE];
}FramePbufferInfo;



typedef struct
{
	uint8_t							Established;
	uint8_t                 		ConnectingCnt;
	TcpIp_CurStatusDef				CurrentStat;
	TcpIp_SocketIdType 				SocketId;
	TcpIp_PortType		   			LocalPort;
	TcpIp_SockAddrInetType  		RemoteSockAddr;
	TcpIp_SwitchPortDef				SwitchPort;
}Socket_InfoTypeDef;


typedef struct
{
	TcpIp_CurStatusDef				CurrentStat;
	TcpIp_SocketIdType   			ConnectedSocketId;
	TcpIp_SockAddrInetType  		RemoteSockAddress;
}Connected_SocketInfoDef;


typedef struct
{
	TcpIp_IpAddrAssignmentStatusDef IpAssignmentStat;
	TcpIp_CurStatusDef              TcpServerInitStat;
	uint8							ConnectedSocketSize;
	TcpIp_SocketIdType      		TcpListenSocketIdx;
	Connected_SocketInfoDef 		ConnectedSocketInfo[TCPIP_TCPSOCKETMAX];
}Eth_SvrCommInfoDef;


typedef struct
{
	TcpIp_IpAddrAssignmentStatusDef IpAssignmentStat;
	uint16_t						SocketPollTimeCnt;
	Socket_InfoTypeDef				SocketInfo[TCPIP_TCPSOCKETMAX];
}Eth_CommInfoDef;


extern zvt_pt_ipmode IpModeFlag;
extern zvt_pt_ipmode PrevIpModeFlag;

extern TcpIp_SockAddrInetType Factory_LocalAddress;
extern TcpIp_SockAddrInetType Factory_RouterAddress;
extern TcpIp_SockAddrInetType User_LocalAddress;
extern TcpIp_SockAddrInetType User_RouterAddress;

extern TcpIp_SockAddrInetType User_RemoteAddress;
extern TcpIp_SockAddrInetType Factory_RemoteAddress;

extern TcpIp_SockAddrInetType CCV_POS_RemoteAddress;			/* CCV_POS:   192.168.0.71:20007 */
extern TcpIp_SockAddrInetType LEFT_LEM_RemoteAddress;			/* Left_LEM:  192.168.0.20:50007 */
extern TcpIp_SockAddrInetType RIGHT_LEM_RemoteAddess;			/* Right_LEM: 192.168.0.40:50008 */
extern TcpIp_SockAddrInetType Tbox1_RemoteAddress;				/* Tbox1:     192.168.0.50:1002  */
extern TcpIp_SockAddrInetType AdBoard_RemoteAddress;			/* AdBoard:   192.168.0.60:2002  */
extern TcpIp_SockAddrInetType Maintain_RemoteAddress;			/* Maintain:  192.168.0.70:3002  */

extern Eth_CommInfoDef        EthernetCommInfo;
extern TcpIp_PortType 		  TcpClientLocalPort_array[TCPIP_TCPSOCKETMAX];

extern FramePbufferInfo       CCV_RxFramePbuffer;
extern FramePbufferInfo       LEFT_LEM_RxFramePbuffer;
extern FramePbufferInfo       RIGHT_LEM_RxFramePbuffer;
extern FramePbufferInfo       Maintain_RxFramePbuffer;
extern FramePbufferInfo       Tbox1_RxFramePbuffer;

extern volatile uint32_t 	  EthComm_1MS_Counter;



uint16_t	   Ethernet_GetRandom(uint32_t countVal);
void 		   EthIf_Initial(void);
void 		   Ethernet_TrcvState_MainFunction(void);
void 		   Ethernet_IpAddrAssignment_MainFunction(void);
void           Ethernet_WaitRxFrameCompleteMainFunction(void);

int 		   Socket_WriteRxFrameBuffer(TcpIp_SocketIdType sockIdx, TcpIp_SockAddrType* pRemoteAddrPtr, uint8* pRxBuf, uint16_t rxLength);

int            Ethernet_ReceiveData_MainFunction(void);

int            Ethernet_CCV_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength);
int            Ethernet_LeftLEM_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength);
int            Ethernet_RightLEM_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength);
int            Ethernet_Maintain_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength);
int            Ethernet_Tbox1_GetRxData(uint8_t* pRxDataBuf, uint16_t* pDataLength);

int            Ethernet_TcpClient_Send(TcpIp_PortType uLocalPort, TcpIp_SockAddrInetType *pRemoteAddrPtr, uint8_t* pTxDatabuf, uint16_t txLength);
int            Ethernet_TcpClient_Transmit(TcpIp_PortType u16localPort, TcpIp_SockAddrInetType *remoteAddrPtr, uint8_t *txDatabuffer, uint16_t txDataSize);

void 		   Ethernet_TcpSocket_MainFunction(void);


#ifdef __cplusplus
}
#endif
#endif /* USERS_ETHCOMM_H_ */
