#include <WiFi.h>
#include <AsyncTCP.h>

// extern "C" {
// #include <osapi.h>
// #include <os_type.h>
// }
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#include "config.h"

static TimerHandle_t wifiReconnectTimer;

// static os_timer_t intervalTimer;
static TimerHandle_t tcpReconnectTimer;
static AsyncClient client;

void replyToServer(AsyncClient *client, const String &msg)
{
    const unsigned int len = msg.length();
    // send reply
    if (client->space() > len && client->canSend())
    {
        Serial.printf("\n reply to server %s on port %d : \n %s", client->remoteIP().toString().c_str(), TCP_PORT, msg.c_str());

        // char message[32];
        // sprintf(message, "this is from %s", WiFi.localIP().toString().c_str());
        // client->add(message, strlen(message));
        client->add(msg.c_str(), len);
        client->send();
    }
}

/* event callbacks */
void onConnect(void *arg, AsyncClient *client)
{
    Serial.printf("\n client has been connected to %s on port %d \n", SERVER_HOST_NAME, TCP_PORT);

    String cmdSubscribeTopic = "cmd=1&uid=";
    cmdSubscribeTopic += UID;
    cmdSubscribeTopic += "&topic=led001\r\n"; // 构建订阅指令

    replyToServer(client, cmdSubscribeTopic);
}

void onDisconnect(void *arg, AsyncClient *client)
{
    Serial.printf("\n client has been disconnected from %s on port %d \n", SERVER_HOST_NAME, TCP_PORT);

    // 重连
    if (WiFi.isConnected())
    {
        xTimerStart(tcpReconnectTimer, 0);
    }
}

void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
    Serial.printf("\n data received from %s on port %d : \n ", client->remoteIP().toString().c_str(), TCP_PORT);
    Serial.write((uint8_t *)data, len);

    // os_timer_arm(&intervalTimer, 2000, true); // schedule for reply to server at next 2s
}

void handlePacket(void *arg, AsyncClient *client, pbuf *pb)
{
    Serial.printf("\n packet received from %s on port %d : \n", client->remoteIP().toString().c_str(), TCP_PORT);
    void *data = pb->payload;
    size_t len = pb->len;

    Serial.write((uint8_t *)data, len);
}

void onPoll(void *arg, AsyncClient *client)
{
    Serial.printf("\n client has been polled from %s on port %d \n", client->remoteIP().toString().c_str(), TCP_PORT);
}

void onAck(void *arg, AsyncClient *client, size_t len, uint32_t time)
{
    Serial.printf("\n client ACK received from %s on port %d \n", client->remoteIP().toString().c_str(), TCP_PORT);
}

void onTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
    Serial.printf("\n client ACK timeout from %s on port %d \n", client->remoteIP().toString().c_str(), TCP_PORT);
}

void onError(void *arg, AsyncClient *client, int8_t error)
{
    Serial.printf("\n client error %d from %s on port %d \n", error, client->remoteIP().toString().c_str(), TCP_PORT);
}

void onWiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToTcp();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(tcpReconnectTimer, 0); // ensure we don't reconnect to TCP while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void connectToWifi()
{
    Serial.println(" Connecting to Wi-Fi...");
    // connects to access point
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //     Serial.print('.');
    //     delay(500);
    // }
    // Serial.println('.');
}

void connectToTcp()
{
    Serial.println(" Connecting to TCP...");

    client.connect(SERVER_HOST_NAME, TCP_PORT);
    // while (!client.connected() || !client.canSend())
    // {
    //     Serial.print('.');
    //     delay(500);
    // }
    // Serial.println('.');
}

void setup()
{
    Serial.begin(115200);

    // os_timer_disarm(&intervalTimer);
    // os_timer_setfn(&intervalTimer, &replyToServer, client);
    tcpReconnectTimer = xTimerCreate("tcpTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToTcp));
    // xTimerStop(tcpReconnectTimer, 0);
    client.onData(&handleData, &client);
    // client.onPacket(&handlePacket, client);
    client.onDisconnect(&onDisconnect, &client);
    client.onConnect(&onConnect, &client);
    // client.onPoll(&onPoll, client);
    client.onAck(&onAck, &client);
    client.onTimeout(&onTimeOut, &client);
    client.onError(&onError, &client);

    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
    // xTimerStop(wifiReconnectTimer, 0);
    WiFi.onEvent(onWiFiEvent);
    connectToWifi();
}

void loop()
{
}
