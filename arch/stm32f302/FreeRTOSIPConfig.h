#pragma once

#define FREERTOS_IP_CONFIG_H

#define ipconfigNETWORK_MTU     1500


#define ipconfigBYTE_ORDER    pdFREERTOS_LITTLE_ENDIAN
#define ipconfigEVENT_QUEUE_LENGTH (ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5)

#define ipconfigIP_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define ipconfigIP_TASK_STACK_SIZE_WORDS (configMINIMAL_STACK_SIZE * 5)

#define ipconfigUSE_TCP 0
#define ipconfigTCP_MSS         1460
#define ipconfigTCP_TX_BUFFER_LENGTH  ( 16 * ipconfigTCP_MSS )
#define ipconfigTCP_RX_BUFFER_LENGTH  ( 16 * ipconfigTCP_MSS )

#define ipconfigDHCP_REGISTER_HOSTNAME 1
#define ipconfigUSE_DHCP 1
#define ipconfigUSE_NETWORK_EVENT_HOOK 1

