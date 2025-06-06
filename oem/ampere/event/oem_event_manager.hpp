#pragma once

#include "common/instance_id.hpp"
#include "common/types.hpp"
#include "oem_event_manager.hpp"
#include "platform-mc/manager.hpp"
#include "requester/handler.hpp"
#include "requester/request.hpp"

#include <libpldm/pldm.h>

namespace pldm
{
namespace oem_ampere
{
using namespace pldm::pdr;
#define NORMAL_EVENT_POLLING_TIME 5000000 // ms

using EventToMsgMap_t = std::unordered_map<uint8_t, std::string>;

enum sensor_ids
{
    DDR_STATUS = 51,
    PCP_VR_STATE = 75,
    SOC_VR_STATE = 80,
    DPHY_VR1_STATE = 85,
    DPHY_VR2_STATE = 90,
    D2D_VR_STATE = 95,
    IOC_VR1_STATE = 100,
    IOC_VR2_STATE = 105,
    PCI_D_VR_STATE = 110,
    PCI_A_VR_STATE = 115,
    PCIE_HOT_PLUG = 169,
    SOC_HEALTH_AVAILABILITY = 170,
    BOOT_OVERALL = 175,
    WATCH_DOG = 179,
    CORE_UE = 192,
    MCU_UE = 194,
    PCIE_UE = 196,
    SOC_UE = 198,
    SOC_BERT = 200,
};

namespace boot
{
namespace status
{
enum boot_status
{
    BOOT_STATUS_SUCCESS = 0x80,
    BOOT_STATUS_FAILURE = 0x81,
};
} // namespace status
namespace stage
{
enum boot_stage
{
    UEFI_STATUS_CLASS_CODE_MIN = 0x00,
    UEFI_STATUS_CLASS_CODE_MAX = 0x7f,
    SECPRO = 0x90,
    MPRO = 0x91,
    ATF_BL1 = 0x92,
    ATF_BL2 = 0x93,
    DDR_INITIALIZATION = 0x94,
    DDR_TRAINING = 0x95,
    S0_DDR_TRAINING_FAILURE = 0x96,
    ATF_BL31 = 0x97,
    ATF_BL32 = 0x98,
    S1_DDR_TRAINING_FAILURE = 0x99,
};
} // namespace stage
} // namespace boot

enum class log_level : int
{
    OK,
    WARNING,
    CRITICAL,
    BIOSFWPANIC,
};

/*
 * PresentReading value format
 * FIELD       |                   COMMENT
 * Bit 31      |   Reserved
 * Bit 30:24   |   Media slot number (0 - 63) This field can be used by UEFI
 *             |   to indicate the media slot number (such as NVMe/SSD slot)
 *             |   (7 bits)
 * Bit 23      |   Operation status: 1 = operation failed
 *             |   0 = operation successful
 * Bit 22      |   Action: 0 - Insertion 1 - Removal
 * Bit 21:18   |   Function (4 bits)
 * Bit 17:13   |   Device (5 bits)
 * Bit 12:5    |   Bus (8 bits)
 * Bit 4:0     |   Segment (5 bits)
 */
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t segment:5;
        uint32_t bus:8;
        uint32_t device:5;
        uint32_t function:4;
        uint32_t action:1;
        uint32_t opStatus:1;
        uint32_t mediaSlot:7;
        uint32_t reserved:1;
    } __attribute__((packed)) bits;
} PCIeHotPlugEventRecord_t;

typedef union
{
    uint32_t value;
    struct
    {
        uint32_t type:2;
        uint32_t mcuRankIdx:3;
        uint32_t reserved_1:3; // byte0
        uint32_t sliceNum:4;
        uint32_t upperNibbStatErr:1;
        uint32_t lowerNibbStatErr:1;
        uint32_t reserved_2:2; // byte1
        uint32_t syndrome:4;
        uint32_t reserved_3:4; // byte2
        uint32_t reserved_byte:8;
    } __attribute__((packed)) bits;
} DIMMTrainingFailure_t;

namespace ddr
{
namespace status
{
enum ddr_status
{
    NO_SYSTEM_LEVEL_ERROR = 0x01,
    ECC_INITIALIZATION_FAILURE = 0x04,
    CONFIGURATION_FAILURE = 0x05,
    TRAINING_FAILURE = 0x06,
    OTHER_FAILURE = 0x07,
    BOOT_FAILURE_NO_VALID_CONFIG = 0x08,
    FAILSAFE_ACTIVATED_NEXT_BOOT_SUCCESS = 0x09,
};
}
} // namespace ddr

