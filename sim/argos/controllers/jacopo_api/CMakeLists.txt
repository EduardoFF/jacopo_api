
set(footbot_jacopo_api_HEADERS  
  footbot_jacopo_api.h
  )

set(footbot_jacopo_api_IMPLEMENTATIONS
  footbot_jacopo_api.cc
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/controllers/config_lcm_handler)
include_directories( ${CMAKE_SOURCE_DIR}/controllers)

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
  add_executable(footbot_jacopo_api
    ${footbot_jacopo_api_IMPLEMENTATIONS}
    ${CMAKE_SOURCE_DIR}/main_functions/real_footbot_main.cpp
    )

  target_link_libraries(footbot_jacopo_api 
    argos2_real_robot_swarmanoid_footbot
    argos2_real_robot_swarmanoid_common
    argos2_common_control_interface 
    argos2_common_utility
    lcm
    navigation
    config_lcm_handler
    )

else( ARGOS_COMPILE_REAL_SWARMANOID )
  add_definitions(-DFOOTBOT_SIM)
  add_library(footbot_jacopo_api 
    MODULE 
    ${footbot_jacopo_api_IMPLEMENTATIONS})
  target_link_libraries(footbot_jacopo_api lcm navigation config_lcm_handler)
endif( ARGOS_COMPILE_REAL_SWARMANOID  )




