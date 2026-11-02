execute_process(COMMAND "${EXECUTABLE}" INPUT_FILE "${INPUT_FILE}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT result EQUAL 0 OR NOT output STREQUAL "ok\n")
    message(FATAL_ERROR "Redirected consumer failed: ${result}; ${output}; ${error}")
endif()
