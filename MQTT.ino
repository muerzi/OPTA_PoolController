//*************************************************
// MQTT functions
//*************************************************
/*
void startMQTT(){
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
  connectMQTT();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived ");
  //Serial.print(topic);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  //Serial.println(message);

  if (     strcmp(topic, "Pool/Pumpe/fromPump") == 0) fromPump = message;
  else if (strcmp(topic, "Pool/Pumpe/toPump") == 0)  toPump = message;
  else if (strcmp(topic, "Pool/Pumpe/fromPump2") == 0)  fromPump2 = message;
  else if (strcmp(topic, "Pool/Pumpe/toPump2") == 0)  toPump2 = message;
  else if (strcmp(topic, "Pool/UV/fromUV") == 0) fromUV = message;
  else if (strcmp(topic, "Pool/UV/toUV") == 0) toUV = message;
  else if (strcmp(topic, "Pool/UV/fromUV2") == 0) fromUV2 = message;
  else if (strcmp(topic, "Pool/UV/toUV2") == 0) toUV2 = message;
  else if (strcmp(topic, "Pool/Heizung/fromHeizung") == 0) fromHeater = message;
  else if (strcmp(topic, "Pool/Heizung/toHeizung") == 0) toHeater = message;
  else if (strcmp(topic, "Pool/Heizung/fromHeizung2") == 0) fromHeater2 = message;
  else if (strcmp(topic, "Pool/Heizung/toHeizung2") == 0) toHeater2 = message;
  else if (strcmp(topic, "Pool/Licht/fromLicht") == 0) fromLight = message;
  else if (strcmp(topic, "Pool/Licht/toLicht") == 0) toLight = message;
  else if (strcmp(topic, "Pool/Licht/fromLicht2") == 0) fromLight2 = message;
  else if (strcmp(topic, "Pool/Licht/toLicht2") == 0) toLight2 = message;
  else if (strcmp(topic, "Pool/Pumpe/PumpState") == 0) {
    if (message == "true") {
      pumpOverride = true;
      PumpOn();
    } else if (message == "false") {
      pumpOverride = false;
      PumpOff();
    }
  }
}

bool connectMQTT() {
  if (mqttClient.connect("PoolController")) {
    if(debugFlag){
      Serial.println("Connected to MQTT Broker");
    }
    mqttClient.subscribe("Pool/*");
    return true;
  } else {
    if(debugFlag){
      Serial.print("Failed to connect, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
    }
    return false;
  }
}

void publishStates() {
  // Publish IP Address
  IPAddress ip = Ethernet.localIP();
  String ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
  mqttClient.publish("Pool/allgemein/IP", ipStr.c_str());

  // Publish System Uptime
  unsigned long uptime = millis();
  unsigned long seconds = uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  char uptimeStr[50];
  sprintf(uptimeStr, "%lu days, %lu hours, %lu minutes, %lu seconds", days, hours % 24, minutes % 60, seconds % 60);
  mqttClient.publish("Pool/allgemein/UpTime", uptimeStr);

  // Optionally, you can also publish run state or other information
  // For example, publishing a simple run state message
  if(runState){
    mqttClient.publish("Pool/allgemein/runState", "Automatic");
  }else{
    mqttClient.publish("Pool/allgemein/runState", "Manual");
  }
  

}
*/