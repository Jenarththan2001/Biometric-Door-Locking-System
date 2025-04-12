idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       EMBED_TXTFILES "firebase_cert.pem"
                       REQUIRES oled_driver 
                                 keypad_driver 
                                 as608_driver 
                                 ds1307_driver 
                                 wifi_connect 
                                 esp_wifi 
                                 esp_event 
                                 nvs_flash 
                                 esp_netif 
                                 lwip 
                                 esp_http_client 
                                 esp_timer)
