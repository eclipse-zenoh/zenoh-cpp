
FetchContent_Declare(
  zenohc
  GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-c.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(zenohc)
