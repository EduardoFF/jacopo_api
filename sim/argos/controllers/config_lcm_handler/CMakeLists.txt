
set(config_lcm_handler_HEADERS  
  config_lcm_handler.h
  )

set(config_lcm_handler_IMPLEMENTATIONS
  config_lcm_handler.cpp
  )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
if( ARGOS_COMPILE_REAL_SWARMANOID )
  include_directories(/usr/local/angstrom/arm/include/)
  link_directories(
    ${CMAKE_BINARY_DIR}/../../../common/swarmanoid/control_interface
    ${CMAKE_BINARY_DIR}/../../../common/swarmanoid/utility
    ${CMAKE_BINARY_DIR}/../../../real_robot/swarmanoid/common
    ${CMAKE_BINARY_DIR}/../../../real_robot/swarmanoid/vision_robot/filtering
    ${CMAKE_SOURCE_DIR}/../../real_robot/swarmanoid/opencv/lib
    ${CMAKE_BINARY_DIR}/../../../real_robot/swarmanoid/footbot
    )
  add_library(config_lcm_handler
    ${config_lcm_handler_IMPLEMENTATIONS}
    )

  target_link_libraries(config_lcm_handler 
    argos2_real_robot_swarmanoid_footbot
    argos2_real_robot_swarmanoid_common
    argos2_common_control_interface 
    argos2_common_utility
    lcm
    # any other library
    )

else( ARGOS_COMPILE_REAL_SWARMANOID )
  add_definitions(-DFOOTBOT_SIM)
  add_definitions(-fPIC)
  add_library(config_lcm_handler 
    STATIC
    ${config_lcm_handler_IMPLEMENTATIONS})
  target_link_libraries(config_lcm_handler lcm)

endif( )

#test 2



