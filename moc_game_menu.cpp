/****************************************************************************
** Meta object code from reading C++ file 'game_menu.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "game_menu.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'game_menu.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8GameMenuE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN8GameMenuE = QtMocHelpers::stringData(
    "GameMenu",
    "logMessageTriggered",
    "",
    "message",
    "startNewGame",
    "loadGame",
    "showRecords",
    "showCredits",
    "quitGame",
    "onInventoryClicked",
    "onMarlithClicked",
    "onOptionsClicked",
    "onAboutClicked",
    "onEditMonsterClicked",
    "onEditSpellbookClicked",
    "onCharacterListClicked",
    "onHelpClicked",
    "onShowStatisticsClicked"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN8GameMenuE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  104,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,  107,    2, 0x08,    3 /* Private */,
       5,    0,  108,    2, 0x08,    4 /* Private */,
       6,    0,  109,    2, 0x08,    5 /* Private */,
       7,    0,  110,    2, 0x08,    6 /* Private */,
       8,    0,  111,    2, 0x08,    7 /* Private */,
       9,    0,  112,    2, 0x08,    8 /* Private */,
      10,    0,  113,    2, 0x08,    9 /* Private */,
      11,    0,  114,    2, 0x08,   10 /* Private */,
      12,    0,  115,    2, 0x08,   11 /* Private */,
      13,    0,  116,    2, 0x08,   12 /* Private */,
      14,    0,  117,    2, 0x08,   13 /* Private */,
      15,    0,  118,    2, 0x08,   14 /* Private */,
      16,    0,  119,    2, 0x08,   15 /* Private */,
      17,    0,  120,    2, 0x08,   16 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

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
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject GameMenu::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN8GameMenuE.offsetsAndSizes,
    qt_meta_data_ZN8GameMenuE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN8GameMenuE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GameMenu, std::true_type>,
        // method 'logMessageTriggered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'startNewGame'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'loadGame'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showRecords'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showCredits'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'quitGame'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onInventoryClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMarlithClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onOptionsClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAboutClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEditMonsterClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEditSpellbookClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCharacterListClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onHelpClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onShowStatisticsClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void GameMenu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GameMenu *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->logMessageTriggered((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->startNewGame(); break;
        case 2: _t->loadGame(); break;
        case 3: _t->showRecords(); break;
        case 4: _t->showCredits(); break;
        case 5: _t->quitGame(); break;
        case 6: _t->onInventoryClicked(); break;
        case 7: _t->onMarlithClicked(); break;
        case 8: _t->onOptionsClicked(); break;
        case 9: _t->onAboutClicked(); break;
        case 10: _t->onEditMonsterClicked(); break;
        case 11: _t->onEditSpellbookClicked(); break;
        case 12: _t->onCharacterListClicked(); break;
        case 13: _t->onHelpClicked(); break;
        case 14: _t->onShowStatisticsClicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (GameMenu::*)(const QString & );
            if (_q_method_type _q_method = &GameMenu::logMessageTriggered; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *GameMenu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameMenu::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN8GameMenuE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int GameMenu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void GameMenu::logMessageTriggered(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
