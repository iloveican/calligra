include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

SET (pictureshape_LIB_SRCS
    PictureShape.cpp
    PictureShapeFactory.cpp
    PictureTool.cpp
    PictureToolFactory.cpp
    Plugin.cpp
    PictureShapeConfigWidget.cpp
    ChangeImageCommand.cpp
    ClipCommand.cpp
    CropWidget.cpp
    SelectionRect.cpp
    ClippingRect.cpp
    filters/GreyscaleFilterEffect.cpp
    filters/MonoFilterEffect.cpp
    filters/WatermarkFilterEffect.cpp
)

kde4_add_ui_files(pictureshape_LIB_SRCS forms/wdgPictureTool.ui)
kde4_add_plugin(pictureshape ${pictureshape_LIB_SRCS})

target_link_libraries(pictureshape komain flake ${KDE4_KFILE_LIBRARY})
install(TARGETS pictureshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES pictureshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
