// Copyright GHI Electronics, LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <string.h>
#include "STM32F4.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef INCLUDE_CAN

#define CAN_TRANSFER_TIMEOUT 0xFFFF

#define CAN_Mode_Normal             ((uint8_t)0x00)  /*!< normal mode */
#define CAN_Mode_LoopBack           ((uint8_t)0x01)  /*!< loopback mode */
#define CAN_Mode_Silent             ((uint8_t)0x02)  /*!< silent mode */
#define CAN_Mode_Silent_LoopBack    ((uint8_t)0x03)  /*!< loopback combined with silent mode */


/** @defgroup CAN_InitStatus
* @{
*/

#define CAN_InitStatus_Failed              ((uint8_t)0x00) /*!< CAN initialization failed */
#define CAN_InitStatus_Success             ((uint8_t)0x01) /*!< CAN initialization OK */

/** @defgroup CAN_filter_mode
* @{
*/
#define CAN_FilterMode_IdMask       ((uint8_t)0x00)  /*!< identifier/mask mode */
#define CAN_FilterMode_IdList       ((uint8_t)0x01)  /*!< identifier list mode */

/** @defgroup CAN_filter_scale
* @{
*/
#define CAN_FilterScale_16bit       ((uint8_t)0x00) /*!< Two 16-bit filters */
#define CAN_FilterScale_32bit       ((uint8_t)0x01) /*!< One 32-bit filter */

/** @defgroup CAN_filter_FIFO
* @{
*/
#define CAN_Filter_FIFO0             ((uint8_t)0x00)  /*!< Filter FIFO 0 assignment for filter x */
#define CAN_Filter_FIFO1             ((uint8_t)0x01)  /*!< Filter FIFO 1 assignment for filter x *

/** @defgroup CAN_transmit_constants
* @{
*/
#define CAN_TxStatus_Failed         ((uint8_t)0x00)/*!< CAN transmission failed */
#define CAN_TxStatus_Ok             ((uint8_t)0x01) /*!< CAN transmission succeeded */
#define CAN_TxStatus_Pending        ((uint8_t)0x02) /*!< CAN transmission pending */
#define CAN_TxStatus_NoMailBox      ((uint8_t)0x04) /*!< CAN cell did not provide
an empty mailbox */

/** @defgroup CAN_identifier_type
* @{
*/
#define CAN_Id_Standard             ((uint32_t)0x00000000)  /*!< Standard Id */
#define CAN_Id_Extended             ((uint32_t)0x00000004)  /*!< Extended Id */


/** @defgroup CAN_receive_FIFO_number_constants
  * @{
  */
#define CAN_FIFO0                 ((uint8_t)0x00) /*!< CAN FIFO 0 used to receive */
#define CAN_FIFO1                 ((uint8_t)0x01) /*!< CAN FIFO 1 used to receive */

  /** @defgroup CAN_interrupts
    * @{
    */
#define CAN_IT_TME                  ((uint32_t)0x00000001) /*!< Transmit mailbox empty Interrupt*/

    /* Receive Interrupts */
#define CAN_IT_FMP0                 ((uint32_t)0x00000002) /*!< FIFO 0 message pending Interrupt*/
#define CAN_IT_FF0                  ((uint32_t)0x00000004) /*!< FIFO 0 full Interrupt*/
#define CAN_IT_FOV0                 ((uint32_t)0x00000008) /*!< FIFO 0 overrun Interrupt*/
#define CAN_IT_FMP1                 ((uint32_t)0x00000010) /*!< FIFO 1 message pending Interrupt*/
#define CAN_IT_FF1                  ((uint32_t)0x00000020) /*!< FIFO 1 full Interrupt*/
#define CAN_IT_FOV1                 ((uint32_t)0x00000040) /*!< FIFO 1 overrun Interrupt*/

/* Operating Mode Interrupts */
#define CAN_IT_WKU                  ((uint32_t)0x00010000) /*!< Wake-up Interrupt*/
#define CAN_IT_SLK                  ((uint32_t)0x00020000) /*!< Sleep acknowledge Interrupt*/

/* Error Interrupts */
#define CAN_IT_EWG                  ((uint32_t)0x00000100) /*!< Error warning Interrupt*/
#define CAN_IT_EPV                  ((uint32_t)0x00000200) /*!< Error passive Interrupt*/
#define CAN_IT_BOF                  ((uint32_t)0x00000400) /*!< Bus-off Interrupt*/
#define CAN_IT_LEC                  ((uint32_t)0x00000800) /*!< Last error code Interrupt*/
#define CAN_IT_ERR                  ((uint32_t)0x00008000) /*!< Error Interrupt*/

/* Flags named as Interrupts : kept only for FW compatibility */
#define CAN_IT_RQCP0   CAN_IT_TME
#define CAN_IT_RQCP1   CAN_IT_TME
#define CAN_IT_RQCP2   CAN_IT_TME

/* CAN Master Control Register bits */
#define MCR_DBF           ((uint32_t)0x00010000) /* software master reset */

/* CAN Mailbox Transmit Request */
#define TMIDxR_TXRQ       ((uint32_t)0x00000001) /* Transmit mailbox request */

/* CAN Filter Master Register bits */
#define FMR_FINIT         ((uint32_t)0x00000001) /* Filter init mode */

/* Time out for INAK bit */
#define INAK_TIMEOUT      ((uint32_t)0x0000FFFF)
/* Time out for SLAK bit */
#define SLAK_TIMEOUT      ((uint32_t)0x0000FFFF)

/* Flags in TSR register */
#define CAN_FLAGS_TSR     ((uint32_t)0x08000000)
/* Flags in RF1R register */
#define CAN_FLAGS_RF1R    ((uint32_t)0x04000000)
/* Flags in RF0R register */
#define CAN_FLAGS_RF0R    ((uint32_t)0x02000000)
/* Flags in MSR register */
#define CAN_FLAGS_MSR     ((uint32_t)0x01000000)
/* Flags in ESR register */
#define CAN_FLAGS_ESR     ((uint32_t)0x00F00000)

/* Mailboxes definition */
#define CAN_TXMAILBOX_0   ((uint8_t)0x00)
#define CAN_TXMAILBOX_1   ((uint8_t)0x01)
#define CAN_TXMAILBOX_2   ((uint8_t)0x02)

#define CAN_MODE_MASK     ((uint32_t) 0x00000003)

/**
  * @brief  CAN init structure definition
  */
typedef struct {
    uint16_t CAN_Prescaler;   /*!< Specifies the length of a time quantum.
                                   It ranges from 1 to 1024. */

    uint8_t CAN_Mode;         /*!< Specifies the CAN operating mode.
                                   This parameter can be a value of @ref CAN_operating_mode */

    uint8_t CAN_SJW;          /*!< Specifies the maximum number of time quanta
                                   the CAN hardware is allowed to lengthen or
                                   shorten a bit to perform resynchronization.
                                   This parameter can be a value of @ref CAN_synchronisation_jump_width */

    uint8_t CAN_BS1;          /*!< Specifies the number of time quanta in Bit
                                   Segment 1. This parameter can be a value of
                                   @ref CAN_time_quantum_in_bit_segment_1 */

    uint8_t CAN_BS2;          /*!< Specifies the number of time quanta in Bit Segment 2.
                                   This parameter can be a value of @ref CAN_time_quantum_in_bit_segment_2 */

    FunctionalState CAN_TTCM; /*!< Enable or disable the time triggered communication mode.
                                  This parameter can be set either to ENABLE or DISABLE. */

    FunctionalState CAN_ABOM;  /*!< Enable or disable the automatic bus-off management.
                                    This parameter can be set either to ENABLE or DISABLE. */

    FunctionalState CAN_AWUM;  /*!< Enable or disable the automatic wake-up mode.
                                    This parameter can be set either to ENABLE or DISABLE. */

    FunctionalState CAN_NART;  /*!< Enable or disable the non-automatic retransmission mode.
                                    This parameter can be set either to ENABLE or DISABLE. */

    FunctionalState CAN_RFLM;  /*!< Enable or disable the Receive FIFO Locked mode.
                                    This parameter can be set either to ENABLE or DISABLE. */

    FunctionalState CAN_TXFP;  /*!< Enable or disable the transmit FIFO priority.
                                    This parameter can be set either to ENABLE or DISABLE. */
} STM32F4_Can_InitTypeDef;

/**
  * @brief  CAN filter init structure definition
  */
