#include "labeledittype.h"

LabelEditType::LabelEditType(QWidget *parent) : QLabel(parent)
{
    setContentsMargins(0, 0, 0, 0); //设置边距

    m_editor = new QLineEdit(this);
    auto layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->addWidget(m_editor);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    m_editor->hide();
    connect(m_editor, &QLineEdit::editingFinished, this, &LabelEditType::onFinishedEdit);
}

void LabelEditType::openEditor()
{
    //相关参数，然后展示
    m_editor->setText(text());
    m_editor->setSelection(0, text().length());
    m_editor->setFont(font());
    m_editor->setFocus(Qt::MouseFocusReason);
    m_editor->show();

    emit editingStarted();      //开始编辑
}

void LabelEditType::onFinishedEdit()
{
    m_editor->hide();           //完成编辑，编辑器隐藏
    //内容不为空，设置编辑器的内容
    if (!m_editor->text().isEmpty())
    {
        setText(m_editor->text());
    }
    emit editingFinished(text());   //发送编辑完成信号
}
