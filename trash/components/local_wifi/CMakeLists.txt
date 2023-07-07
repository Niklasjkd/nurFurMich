idf_component_register(SRCS "local_wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event esp_wifi mqtt protocomm)
