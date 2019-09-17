QT = websockets network

TARGET = DatiKeepAlive
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    socket.cpp

win32 {
    target.path = E:\daticloud
    INSTALLS += target
    INCLUDEPATH += C:/openssl-1.1/x86/include
    LIBS += \
        -L"C:/openssl-1.1/x86/lib" -llibcrypto -llibssl
}

HEADERS += \
    socket.h
