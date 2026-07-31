add_executable(UnixKeyTest)

target_sources(UnixKeyTest
  PRIVATE
    test.cpp
    ../substitution/matcher.cpp
    ../substitution/config.cpp
)

target_link_libraries(UnixKeyTest PRIVATE Fcitx5::Core)
target_link_libraries(UnixKeyTest PRIVATE ICU::uc ICU::io)