namespace dimm
{
namespace status
{
enum dimm_status
{
    INSTALLED_NO_ERROR = 0x01,
    NOT_INSTALLED = 0x02,
    OTHER_FAILURE = 0x07,
    INSTALLED_BUT_DISABLED = 0x10,
    TRAINING_FAILURE = 0x12,
    PMIC_HIGH_TEMP = 0x13,
    TSx_HIGH_TEMP = 0x14,
    SPD_HUB_HIGH_TEMP = 0x15,
    PMIC_TEMP_ALERT = 0x16,
};
} // namespace status

namespace training_failure
{
enum dimm_training_failure_type
{
    PHY_TRAINING_FAILURE_TYPE = 0x01,
    DIMM_TRAINING_FAILURE_TYPE = 0x02,
};

namespace phy_syndrome
{
enum phy_training_failure_syndrome
{
    NA = 0x00,
    PHY_TRAINING_SETUP_FAILURE = 0x01,
    CA_LEVELING = 0x02,
    PHY_WRITE_LEVEL_FAILURE = 0x03,
    PHY_READ_GATE_LEVELING_FAILURE = 0x04,
    PHY_READ_LEVEL_FAILURE = 0x05,
    WRITE_DQ_LEVELING = 0x06,
    PHY_SW_TRAINING_FAILURE = 0x07,
};
} // namespace phy_syndrome

namespace dimm_syndrome
{
enum dimm_training_failure_syndrome
{
    NA = 0x00,
    DRAM_VREFDQ_TRAINING_FAILURE = 0x01,
    LRDIMM_DB_TRAINING_FAILURE = 0x02,
    LRDRIMM_DB_SW_TRAINING_FAILURE = 0x03,
};
} // namespace dimm_syndrome
} // namespace training_failure
} // namespace dimm

/*
 * PresentReading value format
 * FIELD       |                   COMMENT
 * Bit 31:30   |   Reserved (2 bits)
 * Bit 29      |   A VR Critical condition observed (1 bit)
 * Bit 28      |   A VR Warning condition observed (1 bit)
 * Bit 27:16   |   Reserved (12 bits)
 * Bit 15:8    |   VR status byte high - The bit definition is the same as the
 *             |   corresponding VR PMBUS STATUS_WORD (upper byte) (8 bits)
 * Bit 7:0     |   VR status byte low - The bit definition is the same as the
 *             |   corresponding VR PMBUS STATUS_WORD (lower byte) (8 bits)
 */
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t vr_status_byte_low:8;
        uint32_t vr_status_byte_high:8;
        uint32_t reserved_1:12;
        uint32_t warning:1;
        uint32_t critical:1;
        uint32_t reserved_2:2;
    } __attribute__((packed)) bits;
} VRDStatus_t;

/**
 * @brief OemEventManager
 *
 *
 */
class OemEventManager
{
  public:
    OemEventManager() = delete;
    OemEventManager(const OemEventManager&) = delete;
    OemEventManager(OemEventManager&&) = delete;
    OemEventManager& operator=(const OemEventManager&) = delete;
    OemEventManager& operator=(OemEventManager&&) = delete;
    virtual ~OemEventManager() = default;

    explicit OemEventManager(
        sdeventplus::Event& event,
        requester::Handler<requester::Request>* /* handler */,
        pldm::InstanceIdDb& /* instanceIdDb */,
        platform_mc::Manager* platformManager) :
        event(event), manager(platformManager) {};

    /** @brief Decode sensor event messages and handle correspondingly.
     *
     *  @param[in] request - the request message of sensor event
     *  @param[in] payloadLength - the payload length of sensor event
     *  @param[in] formatVersion - the format version of sensor event
     *  @param[in] tid - TID
     *  @param[in] eventDataOffset - the event data offset of sensor event
     *
     *  @return int - returned error code
     */
    int handleSensorEvent(const pldm_msg* request, size_t payloadLength,
                          uint8_t /* formatVersion */, pldm_tid_t tid,
                          size_t eventDataOffset);

    /** @brief Handle the polled CPER (0x07, 0xFA) event class.
     *
     *  @param[in] tid - terminus ID
     *  @param[out] eventId - Event ID
     *  @param[in] eventData - event data
     *  @param[in] eventDataSize - size of event data
     *
     *  @return int - PLDM completion code
     */
    int processOemMsgPollEvent(pldm_tid_t tid, uint16_t eventId,
                               const uint8_t* eventData, size_t eventDataSize);

    /** @brief Decode sensor event messages and handle correspondingly.
     *
     *  @param[in] request - the request message of sensor event
     *  @param[in] payloadLength - the payload length of sensor event
     *  @param[in] formatVersion - the format version of sensor event
     *  @param[in] tid - TID
     *  @param[in] eventDataOffset - the event data offset of sensor event
     *
     *  @return int - returned error code
     */
    int handlepldmMessagePollEvent(
        const pldm_msg* request, size_t payloadLength,
        uint8_t /* formatVersion */, pldm_tid_t tid, size_t eventDataOffset);