typedef struct {
    uint16_t CAN_FilterIdHigh;         /*!< Specifies the filter identification number (MSBs for a 32-bit
                                                configuration, first one for a 16-bit configuration).
                                                This parameter can be a value between 0x0000 and 0xFFFF */

    uint16_t CAN_FilterIdLow;          /*!< Specifies the filter identification number (LSBs for a 32-bit
                                                configuration, second one for a 16-bit configuration).
                                                This parameter can be a value between 0x0000 and 0xFFFF */

    uint16_t CAN_FilterMaskIdHigh;     /*!< Specifies the filter mask number or identification number,
                                                according to the mode (MSBs for a 32-bit configuration,
                                                first one for a 16-bit configuration).
                                                This parameter can be a value between 0x0000 and 0xFFFF */

    uint16_t CAN_FilterMaskIdLow;      /*!< Specifies the filter mask number or identification number,
                                                according to the mode (LSBs for a 32-bit configuration,
                                                second one for a 16-bit configuration).
                                                This parameter can be a value between 0x0000 and 0xFFFF */

    uint16_t CAN_FilterFIFOAssignment; /*!< Specifies the FIFO (0 or 1) which will be assigned to the filter.
                                                This parameter can be a value of @ref CAN_filter_FIFO */

    uint8_t CAN_FilterNumber;          /*!< Specifies the filter which will be initialized. It ranges from 0 to 13. */

    uint8_t CAN_FilterMode;            /*!< Specifies the filter mode to be initialized.
                                                This parameter can be a value of @ref CAN_filter_mode */

    uint8_t CAN_FilterScale;           /*!< Specifies the filter scale.
                                                This parameter can be a value of @ref CAN_filter_scale */

    FunctionalState CAN_FilterActivation; /*!< Enable or disable the filter.
                                                This parameter can be set either to ENABLE or DISABLE. */
} STM32F4_Can_FilterInitTypeDef;

/**
  * @brief  CAN Tx message structure definition
  */
typedef struct {
    uint32_t StdId;  /*!< Specifies the standard identifier.
                          This parameter can be a value between 0 to 0x7FF. */

    uint32_t ExtId;  /*!< Specifies the extended identifier.
                          This parameter can be a value between 0 to 0x1FFFFFFF. */

    uint8_t IDE;     /*!< Specifies the type of identifier for the message that
                          will be transmitted. This parameter can be a value
                          of @ref CAN_identifier_type */

    uint8_t RTR;     /*!< Specifies the type of frame for the message that will
                          be transmitted. This parameter can be a value of
                          @ref CAN_remote_transmission_request */

    uint8_t DLC;     /*!< Specifies the length of the frame that will be
                          transmitted. This parameter can be a value between
                          0 to 8 */

    uint8_t Data[8]; /*!< Contains the data to be transmitted. It ranges from 0
                          to 0xFF. */
} STM32F4_Can_TxMessage;

/**
  * @brief  CAN Rx message structure definition
  */
typedef struct {
    uint32_t StdId;  /*!< Specifies the standard identifier.
                          This parameter can be a value between 0 to 0x7FF. */

    uint32_t ExtId;  /*!< Specifies the extended identifier.
                          This parameter can be a value between 0 to 0x1FFFFFFF. */

    uint8_t IDE;     /*!< Specifies the type of identifier for the message that
                          will be received. This parameter can be a value of
                          @ref CAN_identifier_type */

    uint8_t RTR;     /*!< Specifies the type of frame for the received message.
                          This parameter can be a value of
                          @ref CAN_remote_transmission_request */

    uint8_t DLC;     /*!< Specifies the length of the frame that will be received.
                          This parameter can be a value between 0 to 8 */

    uint8_t Data[8]; /*!< Contains the data to be received. It ranges from 0 to
                          0xFF. */

    uint8_t FMI;     /*!< Specifies the index of the filter the message stored in
                          the mailbox passes through. This parameter can be a
                          value between 0 to 0xFF */
} STM32F4_Can_RxMessage;


struct STM32F4_Can_Filter {
    uint32_t* matchFilters;
    uint32_t matchFiltersSize;

    uint32_t* lowerBoundFilters;
    uint32_t* upperBoundFilters;
    uint32_t groupFiltersSize;

};


typedef struct {
    uint32_t TimeStampL;
    uint32_t TimeStampH;

    uint32_t MsgID;	// CAN Message ID (11-bit or 29-bit)

    bool extendedId;
    bool remoteTransmissionRequest;

    uint32_t DataA;	// CAN Message Data Bytes 0-3
    uint32_t DataB;	// CAN Message Data Bytes 4-7

    int32_t length;

} STM32F4_Can_Message;

struct STM32F4_Can_Controller {
    const TinyCLR_Can_Provider* provider;

    STM32F4_Can_Message *canRxMessagesFifo;

    STM32F4_Can_InitTypeDef initTypeDef;
    STM32F4_Can_FilterInitTypeDef filterInitTypeDef;

    TinyCLR_Can_ErrorReceivedHandler   errorEventHandler;
    TinyCLR_Can_MessageReceivedHandler    messageReceivedEventHandler;

    int32_t can_rx_count;
    int32_t can_rx_in;
    int32_t can_rx_out;

    size_t can_rxBufferSize;
    size_t can_txBufferSize;

    uint32_t baudrate;

    STM32F4_Can_Filter canDataFilter;

    bool isOpened;
};

static const STM32F4_Gpio_Pin g_STM32F4_Can_Tx_Pins[] = STM32F4_CAN_TX_PINS;
static const STM32F4_Gpio_Pin g_STM32F4_Can_Rx_Pins[] = STM32F4_CAN_RX_PINS;
static const uint32_t g_STM32F4_Can_defaultBuffersSize[] = STM32F4_CAN_BUFFER_DEFAULT_SIZE;

static const int TOTAL_CAN_CONTROLLERS = SIZEOF_ARRAY(g_STM32F4_Can_Tx_Pins);

static STM32F4_Can_Controller canController[TOTAL_CAN_CONTROLLERS];

static TinyCLR_Can_Provider canProvider;
static TinyCLR_Api_Info canApi;

bool InsertionSort2CheckOverlap(uint32_t* lowerBounds, uint32_t* upperBounds, int32_t length) {

    uint32_t i, j, tmp, tmp2;

    for (i = 1; i < length; i++) {
        j = i;

        while (j > 0 && lowerBounds[j - 1] > lowerBounds[j]) {
            tmp = lowerBounds[j]; tmp2 = upperBounds[j];
            lowerBounds[j] = lowerBounds[j - 1];	upperBounds[j] = upperBounds[j - 1];
            lowerBounds[j - 1] = tmp;	upperBounds[j - 1] = tmp2;

            j--;
        }
    }

    // check for overlap
    if (lowerBounds[0] > upperBounds[0])
        return false;

    for (i = 1; i < length; i++) {
        if (lowerBounds[i] > upperBounds[i])
            return false;

        if (lowerBounds[i] <= upperBounds[i - 1])
            return false;
    }

    return true;
}

int32_t BinarySearch(uint32_t* sortedArray, int32_t first, int32_t last, uint32_t key) {
    int32_t mid;
    while (first <= last) {
        mid = (first + last) / 2;  // compute mid point.
        if (key > sortedArray[mid])
            first = mid + 1;  // repeat search in top half.
        else if (key < sortedArray[mid])
            last = mid - 1; // repeat search in bottom half.
        else
            return mid;     // found it. return position /////
    }

    return -1;    // failed to find key
}

int32_t BinarySearch2(uint32_t* lowerBounds, uint32_t* upperBounds, int32_t first, int32_t last, uint32_t key) {

    int32_t mid;

    while (first <= last) {
        mid = (first + last) / 2;  // compute mid point.

        if (key > upperBounds[mid])
            first = mid + 1;  // repeat search in top half.
        else if (key < lowerBounds[mid])
            last = mid - 1; // repeat search in bottom half.
        else
            return mid;     // found it. return position /////
    }

    return -1;    // failed to find key
}

/**
  * @brief  Initializes the CAN peripheral according to the specified
  *         parameters in the CAN_InitStruct.
  * @param  CANx: where x can be 1 or 2 to select the CAN peripheral.
  * @param  CAN_InitStruct: pointer to a STM32F4_Can_InitTypeDef structure that contains
  *         the configuration information for the CAN peripheral.
  * @retval Constant indicates initialization succeed which will be
  *         CAN_InitStatus_Failed or CAN_InitStatus_Success.
  */
