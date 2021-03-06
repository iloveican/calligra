# The following variables should be set: KOPROPERTY_TARGETNAME, KOPROPERTY_LIBS, KOPROPERTY_LIB_VERSION, KOPROPERTY_LIB_SOVERSION.
# Set KOPROPERTY_USE_KOLIBS to TRUE if extra features that use Calligra libraries should be compiled-in.
# See calligra/kexi/CMakeLists.txt for example.
# TODO remove when we move to independent place

add_subdirectory( test )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44022)

set(libkopropertyeditors_SRCS
  editors/utils.cpp
  editors/booledit.cpp
  editors/coloredit.cpp
  editors/combobox.cpp
  editors/cursoredit.cpp
#  editors/dateedit.cpp
#  editors/datetimeedit.cpp
#  editors/dummywidget.cpp
  editors/fontedit.cpp
  editors/pixmapedit.cpp
  editors/pointedit.cpp
  editors/pointfedit.cpp
  editors/rectedit.cpp
  editors/sizeedit.cpp
  editors/sizefedit.cpp
  editors/sizepolicyedit.cpp
  editors/spinbox.cpp
  editors/stringedit.cpp
#  editors/stringlistedit.cpp
#  editors/symbolcombo.cpp
#  editors/timeedit.cpp
#  editors/urledit.cpp
)

if(KOPROPERTY_USE_KOLIBS) # TODO remove when we move to independent place
  add_definitions(-DKOPROPERTY_USE_KOLIBS)
  include_directories(
    ${KOPROPERTY_INCLUDES}
  )
  list(APPEND libkopropertyeditors_SRCS editors/linestyleedit.cpp)
else(KOPROPERTY_USE_KOLIBS)
  include_directories(
    ${KDE4_INCLUDES}
  )
endif(KOPROPERTY_USE_KOLIBS)

########### next target ###############

set(koproperty_LIB_SRCS
   Property.cpp
   Set.cpp
   Factory.cpp
   DefaultFactory.cpp
   EditorView.cpp
   EditorDataModel.cpp
   Utils.cpp
   ${libkopropertyeditors_SRCS}
)


kde4_add_library(${KOPROPERTY_TARGETNAME} SHARED ${koproperty_LIB_SRCS})

set(KOPROPERTY_LINK_LIBS ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
if(KOPROPERTY_USE_KOLIBS)
	list(APPEND KOPROPERTY_LINK_LIBS kowidgets)
endif(KOPROPERTY_USE_KOLIBS)
target_link_libraries(${KOPROPERTY_TARGETNAME} ${KOPROPERTY_LINK_LIBS})

target_link_libraries(${KOPROPERTY_TARGETNAME} LINK_INTERFACE_LIBRARIES ${KOPROPERTY_LINK_LIBS})

set_target_properties(${KOPROPERTY_TARGETNAME} PROPERTIES VERSION ${KOPROPERTY_LIB_VERSION} SOVERSION ${KOPROPERTY_LIB_SOVERSION} )
install(TARGETS ${KOPROPERTY_TARGETNAME} ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

add_definitions(-DKOPROPERTY_APP_DIR="\\"${KOPROPERTY_TARGETNAME}\\"") # for KIconLoader::global()->addAppDir();
                                                                       # TODO remove when we move to independent place
kde4_install_icons( ${DATA_INSTALL_DIR}/${KOPROPERTY_TARGETNAME}/icons )

if(FALSE) # TODO: install when we move to independent place
  install( FILES
    Factory.h
    koproperty_global.h
    koproperty_export.h
    Property.h
    Set.h
    Utils.h
    Factory.h
    EditorView.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/koproperty COMPONENT Devel
  )

  install( FILES
    editors/utils.h
    editors/booledit.h
    editors/coloredit.h
    editors/combobox.h
    editors/cursoredit.h
    editors/dateedit.h
    editors/datetimeedit.h
    editors/dummywidget.h
    editors/fontedit.h
    editors/linestyleedit.h
    editors/pixmapedit.h
    editors/pointedit.h
    editors/rectedit.h
    editors/sizeedit.h
    editors/sizepolicyedit.h
    editors/spinbox.h
    editors/spinbox.h
    editors/stringedit.h
    editors/stringlistedit.h
    editors/symbolcombo.h
    editors/timeedit.h
    editors/urledit.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/koproperty/editors COMPONENT Devel)
endif(FALSE)
