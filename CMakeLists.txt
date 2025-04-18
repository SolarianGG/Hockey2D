cmake_minimum_required (VERSION 3.24)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if(WIN32)
project ("HockeyMidterm" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

set(IncludeDir "${PROJECT_SOURCE_DIR}/include")
set(SrcDir "${PROJECT_SOURCE_DIR}/src")

add_executable (HockeyMidtermClient WIN32 
  "${SrcDir}/WindowsProject1.cpp"
  "${SrcDir}/KTileManager.cpp"
  "${SrcDir}/mpr_timer.cpp"
)
target_link_libraries(HockeyMidtermClient PRIVATE winmm gdiplus)
target_include_directories(HockeyMidtermClient PRIVATE "${IncludeDir}/" "${PROJECT_SOURCE_DIR}/external/")
# TODO: Add tests and install targets if needed.
endif(WIN32)
