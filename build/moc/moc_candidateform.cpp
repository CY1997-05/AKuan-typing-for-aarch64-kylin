/****************************************************************************
** Meta object code from reading C++ file 'candidateform.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/forms/candidateform.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'candidateform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CandidateForm_t {
    QByteArrayData data[15];
    char stringdata0[163];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CandidateForm_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CandidateForm_t qt_meta_stringdata_CandidateForm = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CandidateForm"
QT_MOC_LITERAL(1, 14, 6), // "reload"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 12), // "onRowChanged"
QT_MOC_LITERAL(4, 35, 5), // "onAdd"
QT_MOC_LITERAL(5, 41, 8), // "onDelete"
QT_MOC_LITERAL(6, 50, 11), // "onDeleteAll"
QT_MOC_LITERAL(7, 62, 11), // "onDelRecord"
QT_MOC_LITERAL(8, 74, 12), // "onDelRecords"
QT_MOC_LITERAL(9, 87, 14), // "onRestoreState"
QT_MOC_LITERAL(10, 102, 17), // "onRestoreAllState"
QT_MOC_LITERAL(11, 120, 8), // "onGenSfz"
QT_MOC_LITERAL(12, 129, 13), // "onGenMultiSfz"
QT_MOC_LITERAL(13, 143, 9), // "onLoadXls"
QT_MOC_LITERAL(14, 153, 9) // "onSaveXls"

    },
    "CandidateForm\0reload\0\0onRowChanged\0"
    "onAdd\0onDelete\0onDeleteAll\0onDelRecord\0"
    "onDelRecords\0onRestoreState\0"
    "onRestoreAllState\0onGenSfz\0onGenMultiSfz\0"
    "onLoadXls\0onSaveXls"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CandidateForm[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    0,   83,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    0,   85,    2, 0x08 /* Private */,
       9,    0,   86,    2, 0x08 /* Private */,
      10,    0,   87,    2, 0x08 /* Private */,
      11,    0,   88,    2, 0x08 /* Private */,
      12,    0,   89,    2, 0x08 /* Private */,
      13,    0,   90,    2, 0x08 /* Private */,
      14,    0,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CandidateForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CandidateForm *_t = static_cast<CandidateForm *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->reload(); break;
        case 1: _t->onRowChanged(); break;
        case 2: _t->onAdd(); break;
        case 3: _t->onDelete(); break;
        case 4: _t->onDeleteAll(); break;
        case 5: _t->onDelRecord(); break;
        case 6: _t->onDelRecords(); break;
        case 7: _t->onRestoreState(); break;
        case 8: _t->onRestoreAllState(); break;
        case 9: _t->onGenSfz(); break;
        case 10: _t->onGenMultiSfz(); break;
        case 11: _t->onLoadXls(); break;
        case 12: _t->onSaveXls(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CandidateForm::staticMetaObject = {
    { &SuiForm::staticMetaObject, qt_meta_stringdata_CandidateForm.data,
      qt_meta_data_CandidateForm,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CandidateForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CandidateForm::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CandidateForm.stringdata0))
        return static_cast<void*>(this);
    return SuiForm::qt_metacast(_clname);
}

int CandidateForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SuiForm::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
