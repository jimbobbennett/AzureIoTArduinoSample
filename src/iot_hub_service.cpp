#include "iot_hub_service.h"
#include "iot_config.h"

#include <Arduino.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTSocket_WiFi.h>
#include <iothubtransportmqtt.h>
#include <WiFiClientSecure.h>

const IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol = MQTT_Protocol;

static void ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
{
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        LogInfo("The device client is connected to iothub\r\n");
    }
    else
    {
        LogInfo("The device client has been disconnected\r\n");
    }
}

// A helper method to build a response in the format that IoT Hub expects
static void BuildResult(const char *result, unsigned char **response, size_t *response_size)
{
    // Create a JSON document with the classificaiton result
    char resultBuff[128];
    sprintf(resultBuff, "{\"Result\":\"%s\"}", result);

    // Set the size of the response
    *response_size = strlen(resultBuff);

    // Allocate memory for the response, and copy the values into the allocated memory
    *response = (unsigned char *)malloc(*response_size);
    memcpy(*response, resultBuff, *response_size);
}

static char *ConvertPayload(const unsigned char *payload, size_t size)
{
    char *payloadZeroTerminated = (char *)malloc(size + 1);

    memcpy(payloadZeroTerminated, payload, size);
    payloadZeroTerminated[size] = '\0';

    return payloadZeroTerminated;
}

// A callback used when the IoT Hub invokes a direct method
// This is a static method as opposed to a method on the class so it can be pass to the
// IoT hub configuration
static int DirectMethodCallback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
    // Log the direct method received
    Serial.printf("Direct method received %s\r\n", method_name);

    char *converted_payload = ConvertPayload(payload, size);

    Serial.printf("Direct method body %s\r\n", converted_payload);

    // The userContextCallback is the IoT Hub Service, so cast it so it can be used
    IoTHubService *iotHubService = (IoTHubService *)userContextCallback;

    // We only support the Command method, any other method calls return an error status
    if (strcmp(method_name, "Command") == 0)
    {
        // Do something with the direct method

        // Return the result
        BuildResult("", response, response_size);
        return IOTHUB_CLIENT_OK;
    }
    else
    {
        BuildResult("Method is not supported", response, response_size);
        return IOTHUB_CLIENT_ERROR;
    }
}

// A callback used when the IoT Hub updtes the device twin
// This is a static method as opposed to a method on the class so it can be pass to the
// IoT hub configuration
static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char *payload, size_t size, void *userContextCallback)
{
    char *converted_payload = ConvertPayload(payload, size);

    // Log the direct method received
    Serial.printf("Device twin update received %s\r\n", converted_payload);

    // The userContextCallback is the IoT Hub Service, so cast it so it can be used
    IoTHubService *iotHubService = (IoTHubService *)userContextCallback;
}

static IOTHUBMESSAGE_DISPOSITION_RESULT CloudToDeviceMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    try
    {
        Serial.print("message callback");

        const char *message_text = IoTHubMessage_GetString(message);

        // Log the message received
        Serial.printf("Cloud to device message received %s\r\n", message_text);

        // The userContextCallback is the IoT Hub Service, so cast it so it can be used
        IoTHubService *iotHubService = (IoTHubService *)userContextCallback;

        return IOTHUBMESSAGE_ACCEPTED;
    }
    catch (...)
    {
        Serial.print("Exception");        
    }
}

IoTHubService::IoTHubService()
{
    // Used to initialize IoTHub SDK subsystem
    IoTHub_Init();

    // Create the client from the connection string
    _device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(DEVICE_CONNECTION_STRING, protocol);

    Serial.println("Connected to IoT Hub!");

    // If the client connection fails, report an error and restart the device
    if (_device_ll_handle == NULL)
    {
        Serial.println("Failure creating Iothub device. Hint: Check your connection string.");
        ESP.restart();
    }

    // Set any options that are necessary.
    // For available options please see the iothub_sdk_options.md documentation in the main C SDK
    // turn off diagnostic sampling
    int diag_off = 1;
    IoTHubDeviceClient_LL_SetOption(_device_ll_handle, OPTION_DIAGNOSTIC_SAMPLING_PERCENTAGE, &diag_off);

    bool traceOn = true;
    IoTHubDeviceClient_LL_SetOption(_device_ll_handle, OPTION_LOG_TRACE, &traceOn);

    // Setting the Trusted Certificate.
    IoTHubDeviceClient_LL_SetOption(_device_ll_handle, OPTION_TRUSTED_CERT, certificates);

    //Setting the auto URL Encoder (recommended for MQTT). Please use this option unless
    //you are URL Encoding inputs yourself.
    //ONLY valid for use with MQTT
    bool urlEncodeOn = true;
    IoTHubDeviceClient_LL_SetOption(_device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);

    // Setup callbacks for calls from IoT Hub

    // Set the connection status callback
    IoTHubDeviceClient_LL_SetConnectionStatusCallback(_device_ll_handle, ConnectionStatusCallback, this);

    // Setting method call back, so we can receive direct methods.
    IoTHubClient_LL_SetDeviceMethodCallback(_device_ll_handle, DirectMethodCallback, this);

    // Set the device twin callback
    IoTHubClient_LL_SetDeviceTwinCallback(_device_ll_handle, DeviceTwinCallback, this);

    // Set the cloud to device message callback
    //IoTHubClient_LL_SetMessageCallback(_device_ll_handle, CloudToDeviceMessageCallback, this);
}

// Let the IoT Hub do whatever work is needed to send or receive messages
void IoTHubService::DoWork()
{
    // Let the IoT Hub do whatever work is needed to send or receive messages
    IoTHubDeviceClient_LL_DoWork(_device_ll_handle);
}

// Send telemetry to IoT Hub
void IoTHubService::SendTelemetry(const char *telemetry)
{
    IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromString(telemetry);

    Serial.printf("Sending message %s to IoTHub\r\n", telemetry);

    IoTHubDeviceClient_LL_SendEventAsync(_device_ll_handle, message_handle, NULL, NULL);

    IoTHubMessage_Destroy(message_handle);
}
