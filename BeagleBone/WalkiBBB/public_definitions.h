/**
 * @file
 * @brief Definitions that should be used by the client programs of WalkiBBB.
 */

#ifndef DEF_MAINBOARD_PUBLIC_DEFINITIONS_H
#define DEF_MAINBOARD_PUBLIC_DEFINITIONS_H

#define TCP_PORT 9255 ///< TCP communication port.
#define TCP_MAX_TIME_WITHOUT_RX 10000 ///< Maximum time without received message, from which the client is considered as disconnected [ms].

/**
 * @brief Remote computer to mainboard message types enumeration.
 */
enum class PcToMbMessageType
{
    HEARTBEAT = 0,
    SET_DATE = 1,

    GET_VARS_LIST = 8,
    GET_VAR = 9,
    SET_VAR = 10,
    SET_VAR_LOG = 14,
    SET_STREAMING = 11,

    LOG_MESSAGE = 12,
    SYNC_LED = 13
};

/**
 * @brief Mainboard to remote computer message types enumeration.
 */
enum class MbToPcMessageType
{
    HEARTBEAT = 0,
    STATUS, // Joints positions, pitch, roll, current mode, error flags...
    DEBUG_TEXT,

    VARS_LIST,
    VAR_VALUE,
    STREAMING
};

/**
 * @brief SyncVar variable types enumeration.
 */
enum class VarType
{
    BOOL = 0,
    UINT8, INT8, UINT16, INT16, UINT32, INT32, UINT64, INT64,
    FLOAT32, FLOAT64,
    STRING
};

/**
 * @brief SyncVar variable accesses enumeration.
 */
enum class VarAccess
{
    NONE = 0, READ, WRITE, READWRITE
};

/**
 * @brief Peripheral states enumeration.
 */
enum PeripheralState
{
    DISABLED = 0,
    ACTIVE,
    CALIBRATING,
    FAULT
};

#define LOGFILES_N_DIGITS 5 ///< Number of digits for the logs filename.
#define LOGFILES_PREFIX "log_" ///< Prefix for the logs filename.

#define SYNCVAR_NAME_COMM_LENGTH 50 ///< Length of SyncVar names during remote listing.
#define SYNCVAR_UNIT_COMM_LENGTH 20 ///< Length of SyncVar units during remote listing.
#define SYNCVAR_LIST_ITEM_SIZE (SYNCVAR_NAME_COMM_LENGTH + SYNCVAR_UNIT_COMM_LENGTH + 1 + 1 + 4) ///< SyncVar description bytes size during remote listing.
#define SYNCVAR_NAME_SEPARATOR '/' ///< SyncVar prefix separators.

#endif
