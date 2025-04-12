idf_component_register(
    SRCS "firebase_client.c"
    INCLUDE_DIRS "."
    REQUIRES esp_http_client esp_timer
)
