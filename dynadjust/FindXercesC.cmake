#
# Find the native Xerces includes and library
#
# XERCESC_INCLUDE_DIR - where to find dom/dom.hpp, etc.
# XERCESC_LIBRARY     - List of fully qualified libraries to link against when using Xerces.
# XERCESC_FOUND       - Do not attempt to use Xerces if "no" or undefined.


FIND_PATH(XERCESC_INCLUDE_DIR dom/DOM.hpp
	PATH_SUFFIXES xercesc xerces-c 
        PATHS /opt/xerces-c/3.1.4/include
)
get_filename_component(XERCESC_ROOT_DIR ${XERCESC_INCLUDE_DIR} DIRECTORY)

FIND_LIBRARY(XERCESC_LIBRARY
  NAMES
    xerces-c
  HINTS
    "${XERCESC_ROOT_DIR}/lib"
)

IF(XERCESC_INCLUDE_DIR)
  IF(XERCESC_LIBRARY)
    SET( XercesC_LIBRARIES "${XERCES_LIBRARY}" )
    SET( XercesC_LIBRARY "${XERCES_LIBRARY}" )
    SET( XercesC_INCLUDE_DIRS "${XERCES_INCLUDE_DIR}" )
    SET( XERCESC_FOUND "YES" )
  ENDIF(XERCESC_LIBRARY)
ENDIF(XERCESC_INCLUDE_DIR)

mark_as_advanced(XERCESC_LIBRARY)
