/****************************************************************************
** Meta object code from reading C++ file 'audiowidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ALL/audiowidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audiowidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AudioWidget_t {
    QByteArrayData data[17];
    char stringdata0[261];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AudioWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AudioWidget_t qt_meta_stringdata_AudioWidget = {
    {
QT_MOC_LITERAL(0, 0, 11), // "AudioWidget"
QT_MOC_LITERAL(1, 12, 13), // "commandIssued"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 7), // "command"
QT_MOC_LITERAL(4, 35, 11), // "recordStart"
QT_MOC_LITERAL(5, 47, 8), // "filename"
QT_MOC_LITERAL(6, 56, 10), // "recordStop"
QT_MOC_LITERAL(7, 67, 15), // "onDataProcessed"
QT_MOC_LITERAL(8, 83, 15), // "QVector<double>"
QT_MOC_LITERAL(9, 99, 2), // "xs"
QT_MOC_LITERAL(10, 102, 2), // "ys"
QT_MOC_LITERAL(11, 105, 17), // "onSavePathChanged"
QT_MOC_LITERAL(12, 123, 3), // "dir"
QT_MOC_LITERAL(13, 127, 32), // "on_button_audio_capStart_clicked"
QT_MOC_LITERAL(14, 160, 30), // "on_button_audio_capEnd_clicked"
QT_MOC_LITERAL(15, 191, 35), // "on_button_audio_recordStart_c..."
QT_MOC_LITERAL(16, 227, 33) // "on_button_audio_recordEnd_cli..."

    },
    "AudioWidget\0commandIssued\0\0command\0"
    "recordStart\0filename\0recordStop\0"
    "onDataProcessed\0QVector<double>\0xs\0"
    "ys\0onSavePathChanged\0dir\0"
    "on_button_audio_capStart_clicked\0"
    "on_button_audio_capEnd_clicked\0"
    "on_button_audio_recordStart_clicked\0"
    "on_button_audio_recordEnd_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AudioWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    1,   62,    2, 0x06 /* Public */,
       6,    0,   65,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    2,   66,    2, 0x0a /* Public */,
      11,    1,   71,    2, 0x0a /* Public */,
      13,    0,   74,    2, 0x08 /* Private */,
      14,    0,   75,    2, 0x08 /* Private */,
      15,    0,   76,    2, 0x08 /* Private */,
      16,    0,   77,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8, 0x80000000 | 8,    9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AudioWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AudioWidget *_t = static_cast<AudioWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->commandIssued((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->recordStart((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->recordStop(); break;
        case 3: _t->onDataProcessed((*reinterpret_cast< const QVector<double>(*)>(_a[1])),(*reinterpret_cast< const QVector<double>(*)>(_a[2]))); break;
        case 4: _t->onSavePathChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->on_button_audio_capStart_clicked(); break;
        case 6: _t->on_button_audio_capEnd_clicked(); break;
        case 7: _t->on_button_audio_recordStart_clicked(); break;
        case 8: _t->on_button_audio_recordEnd_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
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
            typedef void (AudioWidget::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AudioWidget::commandIssued)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (AudioWidget::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AudioWidget::recordStart)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (AudioWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AudioWidget::recordStop)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject AudioWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_AudioWidget.data,
      qt_meta_data_AudioWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *AudioWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_AudioWidget.stringdata0))
        return static_cast<void*>(const_cast< AudioWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int AudioWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void AudioWidget::commandIssued(QString _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AudioWidget::recordStart(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AudioWidget::recordStop()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