    /** @brief A Coroutine to do OEM PollForPlatformEvent action
     *
     *  @param[in] tid - the destination TID
     *  @return coroutine return_value - PLDM completion code
     */
    exec::task<int> oemPollForPlatformEvent(pldm_tid_t tid);

  protected:
    /** @brief Create prefix string for logging message.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *
     *  @return std::string - the prefeix string
     */
    std::string prefixMsgStrCreation(pldm_tid_t tid, uint16_t sensorId);

    /** @brief Log the message into Redfish SEL.
     *
     *  @param[in] description - the logging message
     *  @param[in] logLevel - the logging level
     */
    void sendJournalRedfish(const std::string& description,
                            log_level& logLevel);

    /** @brief Convert the one-hot DIMM index byte into a string of DIMM
     * indexes.
     *
     *  @param[in] dimmIdxs - the one-hot DIMM index byte
     *
     *  @return std::string - the string of DIMM indexes
     */
    std::string dimmIdxsToString(uint32_t dimmIdxs);

    /** @brief Convert sensor ID to DIMM index. Return maxDIMMInstantNum
     * in failure.
     *
     *  @param[in] sensorId - sensorID
     *
     *  @return uint8_t - DIMM index
     */
    uint8_t sensorIdToDIMMIdx(const uint16_t& sensorId);

    /** @brief Convert the DIMM training failure into logging string.
     *
     *  @param[in] failureInfo - the one-hot DIMM index byte
     *
     *  @return std::string - the returned logging string
     */
    std::string dimmTrainingFailureToMsg(uint32_t failureInfo);

    /** @brief Handle numeric sensor event message from PCIe hot-plug sensor.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] presentReading - the present reading of the sensor
     */
    void handlePCIeHotPlugEvent(pldm_tid_t tid, uint16_t sensorId,
                                uint32_t presentReading);

    /** @brief Handle numeric sensor event message from boot overall sensor.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] presentReading - the present reading of the sensor
     */
    void handleBootOverallEvent(pldm_tid_t /*tid*/, uint16_t /*sensorId*/,
                                uint32_t presentReading);

    /** @brief Handle numeric sensor event message from DIMM status sensor.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] presentReading - the present reading of the sensor
     */
    void handleDIMMStatusEvent(pldm_tid_t tid, uint16_t sensorId,
                               uint32_t presentReading);

    /** @brief Handle numeric sensor event message from DDR status sensor.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] presentReading - the present reading of the sensor
     */
    void handleDDRStatusEvent(pldm_tid_t tid, uint16_t sensorId,
                              uint32_t presentReading);

    /** @brief Handle numeric sensor event message from VRD status sensor.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] presentReading - the present reading of the sensor
     */
    void handleVRDStatusEvent(pldm_tid_t tid, uint16_t sensorId,
                              uint32_t presentReading);

    /** @brief Handle numeric sensor event message from Watchdog status sensor.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] presentReading - the present reading of the sensor
     */
    void handleNumericWatchdogEvent(pldm_tid_t tid, uint16_t sensorId,
                                    uint32_t presentReading);

    /** @brief Handle numeric sensor event messages.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] sensorData - the sensor data
     *  @param[in] sensorDataLength - the length of sensor data
     *
     *  @return int - returned error code
     */
    int processNumericSensorEvent(pldm_tid_t tid, uint16_t sensorId,
                                  const uint8_t* sensorData,
                                  size_t sensorDataLength);

    /** @brief Handle state sensor event messages.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] sensorData - the sensor data
     *  @param[in] sensorDataLength - the length of sensor data
     *
     *  @return int - returned error code
     */
    int processStateSensorEvent(pldm_tid_t tid, uint16_t sensorId,
                                const uint8_t* sensorData,
                                size_t sensorDataLength);

    /** @brief Handle op state sensor event messages.
     *
     *  @param[in] tid - TID
     *  @param[in] sensorId - Sensor ID
     *  @param[in] sensorData - the sensor data
     *  @param[in] sensorDataLength - the length of sensor data
     *
     *  @return int - returned error code
     */
    int processSensorOpStateEvent(pldm_tid_t tid, uint16_t sensorId,
                                  const uint8_t* sensorData,
                                  size_t sensorDataLength);

    /** @brief reference of main event loop of pldmd, primarily used to schedule
     *  work
     */
    sdeventplus::Event& event;

    /** @brief Latest OEM PollForPlatformEvent message timeStamp. */
    std::map<pldm_tid_t, uint64_t> timeStampMap;

    /** @brief A Manager interface for calling the hook functions */
    platform_mc::Manager* manager;
};
} // namespace oem_ampere
} // namespace pldm
