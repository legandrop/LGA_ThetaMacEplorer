#pragma once
#include <QString>
#include <QDateTime>
#include <QMetaType>

// Plain C++ struct - safe to include in both .cpp and .mm files.
// nativeFileHandle holds ICCameraFile* bridged to void* so plain C++ never sees Obj-C types.
struct CameraFileInfo {
    QString  name;
    QString  uti;           // e.g. "public.jpeg", "public.mpeg-4", "com.adobe.raw-image"
    QString  devicePath;    // unique path string from ICCameraFile.deviceFilePath
    qint64   sizeBytes  = 0;
    QDateTime creationDate;
    int      width      = 0;
    int      height     = 0;
    bool     isVideo    = false;
    bool     isRaw      = false;

    // Opaque handle: stores ICCameraFile* as void*.
    // Only ThetaBridge.mm ever casts this back to ICCameraFile*.
    void*    nativeFileHandle = nullptr;
};

Q_DECLARE_METATYPE(CameraFileInfo)
Q_DECLARE_METATYPE(QList<CameraFileInfo>)
