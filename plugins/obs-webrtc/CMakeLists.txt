cmake_minimum_required(VERSION 3.28...3.30)

option(ENABLE_WEBRTC "Enable WebRTC Output support" ON)
if(NOT ENABLE_WEBRTC)
  target_disable(obs-webrtc)
  return()
endif()

find_package(LibDataChannel 0.20 REQUIRED)
find_package(CURL REQUIRED)

add_library(obs-webrtc MODULE)
add_library(OBS::webrtc ALIAS obs-webrtc)

target_sources(
  obs-webrtc
  PRIVATE obs-webrtc.cpp whip-output.cpp whip-output.h whip-service.cpp whip-service.h whip-utils.h
)

target_link_libraries(obs-webrtc PRIVATE OBS::libobs LibDataChannel::LibDataChannel CURL::libcurl)

# OpenVibe requires WebRTC/WHIP on Linux. Upstream OBS disables WebRTC in
# Ubuntu CI, so we build LibDataChannel ourselves and bundle its runtime library
# into the DEB.
if(OS_LINUX)
  set(
    LIBDATACHANNEL_BUNDLE_DIR
    ""
    CACHE PATH
    "Directory containing libdatachannel.so files to bundle in Linux packages"
  )

  if(LIBDATACHANNEL_BUNDLE_DIR)
    if(NOT EXISTS "${LIBDATACHANNEL_BUNDLE_DIR}/libdatachannel.so")
      message(
        FATAL_ERROR
        "LIBDATACHANNEL_BUNDLE_DIR does not contain libdatachannel.so: ${LIBDATACHANNEL_BUNDLE_DIR}"
      )
    endif()

    install(
      DIRECTORY "${LIBDATACHANNEL_BUNDLE_DIR}/"
      DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      FILES_MATCHING
      PATTERN "libdatachannel.so*"
    )
  endif()
endif()

if(OS_WINDOWS)
  configure_file(cmake/windows/obs-module.rc.in obs-webrtc.rc)
  target_sources(obs-webrtc PRIVATE obs-webrtc.rc)
endif()

set_target_properties_obs(obs-webrtc PROPERTIES FOLDER plugins PREFIX "")
