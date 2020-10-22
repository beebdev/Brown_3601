idf_component_register(SRCS "main.c" 
                            "brown_mqtt_client.c"
                            "brown_spi_slave.c"
                            "brown_webserver.c"
                    INCLUDE_DIRS "." "${CMAKE_CURRENT_SOURCE_DIR}/../include")