uint8_t CAN_Initialize(CAN_TypeDef* CANx, STM32F4_Can_InitTypeDef* CAN_InitStruct) {
    uint8_t InitStatus = CAN_InitStatus_Failed;
    uint32_t wait_ack = 0x00000000;

    /* Exit from sleep mode */
    CANx->MCR &= (~(uint32_t)CAN_MCR_SLEEP);

    /* Request initialisation */
    CANx->MCR |= CAN_MCR_INRQ;

    /* Wait the acknowledge */
    while (((CANx->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) && (wait_ack != INAK_TIMEOUT)) {
        wait_ack++;
    }

    /* Check acknowledge */
    if ((CANx->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) {
        InitStatus = CAN_InitStatus_Failed;
    }
    else {
        /* Set the time triggered communication mode */
        if (CAN_InitStruct->CAN_TTCM == ENABLE) {
            CANx->MCR |= CAN_MCR_TTCM;
        }
        else {
            CANx->MCR &= ~(uint32_t)CAN_MCR_TTCM;
        }

        /* Set the automatic bus-off management */
        if (CAN_InitStruct->CAN_ABOM == ENABLE) {
            CANx->MCR |= CAN_MCR_ABOM;
        }
        else {
            CANx->MCR &= ~(uint32_t)CAN_MCR_ABOM;
        }

        /* Set the automatic wake-up mode */
        if (CAN_InitStruct->CAN_AWUM == ENABLE) {
            CANx->MCR |= CAN_MCR_AWUM;
        }
        else {
            CANx->MCR &= ~(uint32_t)CAN_MCR_AWUM;
        }

        /* Set the no automatic retransmission */
        if (CAN_InitStruct->CAN_NART == ENABLE) {
            CANx->MCR |= CAN_MCR_NART;
        }
        else {
            CANx->MCR &= ~(uint32_t)CAN_MCR_NART;
        }

        /* Set the receive FIFO locked mode */
        if (CAN_InitStruct->CAN_RFLM == ENABLE) {
            CANx->MCR |= CAN_MCR_RFLM;
        }
        else {
            CANx->MCR &= ~(uint32_t)CAN_MCR_RFLM;
        }

        /* Set the transmit FIFO priority */
        if (CAN_InitStruct->CAN_TXFP == ENABLE) {
            CANx->MCR |= CAN_MCR_TXFP;
        }
        else {
            CANx->MCR &= ~(uint32_t)CAN_MCR_TXFP;
        }

        /* Set the bit timing register */
        CANx->BTR = (uint32_t)((uint32_t)CAN_InitStruct->CAN_Mode << 30) | \
            ((uint32_t)CAN_InitStruct->CAN_SJW << 24) | \
            ((uint32_t)CAN_InitStruct->CAN_BS1 << 16) | \
            ((uint32_t)CAN_InitStruct->CAN_BS2 << 20) | \
            ((uint32_t)CAN_InitStruct->CAN_Prescaler - 1);

        /* Request leave initialisation */
        CANx->MCR &= ~(uint32_t)CAN_MCR_INRQ;

        /* Wait the acknowledge */
        wait_ack = 0;

        while (((CANx->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) && (wait_ack != INAK_TIMEOUT)) {
            wait_ack++;
        }

        /* ...and check acknowledged */
        if ((CANx->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) {
            InitStatus = CAN_InitStatus_Failed;
        }
        else {
            InitStatus = CAN_InitStatus_Success;
        }
    }

    /* At this step, return the status of initialization */
    return InitStatus;
}

/**
  * @brief  Configures the CAN reception filter according to the specified
  *         parameters in the CAN_FilterInitStruct.
  * @param  CAN_FilterInitStruct: pointer to a STM32F4_Can_FilterInitTypeDef structure that
  *         contains the configuration information.
  * @retval None
  */
void CAN_FilterInit(STM32F4_Can_FilterInitTypeDef* CAN_FilterInitStruct) {
    uint32_t filter_number_bit_pos = 0;
    /* Check the parameters */

    filter_number_bit_pos = ((uint32_t)1) << CAN_FilterInitStruct->CAN_FilterNumber;

    /* Initialisation mode for the filter */
    CAN1->FMR |= FMR_FINIT;

    /* Filter Deactivation */
    CAN1->FA1R &= ~(uint32_t)filter_number_bit_pos;

    /* Filter Scale */
    if (CAN_FilterInitStruct->CAN_FilterScale == CAN_FilterScale_16bit) {
        /* 16-bit scale for the filter */
        CAN1->FS1R &= ~(uint32_t)filter_number_bit_pos;

        /* First 16-bit identifier and First 16-bit mask */
        /* Or First 16-bit identifier and Second 16-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR1 =
            ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdLow) << 16) |
            (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdLow);

        /* Second 16-bit identifier and Second 16-bit mask */
        /* Or Third 16-bit identifier and Fourth 16-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR2 =
            ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdHigh) << 16) |
            (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdHigh);
    }

    if (CAN_FilterInitStruct->CAN_FilterScale == CAN_FilterScale_32bit) {
        /* 32-bit scale for the filter */
        CAN1->FS1R |= filter_number_bit_pos;
        /* 32-bit identifier or First 32-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR1 =
            ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdHigh) << 16) |
            (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdLow);
        /* 32-bit mask or Second 32-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR2 =
            ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdHigh) << 16) |
            (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdLow);
    }

    /* Filter Mode */
    if (CAN_FilterInitStruct->CAN_FilterMode == CAN_FilterMode_IdMask) {
        /*Id/Mask mode for the filter*/
        CAN1->FM1R &= ~(uint32_t)filter_number_bit_pos;
    }
    else /* CAN_FilterInitStruct->CAN_FilterMode == CAN_FilterMode_IdList */
    {
        /*Identifier list mode for the filter*/
        CAN1->FM1R |= (uint32_t)filter_number_bit_pos;
    }

    /* Filter FIFO assignment */
    if (CAN_FilterInitStruct->CAN_FilterFIFOAssignment == CAN_Filter_FIFO0) {
        /* FIFO 0 assignation for the filter */
        CAN1->FFA1R &= ~(uint32_t)filter_number_bit_pos;
    }

    if (CAN_FilterInitStruct->CAN_FilterFIFOAssignment == CAN_Filter_FIFO1) {
        /* FIFO 1 assignation for the filter */
        CAN1->FFA1R |= (uint32_t)filter_number_bit_pos;
    }

    /* Filter activation */
    if (CAN_FilterInitStruct->CAN_FilterActivation == ENABLE) {
        CAN1->FA1R |= filter_number_bit_pos;
    }

    /* Leave the initialisation mode for the filter */
    CAN1->FMR &= ~FMR_FINIT;
}

/**
  * @brief  Receives a correct CAN frame.
  * @param  CANx: where x can be 1 or 2 to select the CAN peripheral.
  * @param  FIFONumber: Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
  * @param  RxMessage: pointer to a structure receive frame which contains CAN Id,
  *         CAN DLC, CAN data and FMI number.
  * @retval None
  */
void CAN_Receive(CAN_TypeDef* CANx, uint8_t FIFONumber, STM32F4_Can_RxMessage* RxMessage) {
    /* Check the parameters */
    // TQD assert_param(IS_CAN_ALL_PERIPH(CANx));
    // TQD assert_param(IS_CAN_FIFO(FIFONumber));
    /* Get the Id */
    RxMessage->IDE = (uint8_t)0x04 & CANx->sFIFOMailBox[FIFONumber].RIR;
    if (RxMessage->IDE == CAN_Id_Standard) {
        RxMessage->StdId = (uint32_t)0x000007FF & (CANx->sFIFOMailBox[FIFONumber].RIR >> 21);
    }
    else {
        RxMessage->ExtId = (uint32_t)0x1FFFFFFF & (CANx->sFIFOMailBox[FIFONumber].RIR >> 3);
    }

    RxMessage->RTR = (uint8_t)0x02 & CANx->sFIFOMailBox[FIFONumber].RIR;
    /* Get the DLC */
    RxMessage->DLC = (uint8_t)0x0F & CANx->sFIFOMailBox[FIFONumber].RDTR;
    /* Get the FMI */
    RxMessage->FMI = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDTR >> 8);
    /* Get the data field */
    RxMessage->Data[0] = (uint8_t)0xFF & CANx->sFIFOMailBox[FIFONumber].RDLR;
    RxMessage->Data[1] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 8);
    RxMessage->Data[2] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 16);
    RxMessage->Data[3] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 24);
    RxMessage->Data[4] = (uint8_t)0xFF & CANx->sFIFOMailBox[FIFONumber].RDHR;
    RxMessage->Data[5] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 8);
    RxMessage->Data[6] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 16);
    RxMessage->Data[7] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 24);
    /* Release the FIFO */
    /* Release FIFO0 */
    if (FIFONumber == CAN_FIFO0) {
        CANx->RF0R |= CAN_RF0R_RFOM0;
    }
    /* Release FIFO1 */
    else /* FIFONumber == CAN_FIFO1 */
    {
        CANx->RF1R |= CAN_RF1R_RFOM1;
    }
}

