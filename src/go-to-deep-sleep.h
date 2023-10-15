void goToDeepSleepDuration(int time_seconds = 300) {
  Serial.print("Going to sleep for ");
  Serial.printf("\d", time_seconds);
  Serial.println(" seconds");

  if (logging) {
    writeFile(SPIFFS, "/error.log", "Going to sleep for 300 seconds \n");
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  adc_power_off();
  esp_wifi_stop();
  esp_bt_controller_disable();

  // Configure the timer to wake us up!
  esp_sleep_enable_timer_wakeup(time_seconds * uS_TO_S_FACTOR);
  esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);

  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}
