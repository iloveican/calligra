add_subdirectory(barcode)
add_subdirectory(chart)

if(MARBLE_FOUND)
  macro_optional_add_subdirectory(maps)
endif(MARBLE_FOUND)

if(QT_QTWEBKIT_FOUND)
  add_subdirectory(web)
endif(QT_QTWEBKIT_FOUND)