/**
  * @brief  Initiates and transmits a CAN frame message.
  * @param  CANx: where x can be 1 or 2 to to select the CAN peripheral.
  * @param  TxMessage: pointer to a structure which contains CAN Id, CAN DLC and CAN data.
  * @retval The number of the mailbox that is used for transmission or
  *         CAN_TxStatus_NoMailBox if there is no empty mailbox.
  */
uint8_t CAN_Transmit(CAN_TypeDef* CANx, STM32F4_Can_TxMessage* TxMessage) {
    uint8_t transmit_mailbox = 0;

    /* Select one empty transmit mailbox */
    if ((CANx->TSR&CAN_TSR_TME0) == CAN_TSR_TME0) {
        transmit_mailbox = 0;
    }
    else if ((CANx->TSR&CAN_TSR_TME1) == CAN_TSR_TME1) {
        transmit_mailbox = 1;
    }
    else if ((CANx->TSR&CAN_TSR_TME2) == CAN_TSR_TME2) {
        transmit_mailbox = 2;
    }
    else {
        transmit_mailbox = CAN_TxStatus_NoMailBox;
    }

    if (transmit_mailbox != CAN_TxStatus_NoMailBox) {
        /* Set up the Id */
        CANx->sTxMailBox[transmit_mailbox].TIR &= TMIDxR_TXRQ;
        if (TxMessage->IDE == CAN_Id_Standard) {
            // TQD assert_param(IS_CAN_STDID(TxMessage->StdId));
            CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->StdId << 21) | \
                TxMessage->RTR);
        }
        else {
            // TQD assert_param(IS_CAN_EXTID(TxMessage->ExtId));
            CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->ExtId << 3) | \
                TxMessage->IDE | \
                TxMessage->RTR);
        }

        /* Set up the DLC */
        TxMessage->DLC &= (uint8_t)0x0000000F;
        CANx->sTxMailBox[transmit_mailbox].TDTR &= (uint32_t)0xFFFFFFF0;
        CANx->sTxMailBox[transmit_mailbox].TDTR |= TxMessage->DLC;

        /* Set up the data field */
        CANx->sTxMailBox[transmit_mailbox].TDLR = (((uint32_t)TxMessage->Data[3] << 24) |
            ((uint32_t)TxMessage->Data[2] << 16) |
            ((uint32_t)TxMessage->Data[1] << 8) |
            ((uint32_t)TxMessage->Data[0]));
        CANx->sTxMailBox[transmit_mailbox].TDHR = (((uint32_t)TxMessage->Data[7] << 24) |
            ((uint32_t)TxMessage->Data[6] << 16) |
            ((uint32_t)TxMessage->Data[5] << 8) |
            ((uint32_t)TxMessage->Data[4]));
        /* Request transmission */
        CANx->sTxMailBox[transmit_mailbox].TIR |= TMIDxR_TXRQ;
    }

    return transmit_mailbox;
}

/**
  * @brief  Checks the transmission status of a CAN Frame.
  * @param  CANx: where x can be 1 or 2 to select the CAN peripheral.
  * @param  TransmitMailbox: the number of the mailbox that is used for transmission.
  * @retval CAN_TxStatus_Ok if the CAN driver transmits the message,
  *         CAN_TxStatus_Failed in an other case.
  */
uint8_t CAN_TransmitStatus(CAN_TypeDef* CANx, uint8_t TransmitMailbox) {
    uint32_t state = 0;

    /* Check the parameters */


    switch (TransmitMailbox) {
    case (CAN_TXMAILBOX_0):
        state = CANx->TSR &  (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0);
        break;
    case (CAN_TXMAILBOX_1):
        state = CANx->TSR &  (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1);
        break;
    case (CAN_TXMAILBOX_2):
        state = CANx->TSR &  (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2);
        break;
    default:
        state = CAN_TxStatus_Failed;
        break;
    }
    switch (state) {
        /* transmit pending  */
    case (0x0): state = CAN_TxStatus_Pending;
        break;
        /* transmit failed  */
    case (CAN_TSR_RQCP0 | CAN_TSR_TME0): state = CAN_TxStatus_Failed;
        break;
    case (CAN_TSR_RQCP1 | CAN_TSR_TME1): state = CAN_TxStatus_Failed;
        break;
    case (CAN_TSR_RQCP2 | CAN_TSR_TME2): state = CAN_TxStatus_Failed;
        break;
        /* transmit succeeded  */
    case (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0):state = CAN_TxStatus_Ok;
        break;
    case (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1):state = CAN_TxStatus_Ok;
        break;
    case (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2):state = CAN_TxStatus_Ok;
        break;
    default: state = CAN_TxStatus_Failed;
        break;
    }
    return (uint8_t)state;
}

uint32_t CAN_GetTransferTimeout() {
    return CAN_TRANSFER_TIMEOUT;
}

/**
  * @brief  Cancels a transmit request.
  * @param  CANx: where x can be 1 or 2 to select the CAN peripheral.
  * @param  Mailbox: Mailbox number.
  * @retval None
  */
void CAN_CancelTransmit(CAN_TypeDef* CANx, uint8_t Mailbox) {
    /* Check the parameters */
    // TQD assert_param(IS_CAN_ALL_PERIPH(CANx));
    // TQD assert_param(IS_CAN_TRANSMITMAILBOX(Mailbox));
    /* abort transmission */
    switch (Mailbox) {
    case (CAN_TXMAILBOX_0): CANx->TSR |= CAN_TSR_ABRQ0;
        break;
    case (CAN_TXMAILBOX_1): CANx->TSR |= CAN_TSR_ABRQ1;
        break;
    case (CAN_TXMAILBOX_2): CANx->TSR |= CAN_TSR_ABRQ2;
        break;
    default:
        break;
    }
}
/**
  * @brief  Clears the CANx's interrupt pending bits.
  * @param  CANx: where x can be 1 or 2 to to select the CAN peripheral.
  * @param  CAN_IT: specifies the interrupt pending bit to clear.
  *          This parameter can be one of the following values:
  *            @arg CAN_IT_TME: Transmit mailbox empty Interrupt
  *            @arg CAN_IT_FF0: FIFO 0 full Interrupt
  *            @arg CAN_IT_FOV0: FIFO 0 overrun Interrupt
  *            @arg CAN_IT_FF1: FIFO 1 full Interrupt
  *            @arg CAN_IT_FOV1: FIFO 1 overrun Interrupt
  *            @arg CAN_IT_WKU: Wake-up Interrupt
  *            @arg CAN_IT_SLK: Sleep acknowledge Interrupt
  *            @arg CAN_IT_EWG: Error warning Interrupt
  *            @arg CAN_IT_EPV: Error passive Interrupt
  *            @arg CAN_IT_BOF: Bus-off Interrupt
  *            @arg CAN_IT_LEC: Last error code Interrupt
  *            @arg CAN_IT_ERR: Error Interrupt
  * @retval None
  */
