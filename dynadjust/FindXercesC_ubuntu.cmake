#
# Find the native Xerces includes and library
#
# XERCESC_INCLUDE_DIR - where to find dom/dom.hpp, etc.
# XERCESC_LIBRARY     - List of fully qualified libraries to link against when using Xerces.
# XERCESC_FOUND       - Do not attempt to use Xerces if "no" or undefined.

SET (XERCESC_ROOT_DIR /usr)

SET (XERCESC_LIBRARY_DIR ${XERCESC_ROOT_DIR}/lib)

SET (XERCESC_INCLUDE_DIR ${XERCESC_ROOT_DIR}/include)

FIND_PATH(XERCESC_INCLUDE_DIR dom/DOM.hpp
    PATH_SUFFIXES xerces-c
)

FIND_LIBRARY(XERCESC_LIBRARY
  NAMES
    xerces-c
  PATHS
    ${XERCESC_LIBRARY_DIR}
)

message (STATUS "Xerces root directory is: ${XERCESC_ROOT_DIR}")

IF (EXISTS ${XERCESC_INCLUDE_DIR})
  IF (EXISTS ${XERCESC_LIBRARY_DIR})
    SET (XERCESC_FOUND TRUE )
  ENDIF (EXISTS ${XERCESC_LIBRARY_DIR})
ENDIF (EXISTS ${XERCESC_INCLUDE_DIR})

mark_as_advanced(XERCESC_LIBRARY)
