project(KexiMobile)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/kexi
    ${CMAKE_SOURCE_DIR}/kexi/core
    ${CMAKE_SOURCE_DIR}/kexi/widget
    ${CMAKE_SOURCE_DIR}/kexi/kexidb
)

set(KexiMobile_SRCS main.cpp
	KexiMobileMainWindow.cpp 
	KexiMobileWidget.cpp
	KexiMobileNavigator.cpp 
        KexiMobileToolbar.cpp
)

qt4_automoc(${KexiMobile_SRCS})

add_executable(keximobile ${KexiMobile_SRCS})

target_link_libraries(keximobile ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
	kexiextendedwidgets
	kexicore
	kdeui
)

install(TARGETS keximobile ${INSTALL_TARGETS_DEFAULT_ARGS})