void CAN_ClearITPendingBit(CAN_TypeDef* CANx, uint32_t CAN_IT) {
    /* Check the parameters */
    // TQD assert_param(IS_CAN_ALL_PERIPH(CANx));
    // TQD assert_param(IS_CAN_CLEAR_IT(CAN_IT));

    switch (CAN_IT) {
    case CAN_IT_TME:
        /* Clear CAN_TSR_RQCPx (rc_w1)*/
        CANx->TSR = CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2;
        break;
    case CAN_IT_FF0:
        /* Clear CAN_RF0R_FULL0 (rc_w1)*/
        CANx->RF0R = CAN_RF0R_FULL0;
        break;
    case CAN_IT_FOV0:
        /* Clear CAN_RF0R_FOVR0 (rc_w1)*/
        CANx->RF0R = CAN_RF0R_FOVR0;
        break;
    case CAN_IT_FF1:
        /* Clear CAN_RF1R_FULL1 (rc_w1)*/
        CANx->RF1R = CAN_RF1R_FULL1;
        break;
    case CAN_IT_FOV1:
        /* Clear CAN_RF1R_FOVR1 (rc_w1)*/
        CANx->RF1R = CAN_RF1R_FOVR1;
        break;
    case CAN_IT_WKU:
        /* Clear CAN_MSR_WKUI (rc_w1)*/
        CANx->MSR = CAN_MSR_WKUI;
        break;
    case CAN_IT_SLK:
        /* Clear CAN_MSR_SLAKI (rc_w1)*/
        CANx->MSR = CAN_MSR_SLAKI;
        break;
    case CAN_IT_EWG:
        /* Clear CAN_MSR_ERRI (rc_w1) */
        CANx->MSR = CAN_MSR_ERRI;
        /* @note the corresponding Flag is cleared by hardware depending on the CAN Bus status*/
        break;
    case CAN_IT_EPV:
        /* Clear CAN_MSR_ERRI (rc_w1) */
        CANx->MSR = CAN_MSR_ERRI;
        /* @note the corresponding Flag is cleared by hardware depending on the CAN Bus status*/
        break;
    case CAN_IT_BOF:
        /* Clear CAN_MSR_ERRI (rc_w1) */
        CANx->MSR = CAN_MSR_ERRI;
        /* @note the corresponding Flag is cleared by hardware depending on the CAN Bus status*/
        break;
    case CAN_IT_LEC:
        /*  Clear LEC bits */
        CANx->ESR = RESET;
        /* Clear CAN_MSR_ERRI (rc_w1) */
        CANx->MSR = CAN_MSR_ERRI;
        break;
    case CAN_IT_ERR:
        /*Clear LEC bits */
        CANx->ESR = RESET;
        /* Clear CAN_MSR_ERRI (rc_w1) */
        CANx->MSR = CAN_MSR_ERRI;
        /* @note BOFF, EPVF and EWGF Flags are cleared by hardware depending on the CAN Bus status*/
        break;
    default:
        break;
    }
}

/**
  * @brief  Checks whether the CAN interrupt has occurred or not.
  * @param  CAN_Reg: specifies the CAN interrupt register to check.
  * @param  It_Bit: specifies the interrupt source bit to check.
  * @retval The new state of the CAN Interrupt (SET or RESET).
  */
static ITStatus CheckITStatus(uint32_t CAN_Reg, uint32_t It_Bit) {
    ITStatus pendingbitstatus = RESET;

    if ((CAN_Reg & It_Bit) != (uint32_t)RESET) {
        /* CAN_IT is set */
        pendingbitstatus = SET;
    }
    else {
        /* CAN_IT is reset */
        pendingbitstatus = RESET;
    }
    return pendingbitstatus;
}


/**
  * @brief  Checks whether the specified CANx interrupt has occurred or not.
  * @param  CANx: where x can be 1 or 2 to to select the CAN peripheral.
  * @param  CAN_IT: specifies the CAN interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg CAN_IT_TME: Transmit mailbox empty Interrupt
  *            @arg CAN_IT_FMP0: FIFO 0 message pending Interrupt
  *            @arg CAN_IT_FF0: FIFO 0 full Interrupt
  *            @arg CAN_IT_FOV0: FIFO 0 overrun Interrupt
  *            @arg CAN_IT_FMP1: FIFO 1 message pending Interrupt
  *            @arg CAN_IT_FF1: FIFO 1 full Interrupt
  *            @arg CAN_IT_FOV1: FIFO 1 overrun Interrupt
  *            @arg CAN_IT_WKU: Wake-up Interrupt
  *            @arg CAN_IT_SLK: Sleep acknowledge Interrupt
  *            @arg CAN_IT_EWG: Error warning Interrupt
  *            @arg CAN_IT_EPV: Error passive Interrupt
  *            @arg CAN_IT_BOF: Bus-off Interrupt
  *            @arg CAN_IT_LEC: Last error code Interrupt
  *            @arg CAN_IT_ERR: Error Interrupt
  * @retval The current state of CAN_IT (SET or RESET).
  */
ITStatus CAN_GetITStatus(CAN_TypeDef* CANx, uint32_t CAN_IT) {
    ITStatus itstatus = RESET;
    /* Check the parameters */
    // TQD assert_param(IS_CAN_ALL_PERIPH(CANx));
    // TQD assert_param(IS_CAN_IT(CAN_IT));

    /* check the interrupt enable bit */
    if ((CANx->IER & CAN_IT) != RESET) {
        /* in case the Interrupt is enabled, .... */
        switch (CAN_IT) {
        case CAN_IT_TME:
            /* Check CAN_TSR_RQCPx bits */
            itstatus = CheckITStatus(CANx->TSR, CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2);
            break;
        case CAN_IT_FMP0:
            /* Check CAN_RF0R_FMP0 bit */
            itstatus = CheckITStatus(CANx->RF0R, CAN_RF0R_FMP0);
            break;
        case CAN_IT_FF0:
            /* Check CAN_RF0R_FULL0 bit */
            itstatus = CheckITStatus(CANx->RF0R, CAN_RF0R_FULL0);
            break;
        case CAN_IT_FOV0:
            /* Check CAN_RF0R_FOVR0 bit */
            itstatus = CheckITStatus(CANx->RF0R, CAN_RF0R_FOVR0);
            break;
        case CAN_IT_FMP1:
            /* Check CAN_RF1R_FMP1 bit */
            itstatus = CheckITStatus(CANx->RF1R, CAN_RF1R_FMP1);
            break;
        case CAN_IT_FF1:
            /* Check CAN_RF1R_FULL1 bit */
            itstatus = CheckITStatus(CANx->RF1R, CAN_RF1R_FULL1);
            break;
        case CAN_IT_FOV1:
            /* Check CAN_RF1R_FOVR1 bit */
            itstatus = CheckITStatus(CANx->RF1R, CAN_RF1R_FOVR1);
            break;
        case CAN_IT_WKU:
            /* Check CAN_MSR_WKUI bit */
            itstatus = CheckITStatus(CANx->MSR, CAN_MSR_WKUI);
            break;
        case CAN_IT_SLK:
            /* Check CAN_MSR_SLAKI bit */
            itstatus = CheckITStatus(CANx->MSR, CAN_MSR_SLAKI);
            break;
        case CAN_IT_EWG:
            /* Check CAN_ESR_EWGF bit */
            itstatus = CheckITStatus(CANx->ESR, CAN_ESR_EWGF);
            break;
        case CAN_IT_EPV:
            /* Check CAN_ESR_EPVF bit */
            itstatus = CheckITStatus(CANx->ESR, CAN_ESR_EPVF);
            break;
        case CAN_IT_BOF:
            /* Check CAN_ESR_BOFF bit */
            itstatus = CheckITStatus(CANx->ESR, CAN_ESR_BOFF);
            break;
        case CAN_IT_LEC:
            /* Check CAN_ESR_LEC bit */
            itstatus = CheckITStatus(CANx->ESR, CAN_ESR_LEC);
            break;
        case CAN_IT_ERR:
            /* Check CAN_MSR_ERRI bit */
            itstatus = CheckITStatus(CANx->MSR, CAN_MSR_ERRI);
            break;
        default:
            /* in case of error, return RESET */
            itstatus = RESET;
            break;
        }
    }
    else {
        /* in case the Interrupt is not enabled, return RESET */
        itstatus = RESET;
    }

    /* Return the CAN_IT status */
    return  itstatus;
}


