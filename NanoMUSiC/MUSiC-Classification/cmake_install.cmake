# Install script for directory: /.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Classification

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.37-355ed/x86_64-centos7/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin" TYPE EXECUTABLE FILES "/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Classification/classification")
  if(EXISTS "$ENV{DESTDIR}/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification"
         OLD_RPATH "/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.37-355ed/x86_64-centos7/bin/strip" "$ENV{DESTDIR}/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/bin/classification")
    endif()
  endif()
endif()

