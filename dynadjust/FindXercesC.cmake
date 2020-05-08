#
# Find the native Xerces includes and library
#
# XERCESC_INCLUDE_DIR - where to find dom/dom.hpp, etc.
# XERCESC_LIBRARY     - List of fully qualified libraries to link against when using Xerces.
# XERCESC_FOUND       - Do not attempt to use Xerces if "no" or undefined.

# SET THE ROOT DIRECTORY WHERE XERCES-C++ IS INSTALLED
IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  SET(XERCESC_ROOT_DIR /usr/local/Cellar/xerces-c/3.2.2)
ELSEIF (UNIX)
  SET (XERCESC_ROOT_DIR /opt/xerces-c/3.1.4)
ELSE ()
  SET (XERCESC_ROOT_DIR "C:/Data/xerces-c/3.1.4")
ENDIF ()

# DO NOT CHANGE
SET (XERCESC_LIBRARY_DIR ${XERCESC_ROOT_DIR}/lib)
SET (XERCESC_INCLUDE_DIR ${XERCESC_ROOT_DIR}/include)

FIND_PATH (XERCESC_INCLUDE_DIR dom/DOM.hpp
  PATH_SUFFIXES 
    xerces xerces-c
  PATHS 
    ${XERCESC_INCLUDE_DIR}
)

FIND_LIBRARY (XERCESC_LIBRARY
  NAMES
    xerces-c
  HINTS
    ${XERCESC_LIBRARY_DIR}
)

message (STATUS "Xerces root directory is: ${XERCESC_ROOT_DIR}")

IF (EXISTS ${XERCESC_INCLUDE_DIR})
  IF (EXISTS ${XERCESC_LIBRARY_DIR})
    SET (XERCESC_FOUND TRUE )
  ENDIF (EXISTS ${XERCESC_LIBRARY_DIR})
ENDIF (EXISTS ${XERCESC_INCLUDE_DIR})

mark_as_advanced(XERCESC_LIBRARY)
