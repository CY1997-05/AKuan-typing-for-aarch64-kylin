#include "theme.h"
#include "common.h"

Theme *Theme::s_instance = 0;

Theme *Theme::instance()
{
    if (!s_instance)
        s_instance = new Theme();
    return s_instance;
}

Theme::Theme()
    : m_index(0)
{
}

int Theme::count() const
{
    return 3;
}

int Theme::index() const
{
    return m_index;
}

void Theme::setIndex(int i)
{
    if (i < 0) i = 0;
    m_index = i % count();
}

void Theme::next()
{
    m_index = (m_index + 1) % count();
}

SuiTheme Theme::current() const
{
    SuiTheme t;
    switch (m_index) {
    case 1: // 浅蓝
        t.name          = L("浅蓝");
        t.clientTop     = QColor(0xF0, 0xF4, 0xFA);
        t.clientBottom  = QColor(0xE4, 0xEC, 0xF7);
        t.border        = QColor(0x6A, 0x7A, 0x90);
        t.titleTop      = QColor(0x2A, 0x5B, 0xA0);
        t.titleBottom   = QColor(0x10, 0x28, 0x50);
        t.titleText     = QColor(0xFF, 0xFF, 0xFF);
        t.buttonTop     = QColor(0xFB, 0xFD, 0xFF);
        t.buttonBottom  = QColor(0xD8, 0xE4, 0xF4);
        t.buttonBorder  = QColor(0x6A, 0x7A, 0x90);
        t.text          = QColor(0x00, 0x00, 0x00);
        t.accent        = QColor(0xFF, 0x00, 0x00);
        t.fieldBg       = QColor(0xFF, 0xFF, 0xFF);
        t.fieldText     = QColor(0x00, 0x00, 0x00);
        t.articleBg     = QColor(0xEC, 0xF1, 0xF9);
        t.articleText   = QColor(0x00, 0x00, 0x00);
        t.inputBg       = QColor(0x00, 0x80, 0xC0);
        t.inputText     = QColor(0xFF, 0xFF, 0xFF);
        t.inputWrong    = QColor(0xFF, 0x40, 0x40);
        break;
    case 2: // 浅绿
        t.name          = L("浅绿");
        t.clientTop     = QColor(0xF2, 0xF7, 0xF0);
        t.clientBottom  = QColor(0xE6, 0xEF, 0xE2);
        t.border        = QColor(0x6E, 0x82, 0x6A);
        t.titleTop      = QColor(0x2E, 0x5E, 0x42);
        t.titleBottom   = QColor(0x10, 0x2A, 0x1C);
        t.titleText     = QColor(0xFF, 0xFF, 0xFF);
        t.buttonTop     = QColor(0xFD, 0xFF, 0xFB);
        t.buttonBottom  = QColor(0xDD, 0xE9, 0xD8);
        t.buttonBorder  = QColor(0x6E, 0x82, 0x6A);
        t.text          = QColor(0x00, 0x00, 0x00);
        t.accent        = QColor(0xFF, 0x00, 0x00);
        t.fieldBg       = QColor(0xFF, 0xFF, 0xFF);
        t.fieldText     = QColor(0x00, 0x00, 0x00);
        t.articleBg     = QColor(0xEE, 0xF4, 0xEC);
        t.articleText   = QColor(0x00, 0x00, 0x00);
        t.inputBg       = QColor(0x00, 0x80, 0xC0);
        t.inputText     = QColor(0xFF, 0xFF, 0xFF);
        t.inputWrong    = QColor(0xFF, 0x40, 0x40);
        break;
    default: // 经典灰（与原版一致）
        t.name          = L("经典");
        t.clientTop     = QColor(0xF0, 0xF0, 0xF0);
        t.clientBottom  = QColor(0xE8, 0xE8, 0xE8);
        t.border        = QColor(0x80, 0x80, 0x80);
        t.titleTop      = QColor(0x40, 0x60, 0x8F);
        t.titleBottom   = QColor(0x1B, 0x2C, 0x47);
        t.titleText     = QColor(0xFF, 0xFF, 0xFF);
        t.buttonTop     = QColor(0xFD, 0xFD, 0xFD);
        t.buttonBottom  = QColor(0xD4, 0xD4, 0xD4);
        t.buttonBorder  = QColor(0x7A, 0x7A, 0x7A);
        t.text          = QColor(0x00, 0x00, 0x00);
        t.accent        = QColor(0xFF, 0x00, 0x00);
        t.fieldBg       = QColor(0xFF, 0xFF, 0xFF);
        t.fieldText     = QColor(0x00, 0x00, 0x00);
        t.articleBg     = QColor(0xE8, 0xE8, 0xE8);
        t.articleText   = QColor(0x00, 0x00, 0x00);
        t.inputBg       = QColor(0x00, 0x80, 0xC0);
        t.inputText     = QColor(0xFF, 0xFF, 0xFF);
        t.inputWrong    = QColor(0xFF, 0x40, 0x40);
        break;
    }
    return t;
}

