#
# Find the native Xerces includes and library
#
# XSD_INCLUDE_DIR - where to find dom/dom.hpp, etc.
# XSD_FOUND       - Do not attempt to use Xerces if "no" or undefined.

# SET THE ROOT DIRECTORY WHERE XERCES-C++ IS INSTALLED

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(XSD_INCLUDE_DIR /usr/local/Cellar/xsd/4.0.0_1/include)
ELSEIF (UNIX)
  SET (XSD_INCLUDE_DIR /opt/xsd/xsd-4.0.0-x86_64-linux-gnu/libxsd/)
ELSE ()
  SET (XSD_INCLUDE_DIR "C:/Program Files (x86)/CodeSynthesis XSD 4.0/include")
ENDIF ()  

FIND_PATH(XSD_INCLUDE_DIR xsd/cxx/config.hxx
    PATHS "${XSD_INCLUDE_DIR}")

message (STATUS "XSD root directory is: ${XSD_INCLUDE_DIR}")

IF (EXISTS ${XSD_INCLUDE_DIR})
    SET (XSD_FOUND TRUE )
ENDIF (EXISTS ${XSD_INCLUDE_DIR})
