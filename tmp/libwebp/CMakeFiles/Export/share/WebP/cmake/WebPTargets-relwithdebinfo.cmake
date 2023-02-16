#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "WebP::sharpyuv" for configuration "RelWithDebInfo"
set_property(TARGET WebP::sharpyuv APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WebP::sharpyuv PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libsharpyuv.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS WebP::sharpyuv )
list(APPEND _IMPORT_CHECK_FILES_FOR_WebP::sharpyuv "${_IMPORT_PREFIX}/lib/libsharpyuv.lib" )

# Import target "WebP::webpdecoder" for configuration "RelWithDebInfo"
set_property(TARGET WebP::webpdecoder APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WebP::webpdecoder PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libwebpdecoder.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS WebP::webpdecoder )
list(APPEND _IMPORT_CHECK_FILES_FOR_WebP::webpdecoder "${_IMPORT_PREFIX}/lib/libwebpdecoder.lib" )

# Import target "WebP::webp" for configuration "RelWithDebInfo"
set_property(TARGET WebP::webp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WebP::webp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libwebp.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS WebP::webp )
list(APPEND _IMPORT_CHECK_FILES_FOR_WebP::webp "${_IMPORT_PREFIX}/lib/libwebp.lib" )

# Import target "WebP::webpdemux" for configuration "RelWithDebInfo"
set_property(TARGET WebP::webpdemux APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WebP::webpdemux PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libwebpdemux.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS WebP::webpdemux )
list(APPEND _IMPORT_CHECK_FILES_FOR_WebP::webpdemux "${_IMPORT_PREFIX}/lib/libwebpdemux.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
