void mqttSendConfig(String identyfikator, String name, Config config, String chipId, String uom = "", String dc = "", String icon = "")
{
  bool print_info = false;

  const String topicStr_c = mqttPrefix + plant_name + "/" + identyfikator + "/config";
  const char *topic_c = topicStr_c.c_str();

  StaticJsonDocument<1000> doc_c;
  JsonObject root = doc_c.to<JsonObject>();

  JsonObject device = doc_c.createNestedObject("device");
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(chipId);

  // Name of the device
  device["name"] = device_name;
  device["manufacturer"] = "LILYGO";
  device["model"] = "TTGO T-Higrow";
  device["sw_version"] = config.releaseVersion;

  // Name of the sensor
  root["name"] = name;
  root["object_id"] = plant_name + " " + identyfikator;
  root["retain"] = "true";
  root["unique_id"] = chipId + "-" + identyfikator;
  root["state_topic"] = mqttPrefix + plant_name + "/state";
  root["value_template"] = "{{ value_json." + identyfikator + " }}";

  // If we want online and offline messages
  // root["availability_topic"] = mqttprefix + plant_name + "/availability";

  if (icon.isEmpty() == false)
  {
    root["icon"] = icon;
  }

  if (dc.isEmpty() == false)
  {
    root["device_class"] = dc;
  }

  if (uom.isEmpty() == false)
  {
    root["unit_of_measurement"] = uom;
  }

  // Nice print of configuration mqtt message
  if (print_info)
  {
    Serial.println();
    Serial.println("*****************************************");
    Serial.println(topic_c);
    Serial.print("Sending message to config: \n");
    serializeJsonPretty(doc_c, Serial);
    Serial.println("*****************************************");
    Serial.println();
  }

  // Send to mqtt
  size_t n = measureJson(doc_c) + 1;
  char buffer_c[n];

  // Print how big the json is
  if (print_info)
  {
    Serial.print("Size of json: ");
    Serial.println(n);
  }

  serializeJson(doc_c, buffer_c, n);

  if (logging)
  {
    writeFile(SPIFFS, "/error.log", "Sending message to topic: \n");
  }

  bool retained = true;

  Serial.print("Publishing message to config: \n");

  if (mqttClient.publish(topic_c, buffer_c, retained))
  {
    if (print_info)
    {
      Serial.println("Message published successfully");
    }
  }
  else
  {
    if (print_info)
    {
      Serial.println("Error in Message, not published");
    }

    goToDeepSleepFiveMinutes();
  }

  if (print_info)
  {
    Serial.println("*****************************************\n");
  }
}

