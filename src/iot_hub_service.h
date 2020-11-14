#ifndef IOT_HUB_SERVICE_H
#define IOT_HUB_SERVICE_H

#include <AzureIoTHub.h>

/**
 * @brief A class that handles interaction with the Azure IoT Hub.
 */
class IoTHubService
{
public:
    /**
     * @brief Create the class, making a connection to Azure IoT Hub and waiting for commands
     */
    IoTHubService();

    /**
     * @brief Allocate some time to the IoT Hub connection to process messages coming in and out
     */
    void DoWork();
    
    /**
     * @brief Sends telemetry to the IoT Hub as telemetry
     * 
     * @param telemetry The telemetry result to send
     */
    void SendTelemetry(const char *telemetry);

private:
    IOTHUB_DEVICE_CLIENT_LL_HANDLE _device_ll_handle;
};

#endif