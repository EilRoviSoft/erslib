## Using with FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    erslib
    GIT_REPOSITORY https://github.com/EilRoviSoft/erslib.git
    GIT_TAG main
)

FetchContent_MakeAvailable(erslib)

target_link_libraries(<your_target> PRIVATE erslib::erslib)
```
