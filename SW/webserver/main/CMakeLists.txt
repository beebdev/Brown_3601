idf_component_register(SRCS "keep_alive.c" "rest_server.c" "main.c" 
                            "brown_spi_slave.c"
                            
                    INCLUDE_DIRS "." "${CMAKE_CURRENT_SOURCE_DIR}/../include")

if(CONFIG_EXAMPLE_WEB_DEPLOY_SF)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front/web-demo")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
    endif()
endif()