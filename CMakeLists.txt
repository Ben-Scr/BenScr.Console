cmake_minimum_required(VERSION 3.28)
project(BenScrConsole VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BENSCR_CONSOLE_BUILD_TESTS "Build contract tests" ${PROJECT_IS_TOP_LEVEL})
option(BENSCR_CONSOLE_BUILD_EXAMPLES "Build example" ${PROJECT_IS_TOP_LEVEL})
option(BENSCR_CONSOLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
add_library(benscr_console STATIC src/console.cpp)
add_library(BenScr::Console ALIAS benscr_console)
set_target_properties(benscr_console PROPERTIES EXPORT_NAME Console CXX_EXTENSIONS OFF)
target_compile_features(benscr_console PUBLIC cxx_std_20)
target_include_directories(benscr_console PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

function(benscr_console_checks target)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    if(BENSCR_CONSOLE_SANITIZERS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
            target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        else()
            message(FATAL_ERROR "Sanitizers require GCC or Clang on a supported POSIX host.")
        endif()
    endif()
endfunction()
benscr_console_checks(benscr_console)

if(BENSCR_CONSOLE_BUILD_TESTS)
    enable_testing()
    add_executable(console_contract_tests tests/console_tests.cpp)
    target_link_libraries(console_contract_tests PRIVATE BenScr::Console)
    benscr_console_checks(console_contract_tests)
    add_test(NAME console_contract COMMAND console_contract_tests)
    add_executable(console_header_test tests/header.cpp)
    target_link_libraries(console_header_test PRIVATE BenScr::Console)
    benscr_console_checks(console_header_test)
    add_test(NAME standalone_header COMMAND console_header_test)
    add_executable(console_redirect_test tests/redirect.cpp)
    target_link_libraries(console_redirect_test PRIVATE BenScr::Console)
    benscr_console_checks(console_redirect_test)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/redirect-input.txt" CONTENT "line\nQ")
    add_test(NAME redirected_standard_streams COMMAND ${CMAKE_COMMAND}
        -DEXECUTABLE=$<TARGET_FILE:console_redirect_test>
        -DINPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/redirect-input.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/redirect.cmake)
endif()
if(BENSCR_CONSOLE_BUILD_EXAMPLES)
    add_executable(console_example examples/console.cpp)
    target_link_libraries(console_example PRIVATE BenScr::Console)
    benscr_console_checks(console_example)
endif()

install(TARGETS benscr_console EXPORT BenScrConsoleTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES LICENSE.txt README.md DESTINATION ${CMAKE_INSTALL_DATADIR}/BenScrConsole)
configure_package_config_file(cmake/BenScrConsoleConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/BenScrConsoleConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BenScrConsole)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/BenScrConsoleConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BenScrConsoleConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BenScrConsoleConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BenScrConsole)
install(EXPORT BenScrConsoleTargets NAMESPACE BenScr:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BenScrConsole)
