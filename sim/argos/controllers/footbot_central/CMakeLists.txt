include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-fPIC)
add_library(footbot_central_controller MODULE
  footbot_central.cpp
)

find_package(LCM REQUIRED)
target_link_libraries(footbot_central_controller
  ${LCM_LIBRARY}
  navigation
  #argos2_simulator_dynamic_linking
  )
