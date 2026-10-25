#
# Copyright 2022 naracanto <https://naracanto.github.io>.
#
# This file is part of QTabelo <https://github.com/beletalabs/qtabelo>.
#
# QTabelo is an open source table editor written in C++ using the
# Qt framework.
#
# QTabelo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# QTabelo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with QTabelo.  If not, see <https://www.gnu.org/licenses/>.
#

QT += core gui widgets svg
QT += quick quickwidgets

QT += svg
greaterThan(QT_MAJOR_VERSION, 5): QT += svgwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    about_dialog.cpp \
    application_window.cpp \
    colophon_dialog.cpp \
    colophon_pages.cpp \
    confirmation_dialog.cpp \
    dialog_header_box.cpp \
    document_manager.cpp \
    document_widget.cpp \
    document_window.cpp \
    main.cpp \
    preferences_dialog.cpp \
    properties_dialog.cpp \
    properties_pages.cpp \
    recent_document_list.cpp \
    rename_dialog.cpp \
    table_document.cpp \
    table_info_provider.cpp

HEADERS += \
    about_dialog.h \
    application_window.h \
    colophon_dialog.h \
    colophon_pages.h \
    confirmation_dialog.h \
    dialog_header_box.h \
    document_manager.h \
    document_widget.h \
    document_window.h \
    preferences_dialog.h \
    properties_dialog.h \
    properties_pages.h \
    recent_document_list.h \
    rename_dialog.h \
    table_document.h \
    table_info_provider.h

RESOURCES += \
    icons.qrc \
    qml.qrc

CONFIG += lrelease

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    TableInfo.qml
