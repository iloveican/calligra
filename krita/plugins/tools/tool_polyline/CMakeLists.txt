

set(kritatoolpolyline_PART_SRCS
    tool_polyline.cc
    kis_tool_polyline.cc
    )


kde4_add_plugin(kritatoolpolyline ${kritatoolpolyline_PART_SRCS})



target_link_libraries(kritatoolpolyline kritaui)

install(TARGETS kritatoolpolyline  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_polyline_cursor.png  	polyline.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolpolyline.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