bool CAN_ErrorHandler(uint8_t controller) {
    DISABLE_INTERRUPTS_SCOPED(irq);

    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    if (CAN_GetITStatus(CANx, CAN_IT_FF0)) {
        CAN_ClearITPendingBit(CANx, CAN_IT_FF0);
        canController[controller].errorEventHandler(canController[controller].provider, controller, TinyCLR_Can_Error::ReadBufferFull);

        return true;
    }
    else if (CAN_GetITStatus(CANx, CAN_IT_FOV0)) {
        CAN_ClearITPendingBit(CANx, CAN_IT_FOV0);
        canController[controller].errorEventHandler(canController[controller].provider, controller, TinyCLR_Can_Error::ReadBufferOverrun);

        return true;
    }
    else if (CAN_GetITStatus(CANx, CAN_IT_BOF)) {
        CAN_ClearITPendingBit(CANx, CAN_IT_BOF);
        canController[controller].errorEventHandler(canController[controller].provider, controller, TinyCLR_Can_Error::BusOff);

        return true;
    }
    else if (CAN_GetITStatus(CANx, CAN_IT_EPV)) {
        CAN_ClearITPendingBit(CANx, CAN_IT_EPV);
        canController[controller].errorEventHandler(canController[controller].provider, controller, TinyCLR_Can_Error::Passive);

        return true;
    }

    else if (CAN_GetITStatus(CANx, CAN_IT_LEC)) {
        CAN_ClearITPendingBit(CANx, CAN_IT_LEC);
    }
    else if (CAN_GetITStatus(CANx, CAN_IT_ERR)) {
        CAN_ClearITPendingBit(CANx, CAN_IT_ERR);
        canController[controller].errorEventHandler(canController[controller].provider, controller, TinyCLR_Can_Error::Passive);

        return true;
    }
    else if (CAN_GetITStatus(CANx, CAN_IT_EWG)) {
        CAN_ClearITPendingBit(CANx, CAN_IT_EWG);
        canController[controller].errorEventHandler(canController[controller].provider, controller, TinyCLR_Can_Error::Passive);
    }

    return false;
}


const TinyCLR_Api_Info* STM32F4_Can_GetApi() {
    canProvider.Parent = &canApi;
    canProvider.Acquire = &STM32F4_Can_Acquire;
    canProvider.Release = &STM32F4_Can_Release;
    canProvider.Reset = &STM32F4_Can_SoftReset;
    canProvider.WriteMessage = &STM32F4_Can_WriteMessage;
    canProvider.ReadMessage = &STM32F4_Can_ReadMessage;
    canProvider.SetBitTiming = &STM32F4_Can_SetBitTiming;
    canProvider.GetUnreadMessageCount = &STM32F4_Can_GetUnreadMessageCount;
    canProvider.SetMessageReceivedHandler = &STM32F4_Can_SetMessageReceivedHandler;
    canProvider.SetErrorReceivedHandler = &STM32F4_Can_SetErrorReceivedHandler;
    canProvider.SetExplicitFilters = &STM32F4_Can_SetExplicitFilters;
    canProvider.SetGroupFilters = &STM32F4_Can_SetGroupFilters;
    canProvider.ClearReadBuffer = &STM32F4_Can_ClearReadBuffer;
    canProvider.IsWritingAllowed = &STM32F4_Can_IsWritingAllowed;
    canProvider.GetWriteErrorCount = &STM32F4_Can_GetWriteErrorCount;
    canProvider.GetReadErrorCount = &STM32F4_Can_GetReadErrorCount;
    canProvider.GetSourceClock = &STM32F4_Can_GetSourceClock;
    canProvider.GetReadBufferSize = STM32F4_Can_GetReadBufferSize;
    canProvider.SetReadBufferSize = STM32F4_Can_SetReadBufferSize;
    canProvider.GetWriteBufferSize = STM32F4_Can_GetWriteBufferSize;
    canProvider.SetWriteBufferSize = STM32F4_Can_SetWriteBufferSize;
    canProvider.GetControllerCount = STM32F4_Can_GetControllerCount;

    canApi.Author = "GHI Electronics, LLC";
    canApi.Name = "GHIElectronics.TinyCLR.NativeApis.STM32F4.CanProvider";
    canApi.Type = TinyCLR_Api_Type::CanProvider;
    canApi.Version = 0;
    canApi.Implementation = &canProvider;

    return &canApi;
}

