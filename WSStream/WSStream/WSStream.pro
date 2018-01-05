QT += core
QT += network
QT -= gui

CONFIG += c++11

TARGET = WSStream
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    decoder.cpp \
    encoder.cpp \
    ffmpegmanager.cpp \
    inputstream.cpp \
    mpeg1videoencoder.cpp \
    multiavstream.cpp \
    outputstream.cpp \
    rawvideodecoder.cpp \
    streamingthread.cpp \
    videocapturethread.cpp \
    videoinputstream.cpp \
    wsstreammanager.cpp \
    audioinputstream.cpp \
    pcmaudiodecoder.cpp \
    mp2audioencoder.cpp \
    updateloadfile.cpp \
    jpegutils.cpp \
    jpegencoder.cpp \
    httpuploadthread.cpp \
    httpuploadutils.cpp \
    streamconfig.cpp

LIBS += F:\dongyf\ffmpeg\dev\lib\libavcodec.dll.a \
        F:\dongyf\ffmpeg\dev\lib\libavdevice.dll.a \
        F:\dongyf\ffmpeg\dev\lib\libavfilter.dll.a \
        F:\dongyf\ffmpeg\dev\lib\libavformat.dll.a \
        F:\dongyf\ffmpeg\dev\lib\libavutil.dll.a \
        F:\dongyf\ffmpeg\dev\lib\libpostproc.dll.a \
        F:\dongyf\ffmpeg\dev\lib\libswresample.dll.a \
        F:\dongyf\ffmpeg\dev\lib\libswscale.dll.a

INCLUDEPATH += F:\dongyf\ffmpeg\dev\include

HEADERS += \
    common.h \
    decoder.h \
    encoder.h \
    ffmpegall.h \
    ffmpegmanager.h \
    inputstream.h \
    mpeg1videoencoder.h \
    multiavstream.h \
    outputstream.h \
    rawvideodecoder.h \
    streamingthread.h \
    videocapturethread.h \
    videoinputstream.h \
    wsstreammanager.h \
    audioinputstream.h \
    pcmaudiodecoder.h \
    mp2audioencoder.h \
    updateloadfile.h \
    jpegutils.h \
    jpegencoder.h \
    httpuploadthread.h \
    httpuploadutils.h \
    rapidjson/error/en.h \
    rapidjson/error/error.h \
    rapidjson/internal/biginteger.h \
    rapidjson/internal/diyfp.h \
    rapidjson/internal/dtoa.h \
    rapidjson/internal/ieee754.h \
    rapidjson/internal/itoa.h \
    rapidjson/internal/meta.h \
    rapidjson/internal/pow10.h \
    rapidjson/internal/regex.h \
    rapidjson/internal/stack.h \
    rapidjson/internal/strfunc.h \
    rapidjson/internal/strtod.h \
    rapidjson/internal/swap.h \
    rapidjson/msinttypes/inttypes.h \
    rapidjson/msinttypes/stdint.h \
    rapidjson/allocators.h \
    rapidjson/document.h \
    rapidjson/encodedstream.h \
    rapidjson/encodings.h \
    rapidjson/filereadstream.h \
    rapidjson/filewritestream.h \
    rapidjson/fwd.h \
    rapidjson/istreamwrapper.h \
    rapidjson/memorybuffer.h \
    rapidjson/memorystream.h \
    rapidjson/ostreamwrapper.h \
    rapidjson/pointer.h \
    rapidjson/prettywriter.h \
    rapidjson/rapidjson.h \
    rapidjson/reader.h \
    rapidjson/schema.h \
    rapidjson/stream.h \
    rapidjson/stringbuffer.h \
    rapidjson/writer.h \
    streamconfig.h
