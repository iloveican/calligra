

set(kritatooltransform_PART_SRCS
    tool_transform.cc
    tool_transform_args.cc
    tool_transform_commands.cc
    kis_tool_transform.cc
    )


kde4_add_ui_files(kritatooltransform_PART_SRCS wdg_tool_transform.ui)

kde4_add_plugin(kritatooltransform ${kritatooltransform_PART_SRCS})



target_link_libraries(kritatooltransform kritaui)

install(TARGETS kritatooltransform  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  rotate_cursor.xpm
                krita_tool_transform.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatooltransform.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
