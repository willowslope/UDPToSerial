#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//Sleep Timer 設定時間:30分
#define SLEEP_TIMER (unsigned long)(30*60*1000)
unsigned long time_old = 0;

const char *ssid = "ESP_OBDII";
const char *password = "";

WiFiUDP udp;                       //udpサーバー
#define UDP_PORT 10000
#define MAX_CLIENTS 10
IPAddress clientIPs[MAX_CLIENTS]; //返信先
unsigned int clientNum=0;

void setup() {
  //WiFi設定
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 0, 10), IPAddress(192, 168, 0, 10), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  //シリアルポート設定
  Serial.begin(38400);
  //UDP通信開始
  udp.begin(UDP_PORT);
}

void loop() {
  //長時間アクセスが無いときはスリープする
  if ((unsigned long)(millis()-time_old)>SLEEP_TIMER){
    ESP.deepSleep(0); 
  }

  //UDP -> Serial
  int rlen= udp.parsePacket();
  if(rlen>0){
    //UDP受信
    uint8_t rbuf[rlen];
    udp.read(rbuf, rlen);
    //データ送信してきたIPを登録
    registIP(udp.remoteIP());
    //Serial送信
    Serial.write(rbuf,rlen);
    time_old = millis();
  }

  //Seral -> UDP
  if(Serial.available()){
    size_t len = Serial.available();
    //Serial受信
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //UDP送信
    for(int i=0;i<clientNum;i++){
      if(udp.beginPacket(clientIPs[i],UDP_PORT)){
          udp.write(sbuf,len);
          udp.endPacket();
      }
    }
  }
}

// 返信先のIPアドレスを登録
void registIP(IPAddress IP){
  if(clientNum>=MAX_CLIENTS) return;
  if(IP == IPAddress(0,0,0,0)) return;
  for(int i=0;i<clientNum;i++){
    if(clientIPs[i]==IP){
      return;
    }
  }
  clientIPs[clientNum]=IP;
  clientNum++;
}

