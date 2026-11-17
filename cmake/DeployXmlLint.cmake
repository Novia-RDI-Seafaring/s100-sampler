# Deploy just xmllint and the non-system DLLs it loads, for both builds and installs.
cmake_minimum_required(VERSION 3.21)
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
if(NOT DEFINED S100_XMLLINT OR NOT DEFINED S100_DESTINATION)
  message(FATAL_ERROR "S100_XMLLINT and S100_DESTINATION are required")
endif()

file(MAKE_DIRECTORY "${S100_DESTINATION}")
file(COPY "${S100_XMLLINT}" DESTINATION "${S100_DESTINATION}")
if(WIN32 OR CMAKE_HOST_WIN32)
  if(S100_DUMPBIN)
    set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL dumpbin)
    set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND "${S100_DUMPBIN}")
  endif()
  get_filename_component(_tool_directory "${S100_XMLLINT}" DIRECTORY)
  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${S100_XMLLINT}"
    DIRECTORIES "${_tool_directory}" "${S100_DEPENDENCY_BIN}"
    PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
    POST_EXCLUDE_REGEXES ".*[Ss][Yy][Ss][Tt][Ee][Mm]32[/\\\\].*\\.dll"
    RESOLVED_DEPENDENCIES_VAR _dependencies
    UNRESOLVED_DEPENDENCIES_VAR _missing
  )
  if(_missing)
    message(FATAL_ERROR "xmllint dependencies could not be resolved: ${_missing}")
  endif()
  foreach(_dependency IN LISTS _dependencies)
    file(COPY "${_dependency}" DESTINATION "${S100_DESTINATION}")
  endforeach()
endif()
