/****************************************************************************
** Meta object code from reading C++ file 'ThetaBridge.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/thetaexplorer/ThetaBridge.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ThetaBridge.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSThetaBridgeENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSThetaBridgeENDCLASS = QtMocHelpers::stringData(
    "ThetaBridge",
    "cameraConnected",
    "",
    "cameraName",
    "cameraDisconnected",
    "cameraError",
    "errorMessage",
    "fileListUpdated",
    "QList<CameraFileInfo>",
    "files",
    "thumbnailReady",
    "devicePath",
    "thumbnail",
    "downloadProgress",
    "fileName",
    "percentComplete",
    "downloadFileCompleted",
    "savedPath",
    "downloadError",
    "deleteCompleted",
    "deletedPaths",
    "deleteError"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSThetaBridgeENDCLASS_t {
    uint offsetsAndSizes[44];
    char stringdata0[12];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[19];
    char stringdata5[12];
    char stringdata6[13];
    char stringdata7[16];
    char stringdata8[22];
    char stringdata9[6];
    char stringdata10[15];
    char stringdata11[11];
    char stringdata12[10];
    char stringdata13[17];
    char stringdata14[9];
    char stringdata15[16];
    char stringdata16[22];
    char stringdata17[10];
    char stringdata18[14];
    char stringdata19[16];
    char stringdata20[13];
    char stringdata21[12];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSThetaBridgeENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSThetaBridgeENDCLASS_t qt_meta_stringdata_CLASSThetaBridgeENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "ThetaBridge"
        QT_MOC_LITERAL(12, 15),  // "cameraConnected"
        QT_MOC_LITERAL(28, 0),  // ""
        QT_MOC_LITERAL(29, 10),  // "cameraName"
        QT_MOC_LITERAL(40, 18),  // "cameraDisconnected"
        QT_MOC_LITERAL(59, 11),  // "cameraError"
        QT_MOC_LITERAL(71, 12),  // "errorMessage"
        QT_MOC_LITERAL(84, 15),  // "fileListUpdated"
        QT_MOC_LITERAL(100, 21),  // "QList<CameraFileInfo>"
        QT_MOC_LITERAL(122, 5),  // "files"
        QT_MOC_LITERAL(128, 14),  // "thumbnailReady"
        QT_MOC_LITERAL(143, 10),  // "devicePath"
        QT_MOC_LITERAL(154, 9),  // "thumbnail"
        QT_MOC_LITERAL(164, 16),  // "downloadProgress"
        QT_MOC_LITERAL(181, 8),  // "fileName"
        QT_MOC_LITERAL(190, 15),  // "percentComplete"
        QT_MOC_LITERAL(206, 21),  // "downloadFileCompleted"
        QT_MOC_LITERAL(228, 9),  // "savedPath"
        QT_MOC_LITERAL(238, 13),  // "downloadError"
        QT_MOC_LITERAL(252, 15),  // "deleteCompleted"
        QT_MOC_LITERAL(268, 12),  // "deletedPaths"
        QT_MOC_LITERAL(281, 11)   // "deleteError"
    },
    "ThetaBridge",
    "cameraConnected",
    "",
    "cameraName",
    "cameraDisconnected",
    "cameraError",
    "errorMessage",
    "fileListUpdated",
    "QList<CameraFileInfo>",
    "files",
    "thumbnailReady",
    "devicePath",
    "thumbnail",
    "downloadProgress",
    "fileName",
    "percentComplete",
    "downloadFileCompleted",
    "savedPath",
    "downloadError",
    "deleteCompleted",
    "deletedPaths",
    "deleteError"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSThetaBridgeENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   74,    2, 0x06,    1 /* Public */,
       4,    0,   77,    2, 0x06,    3 /* Public */,
       5,    1,   78,    2, 0x06,    4 /* Public */,
       7,    1,   81,    2, 0x06,    6 /* Public */,
      10,    2,   84,    2, 0x06,    8 /* Public */,
      13,    2,   89,    2, 0x06,   11 /* Public */,
      16,    2,   94,    2, 0x06,   14 /* Public */,
      18,    2,   99,    2, 0x06,   17 /* Public */,
      19,    1,  104,    2, 0x06,   20 /* Public */,
      21,    1,  107,    2, 0x06,   22 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QString, QMetaType::QPixmap,   11,   12,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,   14,   15,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   14,   17,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   14,    6,
    QMetaType::Void, QMetaType::QStringList,   20,
    QMetaType::Void, QMetaType::QString,    6,

       0        // eod
};

Q_CONSTINIT const QMetaObject ThetaBridge::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSThetaBridgeENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSThetaBridgeENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSThetaBridgeENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ThetaBridge, std::true_type>,
        // method 'cameraConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'cameraDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cameraError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'fileListUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<CameraFileInfo> &, std::false_type>,
        // method 'thumbnailReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPixmap &, std::false_type>,
        // method 'downloadProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'downloadFileCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'downloadError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'deleteCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>,
        // method 'deleteError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void ThetaBridge::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ThetaBridge *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->cameraConnected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->cameraDisconnected(); break;
        case 2: _t->cameraError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->fileListUpdated((*reinterpret_cast< std::add_pointer_t<QList<CameraFileInfo>>>(_a[1]))); break;
        case 4: _t->thumbnailReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QPixmap>>(_a[2]))); break;
        case 5: _t->downloadProgress((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->downloadFileCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->downloadError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->deleteCompleted((*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[1]))); break;
        case 9: _t->deleteError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<CameraFileInfo> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ThetaBridge::*)(const QString & );
            if (_t _q_method = &ThetaBridge::cameraConnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)();
            if (_t _q_method = &ThetaBridge::cameraDisconnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QString & );
            if (_t _q_method = &ThetaBridge::cameraError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QList<CameraFileInfo> & );
            if (_t _q_method = &ThetaBridge::fileListUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QString & , const QPixmap & );
            if (_t _q_method = &ThetaBridge::thumbnailReady; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QString & , int );
            if (_t _q_method = &ThetaBridge::downloadProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QString & , const QString & );
            if (_t _q_method = &ThetaBridge::downloadFileCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QString & , const QString & );
            if (_t _q_method = &ThetaBridge::downloadError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QStringList & );
            if (_t _q_method = &ThetaBridge::deleteCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (ThetaBridge::*)(const QString & );
            if (_t _q_method = &ThetaBridge::deleteError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
    }
}

const QMetaObject *ThetaBridge::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ThetaBridge::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSThetaBridgeENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ThetaBridge::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void ThetaBridge::cameraConnected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ThetaBridge::cameraDisconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ThetaBridge::cameraError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ThetaBridge::fileListUpdated(const QList<CameraFileInfo> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ThetaBridge::thumbnailReady(const QString & _t1, const QPixmap & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ThetaBridge::downloadProgress(const QString & _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ThetaBridge::downloadFileCompleted(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ThetaBridge::downloadError(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void ThetaBridge::deleteCompleted(const QStringList & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void ThetaBridge::deleteError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}
QT_WARNING_POP
