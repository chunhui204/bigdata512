/****************************************************************************
** Meta object code from reading C++ file 'audioplotthread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ALL/audioplotthread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audioplotthread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AudioPlotThread_t {
    QByteArrayData data[15];
    char stringdata0[150];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AudioPlotThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AudioPlotThread_t qt_meta_stringdata_AudioPlotThread = {
    {
QT_MOC_LITERAL(0, 0, 15), // "AudioPlotThread"
QT_MOC_LITERAL(1, 16, 13), // "dataProcessed"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 15), // "QVector<double>"
QT_MOC_LITERAL(4, 47, 2), // "xs"
QT_MOC_LITERAL(5, 50, 2), // "ys"
QT_MOC_LITERAL(6, 53, 17), // "onAudioFormatInit"
QT_MOC_LITERAL(7, 71, 4), // "rate"
QT_MOC_LITERAL(8, 76, 4), // "chns"
QT_MOC_LITERAL(9, 81, 4), // "size"
QT_MOC_LITERAL(10, 86, 13), // "onRecordStart"
QT_MOC_LITERAL(11, 100, 12), // "onRecordStop"
QT_MOC_LITERAL(12, 113, 15), // "onWavFileOpened"
QT_MOC_LITERAL(13, 129, 4), // "name"
QT_MOC_LITERAL(14, 134, 15) // "dataTranslation"

    },
    "AudioPlotThread\0dataProcessed\0\0"
    "QVector<double>\0xs\0ys\0onAudioFormatInit\0"
    "rate\0chns\0size\0onRecordStart\0onRecordStop\0"
    "onWavFileOpened\0name\0dataTranslation"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AudioPlotThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    3,   49,    2, 0x0a /* Public */,
      10,    1,   56,    2, 0x0a /* Public */,
      11,    0,   59,    2, 0x0a /* Public */,
      12,    1,   60,    2, 0x0a /* Public */,
      14,    0,   63,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,    7,    8,    9,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,

       0        // eod
};

void AudioPlotThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AudioPlotThread *_t = static_cast<AudioPlotThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->dataProcessed((*reinterpret_cast< const QVector<double>(*)>(_a[1])),(*reinterpret_cast< const QVector<double>(*)>(_a[2]))); break;
        case 1: _t->onAudioFormatInit((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 2: _t->onRecordStart((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->onRecordStop(); break;
        case 4: _t->onWavFileOpened((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->dataTranslation(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<double> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (AudioPlotThread::*_t)(const QVector<double> & , const QVector<double> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AudioPlotThread::dataProcessed)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject AudioPlotThread::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_AudioPlotThread.data,
      qt_meta_data_AudioPlotThread,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *AudioPlotThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioPlotThread::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_AudioPlotThread.stringdata0))
        return static_cast<void*>(const_cast< AudioPlotThread*>(this));
    return QObject::qt_metacast(_clname);
}

int AudioPlotThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void AudioPlotThread::dataProcessed(const QVector<double> & _t1, const QVector<double> & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
