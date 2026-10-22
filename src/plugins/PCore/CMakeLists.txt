project(PCore)

# Files import
file(GLOB_RECURSE PCORE_SOURCE_ALL_FILES CONFIGURE_DEPENDS
    "*.cpp"
    "*.cxx"
    "*.h"
)

set(PCORE_SRC_FILES)

source_group("src" FILES ${PCORE_SOURCE_ALL_FILES})

# Apply list
list(APPEND PCORE_SRC_FILES ${PCORE_SOURCE_ALL_FILES})

# The file system has a Windows and a POSIX implementation; only one of them may be built.
if (WIN32)
    list(FILTER PCORE_SRC_FILES EXCLUDE REGEX "xr_file_system_posix\\.(cxx|h)$")
else()
    list(FILTER PCORE_SRC_FILES EXCLUDE REGEX "xr_file_system_win32\\.(cxx|h)$")
endif()

# Remove unused files
#list(FILTER PCORE_SRC_FILES EXCLUDE REGEX "cl_raypick.cpp$")

# PCore project
add_library(PCore STATIC ${PCORE_SRC_FILES})
target_include_directories(PCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(PCore PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)