QString Theme::styleSheet() const
{
    const SuiTheme t = current();
    QString css = QString::fromLatin1(
        "QWidget { color: @TEXT@; }"
        "QPushButton {"
        "  border: 1px solid @BTNBORDER@;"
        "  border-radius: 3px;"
        "  padding: 2px 8px;"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 @BTNTOP@, stop:0.5 #F2F2F2, stop:1 @BTNBOTTOM@);"
        "  color: @TEXT@;"
        "}"
        "QPushButton:hover {"
        "  border-color: #3355AA;"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #FFFFFF, stop:0.5 #EDF3FF, stop:1 @BTNBOTTOM@);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #C8C8C8, stop:1 #E0E0E0);"
        "}"
        "QPushButton:disabled { color: #808080; }"
        "QLineEdit, QComboBox, QSpinBox, QPlainTextEdit, QTextEdit {"
        "  border: 1px solid @BORDER@; border-radius: 0px; padding: 1px 3px;"
        "  background: @FIELD@; color: @FIELDTEXT@;"
        "}"
        "QComboBox QAbstractItemView { border: 1px solid @BORDER@; background: @FIELD@;"
        "  selection-background-color: #316AC5; selection-color: #ffffff; }"
        "QTableWidget { border: 1px solid @BORDER@; background: @FIELD@; gridline-color: #C0C0C0; }"
        "QHeaderView::section { background: #F0F0F0; border: 0px; border-right: 1px solid #C0C0C0;"
        "  border-bottom: 1px solid #C0C0C0; padding: 3px; }"
        "QTableWidget::item:selected { background: #316AC5; color: #ffffff; }"
        "QCheckBox, QRadioButton, QLabel { background: transparent; }"
        "QGroupBox { border: 1px solid @BORDER@; border-radius: 2px; margin-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px; }"
        "QMenu { border: 1px solid @BORDER@; background: @FIELD@; }"
        "QMenu::item:selected { background: #316AC5; color: #ffffff; }"
        "QScrollBar:vertical { width: 14px; background: #E4E4E4; }"
        "QScrollBar::handle:vertical { background: #B8B8B8; min-height: 24px; }"
    );

    css.replace(QLatin1String("@TEXT@"), t.text.name());
    css.replace(QLatin1String("@BTNBORDER@"), t.buttonBorder.name());
    css.replace(QLatin1String("@BTNTOP@"), t.buttonTop.name());
    css.replace(QLatin1String("@BTNBOTTOM@"), t.buttonBottom.name());
    css.replace(QLatin1String("@BORDER@"), t.border.name());
    css.replace(QLatin1String("@FIELD@"), t.fieldBg.name());
    css.replace(QLatin1String("@FIELDTEXT@"), t.fieldText.name());

    return css;
}
