/****************************************************************************
** Meta object code from reading C++ file 'BankPane.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "BankPane.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BankPane.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.2. It"
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
struct qt_meta_tag_ZN8BankPaneE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN8BankPaneE = QtMocHelpers::stringData(
    "BankPane",
    "handleExit",
    "",
    "handleDepositTextEntry",
    "handleDepositAll",
    "handleWithdrawTextEntry",
    "handleWithdrawAll",
    "handlePartyPool",
    "handlePartyDeposit",
    "handlePartyPoolDeposit",
    "handleItemInfo",
    "handleRemoveItem",
    "handleAddItem"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN8BankPaneE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x08,    1 /* Private */,
       3,    0,   81,    2, 0x08,    2 /* Private */,
       4,    0,   82,    2, 0x08,    3 /* Private */,
       5,    0,   83,    2, 0x08,    4 /* Private */,
       6,    0,   84,    2, 0x08,    5 /* Private */,
       7,    0,   85,    2, 0x08,    6 /* Private */,
       8,    0,   86,    2, 0x08,    7 /* Private */,
       9,    0,   87,    2, 0x08,    8 /* Private */,
      10,    0,   88,    2, 0x08,    9 /* Private */,
      11,    0,   89,    2, 0x08,   10 /* Private */,
      12,    0,   90,    2, 0x08,   11 /* Private */,

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

       0        // eod
};

Q_CONSTINIT const QMetaObject BankPane::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN8BankPaneE.offsetsAndSizes,
    qt_meta_data_ZN8BankPaneE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN8BankPaneE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BankPane, std::true_type>,
        // method 'handleExit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleDepositTextEntry'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleDepositAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleWithdrawTextEntry'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleWithdrawAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handlePartyPool'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handlePartyDeposit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handlePartyPoolDeposit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleItemInfo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleRemoveItem'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleAddItem'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void BankPane::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BankPane *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->handleExit(); break;
        case 1: _t->handleDepositTextEntry(); break;
        case 2: _t->handleDepositAll(); break;
        case 3: _t->handleWithdrawTextEntry(); break;
        case 4: _t->handleWithdrawAll(); break;
        case 5: _t->handlePartyPool(); break;
        case 6: _t->handlePartyDeposit(); break;
        case 7: _t->handlePartyPoolDeposit(); break;
        case 8: _t->handleItemInfo(); break;
        case 9: _t->handleRemoveItem(); break;
        case 10: _t->handleAddItem(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *BankPane::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BankPane::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN8BankPaneE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int BankPane::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
