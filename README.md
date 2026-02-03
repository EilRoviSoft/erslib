## Using with FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    erslib
    GIT_REPOSITORY https://github.com/yourusername/erslib.git
    GIT_TAG v0.1.0  # or main for latest
)

FetchContent_MakeAvailable(erslib)

# Link to your targets
target_link_libraries(your_target PRIVATE erslib::erslib)
