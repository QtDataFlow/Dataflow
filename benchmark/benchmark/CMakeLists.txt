project(dataflow_benchmark)

add_executable(${PROJECT_NAME}
  check_point.cpp
  check_point.h
  check_points.cpp
  check_points.h
  main.cpp
  )

target_link_libraries(${PROJECT_NAME} dataflow++)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

