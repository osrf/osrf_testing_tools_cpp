add_executable(test_runner main.cpp)

install(TARGETS test_runner
  DESTINATION lib/${PROJECT_NAME}
)