// Allocate a JsonDocument
void saveConfiguration(const Config &config)
{
  bool print_info = false;
  //  Serial.println(WiFi.macAddress());
  //  String stringMAC = WiFi.macAddress();
  //  stringMAC.replace(':', '_');

  byte mac[6];
  WiFi.macAddress(mac);

  //  String chipId = String(mac[0], HEX) + String(mac[1], HEX) + String(mac[2], HEX) + String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  String chipId = "";
  String HEXcheck = "";
  for (int i = 0; i <= 5; i++)
  {
    HEXcheck = String(mac[i], HEX);
    if (HEXcheck.length() == 1)
    {
      chipId = chipId + "0" + String(mac[i], HEX);
    }
    else
    {
      chipId = chipId + String(mac[i], HEX);
    }
  }
  // Serial.println("chipId " + chipId);

  // Connect to mqtt broker
  Serial.print("Attempting to connect to the MQTT broker: ");
  if (logging)
  {
    writeFile(SPIFFS, "/error.log", "Attempting to connect to the MQTT broker! \n");
  }

  Serial.println(broker);
  mqttClient.setServer(broker, port);

  if (!mqttClient.connect(broker, mqttuser, mqttpass))
  {
    if (logging)
    {
      writeFile(SPIFFS, "/error.log", "MQTT connection failed! \n");
    }

    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.state());
    goToDeepSleepFiveMinutes();
  }

  if (logging)
  {
    writeFile(SPIFFS, "/error.log", "You're connected to the MQTT broker! \n");
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println("\n");

  // https://www.home-assistant.io/integrations/sensor/#device-class
  // Home Assitant MQTT Autodiscovery mesasaages

  const String topicStr = mqttPrefix + plant_name + "/state";
  const char *topic = topicStr.c_str();

  StaticJsonDocument<1000> doc;
  // Set the values in the document
  // Device changes according to device placement
  JsonObject plant = doc.to<JsonObject>();

  // mqttSetup("time", chipId, "", "", "mdi:clock");
  // plant["time"] = config.time;

  // Raw voltage
  // mqttSetup("battvolt", "Battery Voltage", chipId, "V", "voltage", "mdi:battery");
  // plant["battvolt"] = config.batvolt; // nie

  // True voltage
  mqttSendConfig("battVoltage", "Battery Voltage", config, chipId, "V", "voltage", "mdi:battery");
  plant["battVoltage"] = config.batVoltage;

  mqttSendConfig("batPercentage", "Battery", config, chipId, "%", "", "mdi:battery");
  plant["batPercentage"] = config.batPercentage;

  mqttSendConfig("sleep5Count", "Sleep Count", config, chipId, "", "", "mdi:counter");
  plant["sleep5Count"] = sleep5No;

  mqttSendConfig("bootCount", "Boot Count", config, chipId, "", "", "mdi:counter");
  plant["bootCount"] = config.bootNo;

  // mqttSetup("device_name", "Device Name", config, chipId);
  // plant["device_name"] = device_name;

  mqttSendConfig("chipId", "Mac Address", config, chipId);
  plant["chipId"] = chipId;

  // mqttSetup("sensorName", "Sensor Name", config, chipId);
  // plant["sensorName"] = plant_name;

  // mqttSetup("date", chipId, "", "date", "mdi:calendar");
  // plant["date"] = config.date;

  mqttSendConfig("daysOnBattery", "Days On Battery", config, chipId, "", "", "mdi:calendar-clock");
  plant["daysOnBattery"] = config.daysOnBattery;

  mqttSendConfig("batChargeDate", "Last Charge Date", config, chipId, "", "date", "mdi:calendar-clock");
  plant["batChargeDate"] = config.batChargeDate;

  mqttSendConfig("batStatus", "Battery Status", config, chipId);
  plant["batStatus"] = config.batStatus; // nie

  mqttSendConfig("lux", "Lux", config, chipId, "lx", "illuminance", "mdi:white-balance-sunny");
  plant["lux"] = config.lux;

  mqttSendConfig("temp", "Ambient Temperature", config, chipId, "°C", "temperature", "mdi:thermometer"); // nie
  plant["temp"] = config.temp;

  mqttSendConfig("humid", "Ambient Humidity", config, chipId, "%", "humidity", "mdi:water-percent");
  plant["humid"] = config.humid;

  mqttSendConfig("pressure", "Pressure", config, chipId, "hPa", "pressure", "mdi:gauge");
  plant["pressure"] = config.pressure;

  mqttSendConfig("soilHumidity", "Soil Humidity", config, chipId, "%", "humidity", "mdi:water-percent");
  plant["soilHumidity"] = config.soilHumidity;

  mqttSendConfig("soilTemp", "Soil Temperature", config, chipId, "°C", "temperature", "mdi:thermometer");
  plant["soilTemp"] = config.soilTemp; // nie

  mqttSendConfig("fertilizer", "Fertilizer", config, chipId, "", "", "mdi:food");
  plant["fertilizer"] = config.fertilizer;

  mqttSendConfig("fertilizerAdvice", "Fertilizer Advice", config, chipId, "", "", "mdi:food");
  plant["fertilizerAdvice"] = config.fertilizerAdvice; // nie

  // plant["plantValveNo"] = plantValveNo; //nie
  // plant["wifissid"] = WiFi.SSID(); //nie

  // mqttSetup("rel", chipId);
  // plant["rel"] = config.rel;

  // plant["millis"] = millis() - setupstart;

  mqttSendConfig("RSSI", "RSSI", config, chipId, "dBm", "signal_strength", "mdi:wifi");
  plant["RSSI"] = WiFi.RSSI(); // wifiRSSI;

  size_t n = measureJson(doc) + 1;

  // Print how big the json is
  if (print_info)
  {
    Serial.print("Size of json: ");
    Serial.println(n);
    Serial.println();
    serializeJsonPretty(doc, Serial);
    Serial.println();
  }

  // Send to mqtt
  char buffer[n];
  serializeJson(doc, buffer, n);

  Serial.print("Sending message to state: ");
  Serial.println(topic);
  if (logging)
  {
    writeFile(SPIFFS, "/error.log", "Sending message to state: \n");
  }

  bool retained = true;

  if (mqttClient.publish(topic, buffer, retained))
  {
    Serial.println("Message published successfully");
  }
  else
  {
    Serial.println("Error in Message, not published");
    goToDeepSleepFiveMinutes();
  }
  Serial.println();
}
