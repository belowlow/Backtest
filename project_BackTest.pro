QT       += core gui
QT       += sql
QT       += axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src\backtrader.cpp \
    src\calculator.cpp \
    src\datecount.cpp \
    src\indicators.cpp \
    src\main.cpp \
    src\portmanager.cpp

HEADERS += \
    include\backtrader.h \
    include\calculator.h \
    include\datecount.h \
    include\dbtype.h \
    include\indicators.h \
    include\portmanager.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