TinyCLR_Result STM32F4_Can_GetReadBufferSize(const TinyCLR_Can_Provider* self, int32_t controller, size_t& size) {

    size = canController[controller].can_rxBufferSize == 0 ? g_STM32F4_Can_defaultBuffersSize[controller] : canController[controller].can_rxBufferSize;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_SetReadBufferSize(const TinyCLR_Can_Provider* self, int32_t controller, size_t size) {

    if (size > 3) {
        canController[controller].can_rxBufferSize = size;
        return TinyCLR_Result::Success;
    }
    else {
        canController[controller].can_rxBufferSize = g_STM32F4_Can_defaultBuffersSize[controller];
        return TinyCLR_Result::ArgumentInvalid;;
    }
}

TinyCLR_Result STM32F4_Can_GetWriteBufferSize(const TinyCLR_Can_Provider* self, int32_t controller, size_t& size) {
    size = 1;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_SetWriteBufferSize(const TinyCLR_Can_Provider* self, int32_t controller, size_t size) {

    canController[controller].can_txBufferSize = 1;

    return size == 1 ? TinyCLR_Result::Success : TinyCLR_Result::NotSupported;
}

uint32_t STM32_Can_GetLocalTime() {
    return STM32F4_Time_GetTimeForProcessorTicks(nullptr, STM32F4_Time_GetCurrentProcessorTicks(nullptr));
}
void STM32_Can_RxInterruptHandler(int32_t controller) {
    DISABLE_INTERRUPTS_SCOPED(irq);

    uint32_t* pDest;

    int32_t len = 0;

    uint32_t msgid = 0;

    bool extendMode = 0;
    bool rtrmode = 0;

    char passed = 0;

    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    STM32F4_Can_RxMessage rxMessage;

    uint32_t error = CAN_ErrorHandler(controller);

    CAN_Receive(CANx, CAN_FIFO0, &rxMessage);

    if (error)
        return;

    len = rxMessage.DLC;

    if (rxMessage.IDE == CAN_Id_Standard) {
        msgid = rxMessage.StdId;
        extendMode = false;
    }
    else {
        msgid = rxMessage.ExtId;
        extendMode = true;
    }

    rtrmode = (((rxMessage.RTR) & 0x02) != 0) ? true : false;

    // Filter
    if (canController[controller].canDataFilter.groupFiltersSize || canController[controller].canDataFilter.matchFiltersSize) {
        if (canController[controller].canDataFilter.groupFiltersSize) {
            if (BinarySearch2(canController[controller].canDataFilter.lowerBoundFilters, canController[controller].canDataFilter.upperBoundFilters, 0, canController[controller].canDataFilter.groupFiltersSize - 1, msgid) >= 0)
                passed = 1;
        }

        if (!passed && canController[controller].canDataFilter.matchFiltersSize) {
            if (BinarySearch(canController[controller].canDataFilter.matchFilters, 0, canController[controller].canDataFilter.matchFiltersSize - 1, msgid) >= 0)
                passed = 1;
        }

        if (!passed) {
            return;
        }
    }

    if (canController[controller].can_rx_count > canController[controller].can_rxBufferSize - 3) {
        return;
    }

    STM32F4_Can_Message *can_msg = &canController[controller].canRxMessagesFifo[canController[controller].can_rx_in];

    uint64_t t = STM32_Can_GetLocalTime();

    can_msg->TimeStampL = t & 0xFFFFFFFF;

    can_msg->TimeStampH = t >> 32;

    can_msg->MsgID = msgid;

    can_msg->extendedId = extendMode;

    can_msg->remoteTransmissionRequest = rtrmode;

    can_msg->DataA = rxMessage.Data[0] | (rxMessage.Data[1] << 8) | (rxMessage.Data[2] << 16) | (rxMessage.Data[3] << 24);

    can_msg->DataB = rxMessage.Data[4] | (rxMessage.Data[5] << 8) | (rxMessage.Data[6] << 16) | (rxMessage.Data[7] << 24);

    can_msg->length = len;

    canController[controller].can_rx_count++;
    canController[controller].can_rx_in++;

    if (canController[controller].can_rx_in == canController[controller].can_rxBufferSize) {
        canController[controller].can_rx_in = 0;
    }

    canController[controller].messageReceivedEventHandler(canController[controller].provider, controller, canController[controller].can_rx_count);

    return;
}

void STM32F4_Can_TxInterruptHandler0(void *param) {
    CAN_ErrorHandler(0);
}

void STM32F4_Can_TxInterruptHandler1(void *param) {
    CAN_ErrorHandler(1);
}

void STM32F4_Can_RxInterruptHandler0(void *param) {
    STM32_Can_RxInterruptHandler(0);
}

void STM32F4_Can_RxInterruptHandler1(void *param) {
    STM32_Can_RxInterruptHandler(1);
}

TinyCLR_Result STM32F4_Can_Acquire(const TinyCLR_Can_Provider* self, int32_t controller) {
    if (self == nullptr)
        return TinyCLR_Result::ArgumentNull;

    if (!STM32F4_GpioInternal_OpenPin(g_STM32F4_Can_Tx_Pins[controller].number))
        return TinyCLR_Result::SharingViolation;

    if (!STM32F4_GpioInternal_OpenPin(g_STM32F4_Can_Rx_Pins[controller].number))
        return TinyCLR_Result::SharingViolation;

    // set pin as analog
    STM32F4_GpioInternal_ConfigurePin(g_STM32F4_Can_Tx_Pins[controller].number, STM32F4_Gpio_PortMode::AlternateFunction, STM32F4_Gpio_OutputType::PushPull, STM32F4_Gpio_OutputSpeed::High, STM32F4_Gpio_PullDirection::PullUp, g_STM32F4_Can_Tx_Pins[controller].alternateFunction);
    STM32F4_GpioInternal_ConfigurePin(g_STM32F4_Can_Rx_Pins[controller].number, STM32F4_Gpio_PortMode::AlternateFunction, STM32F4_Gpio_OutputType::PushPull, STM32F4_Gpio_OutputSpeed::High, STM32F4_Gpio_PullDirection::PullUp, g_STM32F4_Can_Rx_Pins[controller].alternateFunction);

    canController[controller].can_rx_count = 0;
    canController[controller].can_rx_in = 0;
    canController[controller].can_rx_out = 0;
    canController[controller].baudrate = 0;
    canController[controller].can_rxBufferSize = g_STM32F4_Can_defaultBuffersSize[controller];
    canController[controller].provider = self;

    canController[controller].canRxMessagesFifo = nullptr;

    canController[controller].isOpened = true;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_Release(const TinyCLR_Can_Provider* self, int32_t controller) {
    if (self == nullptr)
        return TinyCLR_Result::ArgumentNull;

    auto memoryProvider = (const TinyCLR_Memory_Provider*)apiProvider->FindDefault(apiProvider, TinyCLR_Api_Type::MemoryProvider);

    RCC->APB1ENR &= ((controller == 0) ? ~RCC_APB1ENR_CAN1EN : ~RCC_APB1ENR_CAN2EN);

    if (canController[controller].canRxMessagesFifo != nullptr) {
        memoryProvider->Free(memoryProvider, canController[controller].canRxMessagesFifo);

        canController[controller].canRxMessagesFifo = nullptr;
    }

    if (canController[controller].isOpened) {
        STM32F4_GpioInternal_ClosePin(g_STM32F4_Can_Tx_Pins[controller].number);
        STM32F4_GpioInternal_ClosePin(g_STM32F4_Can_Rx_Pins[controller].number);
    }

    canController[controller].isOpened = false;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_SoftReset(const TinyCLR_Can_Provider* self, int32_t controller) {

    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    canController[controller].can_rx_count = 0;
    canController[controller].can_rx_in = 0;
    canController[controller].can_rx_out = 0;

    RCC->APB1RSTR |= ((controller == 0) ? RCC_APB1ENR_CAN1EN : RCC_APB1ENR_CAN2EN);

    STM32F4_Time_Delay(nullptr, 1000);

    RCC->APB1RSTR &= ((controller == 0) ? ~RCC_APB1ENR_CAN1EN : ~RCC_APB1ENR_CAN2EN);

    RCC->APB1ENR |= ((controller == 0) ? RCC_APB1ENR_CAN1EN : (RCC_APB1ENR_CAN1EN | RCC_APB1ENR_CAN2EN));

    CAN_Initialize(CANx, &canController[controller].initTypeDef);

    CAN_FilterInit(&canController[controller].filterInitTypeDef);

    CANx->IER |= (CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0 | CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC | CAN_IT_ERR);

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_WriteMessage(const TinyCLR_Can_Provider* self, int32_t controller, uint32_t arbitrationId, bool isExtendedId, bool isRemoteTransmissionRequest, uint8_t* data, size_t length) {

    uint32_t* data32 = (uint32_t*)data;

    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    uint32_t i = 0;

    uint8_t txmailbox;

    STM32F4_Can_TxMessage txMessage;

    /* Transmit Structure preparation */
    txMessage.RTR = (isRemoteTransmissionRequest == true) ? 1 : 0;

    if (isExtendedId) {
        txMessage.IDE = CAN_Id_Extended;
        txMessage.ExtId = arbitrationId;
    }
    else {
        txMessage.IDE = CAN_Id_Standard;
        txMessage.StdId = arbitrationId;
    }
    txMessage.DLC = length & 0x0F;

    txMessage.Data[0] = ((data32[0] >> 0) & 0xFF);
    txMessage.Data[1] = ((data32[0] >> 8) & 0xFF);
    txMessage.Data[2] = ((data32[0] >> 16) & 0xFF);
    txMessage.Data[3] = ((data32[0] >> 24) & 0xFF);
    txMessage.Data[4] = ((data32[1] >> 0) & 0xFF);
    txMessage.Data[5] = ((data32[1] >> 8) & 0xFF);
    txMessage.Data[6] = ((data32[1] >> 16) & 0xFF);
    txMessage.Data[7] = ((data32[1] >> 24) & 0xFF);

    txmailbox = CAN_Transmit(CANx, &txMessage);

    while (CAN_TransmitStatus(CANx, txmailbox) != CAN_TxStatus_Ok && i++ < CAN_GetTransferTimeout())
        STM32F4_Time_Delay(nullptr, 1);

    if (txmailbox == CAN_TxStatus_NoMailBox || i == CAN_GetTransferTimeout()) {

        CAN_CancelTransmit(CANx, txmailbox);
        CAN_ErrorHandler(controller);
        return TinyCLR_Result::InvalidOperation;
    }

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_ReadMessage(const TinyCLR_Can_Provider* self, int32_t controller, uint32_t& arbitrationId, bool& isExtendedId, bool& isRemoteTransmissionRequest, uint64_t& timestamp, uint8_t* data, size_t& length) {
    STM32F4_Can_Message *can_msg;

    uint32_t* data32 = (uint32_t*)data;

    if (canController[controller].can_rx_count) {
        DISABLE_INTERRUPTS_SCOPED(irq);

        can_msg = &canController[controller].canRxMessagesFifo[canController[controller].can_rx_out];
        canController[controller].can_rx_out++;

        if (canController[controller].can_rx_out == canController[controller].can_rxBufferSize)
            canController[controller].can_rx_out = 0;

        canController[controller].can_rx_count--;

        arbitrationId = can_msg->MsgID;
        isExtendedId = can_msg->extendedId;
        isRemoteTransmissionRequest = can_msg->remoteTransmissionRequest;
        length = can_msg->length;

        data32[0] = can_msg->DataA;
        data32[1] = can_msg->DataB;

        timestamp = ((uint64_t)can_msg->TimeStampL) | ((uint64_t)can_msg->TimeStampH << 32);
    }

    return TinyCLR_Result::Success;

}

TinyCLR_Result STM32F4_Can_SetBitTiming(const TinyCLR_Can_Provider* self, int32_t controller, int32_t propagation, int32_t phase1, int32_t phase2, int32_t baudratePrescaler, int32_t synchronizationJumpWidth, int8_t useMultiBitSampling) {
    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    auto memoryProvider = (const TinyCLR_Memory_Provider*)apiProvider->FindDefault(apiProvider, TinyCLR_Api_Type::MemoryProvider);

    if (canController[controller].canRxMessagesFifo == nullptr)
        canController[controller].canRxMessagesFifo = (STM32F4_Can_Message*)memoryProvider->Allocate(memoryProvider, canController[controller].can_rxBufferSize * sizeof(STM32F4_Can_Message));

    if (canController[controller].canRxMessagesFifo == nullptr) {
        return TinyCLR_Result::OutOfMemory;
    }

    RCC->APB1RSTR |= ((controller == 0) ? RCC_APB1ENR_CAN1EN : RCC_APB1ENR_CAN2EN);

    STM32F4_Time_Delay(nullptr, 1000);

    RCC->APB1RSTR &= ((controller == 0) ? ~RCC_APB1ENR_CAN1EN : ~RCC_APB1ENR_CAN2EN);

    RCC->APB1ENR |= ((controller == 0) ? RCC_APB1ENR_CAN1EN : (RCC_APB1ENR_CAN1EN | RCC_APB1ENR_CAN2EN));

    canController[controller].baudrate = (((synchronizationJumpWidth - 1) << 24) | ((phase2 - 1) << 20) | ((phase1 - 1) << 16) | baudratePrescaler);

    canController[controller].initTypeDef.CAN_TTCM = DISABLE;
    canController[controller].initTypeDef.CAN_ABOM = DISABLE;
    canController[controller].initTypeDef.CAN_AWUM = DISABLE;
    canController[controller].initTypeDef.CAN_NART = DISABLE;
    canController[controller].initTypeDef.CAN_RFLM = DISABLE;
    canController[controller].initTypeDef.CAN_TXFP = DISABLE;
    canController[controller].initTypeDef.CAN_Mode = CAN_Mode_Normal;

    canController[controller].initTypeDef.CAN_SJW = ((canController[controller].baudrate >> 24) & 0x03);
    canController[controller].initTypeDef.CAN_BS2 = ((canController[controller].baudrate >> 20) & 0x07);
    canController[controller].initTypeDef.CAN_BS1 = ((canController[controller].baudrate >> 16) & 0x0F);
    canController[controller].initTypeDef.CAN_Prescaler = ((canController[controller].baudrate >> 0) & 0x3FF);

    CAN_Initialize(CANx, &canController[controller].initTypeDef);

    canController[controller].filterInitTypeDef.CAN_FilterNumber = (controller == 0 ? 0 : 14);

    canController[controller].filterInitTypeDef.CAN_FilterMode = CAN_FilterMode_IdMask;
    canController[controller].filterInitTypeDef.CAN_FilterScale = CAN_FilterScale_32bit;
    canController[controller].filterInitTypeDef.CAN_FilterIdHigh = 0x0000;
    canController[controller].filterInitTypeDef.CAN_FilterIdLow = 0x0000;
    canController[controller].filterInitTypeDef.CAN_FilterMaskIdHigh = 0x0000;
    canController[controller].filterInitTypeDef.CAN_FilterMaskIdLow = 0x0000;
    canController[controller].filterInitTypeDef.CAN_FilterFIFOAssignment = 0;
    canController[controller].filterInitTypeDef.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&canController[controller].filterInitTypeDef);

    if (controller == 0) {
        STM32F4_InterruptInternal_Activate(CAN1_TX_IRQn, (uint32_t*)&STM32F4_Can_TxInterruptHandler0, 0);
        STM32F4_InterruptInternal_Activate(CAN1_RX0_IRQn, (uint32_t*)&STM32F4_Can_RxInterruptHandler0, 0);
    }
    else {
        STM32F4_InterruptInternal_Activate(CAN2_TX_IRQn, (uint32_t*)&STM32F4_Can_TxInterruptHandler1, 0);
        STM32F4_InterruptInternal_Activate(CAN2_RX0_IRQn, (uint32_t*)&STM32F4_Can_RxInterruptHandler1, 0);
    }

    CANx->IER |= (CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0 | CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC | CAN_IT_ERR);

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_GetUnreadMessageCount(const TinyCLR_Can_Provider* self, int32_t controller, size_t& count) {

    count = canController[controller].can_rx_count;

    if (!count)
        return TinyCLR_Result::NoDataAvailable;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_SetMessageReceivedHandler(const TinyCLR_Can_Provider* self, int32_t controller, TinyCLR_Can_MessageReceivedHandler handler) {

    canController[controller].messageReceivedEventHandler = handler;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_SetErrorReceivedHandler(const TinyCLR_Can_Provider* self, int32_t controller, TinyCLR_Can_ErrorReceivedHandler handler) {

    canController[controller].errorEventHandler = handler;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_SetExplicitFilters(const TinyCLR_Can_Provider* self, int32_t controller, uint8_t* filters, size_t length) {
    uint32_t*_matchFilters;
    uint32_t*filters32 = (uint32_t*)filters;

    auto memoryProvider = (const TinyCLR_Memory_Provider*)apiProvider->FindDefault(apiProvider, TinyCLR_Api_Type::MemoryProvider);

    _matchFilters = (uint32_t*)memoryProvider->Allocate(memoryProvider, length * sizeof(uint32_t));

    if (!_matchFilters)
        return TinyCLR_Result::OutOfMemory;

    memcpy(_matchFilters, filters32, length * sizeof(uint32_t));

    std::sort(_matchFilters, _matchFilters + length);

    {
        DISABLE_INTERRUPTS_SCOPED(irq);

        canController[controller].canDataFilter.matchFiltersSize = length;
        canController[controller].canDataFilter.matchFilters = _matchFilters;
    }

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_SetGroupFilters(const TinyCLR_Can_Provider* self, int32_t controller, uint8_t* lowerBounds, uint8_t* upperBounds, size_t length) {
    uint32_t* _lowerBoundFilters, *_upperBoundFilters;
    uint32_t* lowerBounds32 = (uint32_t *)lowerBounds;
    uint32_t* upperBounds32 = (uint32_t *)upperBounds;

    auto memoryProvider = (const TinyCLR_Memory_Provider*)apiProvider->FindDefault(apiProvider, TinyCLR_Api_Type::MemoryProvider);

    _lowerBoundFilters = (uint32_t*)memoryProvider->Allocate(memoryProvider, length * sizeof(uint32_t));
    _upperBoundFilters = (uint32_t*)memoryProvider->Allocate(memoryProvider, length * sizeof(uint32_t));

    if (!_lowerBoundFilters || !_upperBoundFilters) {
        memoryProvider->Free(memoryProvider, _lowerBoundFilters);
        memoryProvider->Free(memoryProvider, _upperBoundFilters);

        return  TinyCLR_Result::OutOfMemory;
    }

    memcpy(_lowerBoundFilters, lowerBounds32, length * sizeof(uint32_t));
    memcpy(_upperBoundFilters, upperBounds32, length * sizeof(uint32_t));

    bool success = InsertionSort2CheckOverlap(_lowerBoundFilters, _upperBoundFilters, length);

    if (!success) {
        memoryProvider->Free(memoryProvider, _lowerBoundFilters);
        memoryProvider->Free(memoryProvider, _upperBoundFilters);

        return TinyCLR_Result::ArgumentInvalid;
    }

    {
        DISABLE_INTERRUPTS_SCOPED(irq);

        canController[controller].canDataFilter.groupFiltersSize = length;
        canController[controller].canDataFilter.lowerBoundFilters = _lowerBoundFilters;
        canController[controller].canDataFilter.upperBoundFilters = _upperBoundFilters;
    }

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_ClearReadBuffer(const TinyCLR_Can_Provider* self, int32_t controller) {
    canController[controller].can_rx_count = 0;
    canController[controller].can_rx_in = 0;
    canController[controller].can_rx_out = 0;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_IsWritingAllowed(const TinyCLR_Can_Provider* self, int32_t controller, bool& allowed) {
    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    allowed = false;

    if ((CANx->TSR&CAN_TSR_TME0) == CAN_TSR_TME0 || (CANx->TSR&CAN_TSR_TME1) == CAN_TSR_TME1 || (CANx->TSR&CAN_TSR_TME2) == CAN_TSR_TME2) allowed = true;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_GetReadErrorCount(const TinyCLR_Can_Provider* self, int32_t controller, size_t& count) {

    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    count = (uint8_t)((CANx->ESR & CAN_ESR_REC) >> 24);;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_GetWriteErrorCount(const TinyCLR_Can_Provider* self, int32_t controller, size_t& count) {

    CAN_TypeDef* CANx = ((controller == 0) ? CAN1 : CAN2);

    count = (uint8_t)((CANx->ESR & CAN_ESR_REC) >> 16);;

    return TinyCLR_Result::Success;
}

TinyCLR_Result STM32F4_Can_GetSourceClock(const TinyCLR_Can_Provider* self, int32_t controller, uint32_t& sourceClock) {
    sourceClock = STM32F4_APB1_CLOCK_HZ;

    return TinyCLR_Result::Success;
}

void STM32F4_Can_Reset() {
    for (int i = 0; i < TOTAL_CAN_CONTROLLERS; i++) {
        canController[i].canRxMessagesFifo = nullptr;

        STM32F4_Can_Release(&canProvider, i);

        canController[i].isOpened = false;
    }
}

TinyCLR_Result STM32F4_Can_GetControllerCount(const TinyCLR_Can_Provider* self, int32_t& count) {
    count = TOTAL_CAN_CONTROLLERS;

    return TinyCLR_Result::Success;
}
#endif // INCLUDE_CAN
