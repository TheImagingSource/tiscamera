/*
 * Copyright 2022 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tcamspinbox.h"

#include <QApplication>
#include <QEvent>
#include <QLineEdit>
#include <QStyle>
#include <QStyleOption>
#include <cmath>

namespace tcam::tools::capture
{

enum class SignalEmitPolicy
{
    IfChanged,
    Always,
    Never
};


class TcamSpinBoxImpl
{
public:
    TcamSpinBoxImpl(TcamSpinBox* parent) : p_parent(parent) {};

    virtual ~TcamSpinBoxImpl() {};

    virtual qlonglong valueFromText(const QString& text) const;
    virtual QString textFromValue(qlonglong val) const;
    void setValue(qlonglong value, SignalEmitPolicy emit_policy, bool update = true);
    QVariant validateAndInterpret(QString& input, int& pos, QValidator::State& state) const;
    QVariant calculateAdaptiveDecimalStep(int steps) const;
    void updateEdit();
    QString stripped(const QString& text, int* pos = 0) const;
    void clearCache() const;
    qlonglong bound(const qlonglong& val, const qlonglong& old = 0, int steps = 0) const;
    bool specialValue() const;
    void emitSignals(SignalEmitPolicy ep, const qlonglong& old);
    void setRange(qlonglong min, qlonglong max);
    QSize sizeHint() const;
    QAbstractSpinBox::StepEnabled stepEnabled() const;
    QString longestAllowedString() const;

    qlonglong m_value = 0;
    qlonglong m_minimum = 0;
    qlonglong m_maximum = 99;
    qlonglong m_signel_step = 1;
    int m_display_base = 10;
    QString m_prefix;
    QString m_suffix;
    mutable QSize m_cached_size_hint;
    mutable QSize m_cached_min_size_hint;
    QString m_special_value_text;

    mutable QString m_cached_text;
    mutable qlonglong m_cached_value;
    mutable QValidator::State m_cached_state;
    mutable QString m_cached_maximum_string;

    bool m_wrapping = false;
    StepType m_step_type = StepType::DefaultStepType;
    bool m_ignore_changed_cursor_position = false;

    TcamSpinBox* p_parent = nullptr;
};

void TcamSpinBoxImpl::setValue(qlonglong value, SignalEmitPolicy emit_policy, bool update)
{
    auto old = m_value;
    m_value = bound(value);
    if (update)
    {
        updateEdit();
    }
    p_parent->update();

    if (emit_policy == SignalEmitPolicy::Always
        || (emit_policy == SignalEmitPolicy::IfChanged && old != value))
    {
        emitSignals(emit_policy, old);
    }
}

void TcamSpinBoxImpl::emitSignals(SignalEmitPolicy emit_policy, const qlonglong& old)
{
    if (emit_policy != SignalEmitPolicy::Never)
    {
        if (emit_policy == SignalEmitPolicy::Always || m_value != old)
        {
            Q_EMIT p_parent->valueChanged(p_parent->lineEdit()->displayText());
            Q_EMIT p_parent->valueChanged(m_value);
        }
    }
}

QString TcamSpinBoxImpl::stripped(const QString& t, int* pos) const
{
    QStringRef text(&t);
    if (m_special_value_text.size() == 0 || text != m_special_value_text)
    {
        int from = 0;
        int size = text.size();
        bool changed = false;
        if (m_prefix.size() && text.startsWith(m_prefix))
        {
            from += m_prefix.size();
            size -= from;
            changed = true;
        }
        if (m_suffix.size() && text.endsWith(m_suffix))
        {
            size -= m_suffix.size();
            changed = true;
        }
        if (changed)
        {
            text = text.mid(from, size);
        }
    }

    const int s = text.size();
    text = text.trimmed();
    if (pos)
    {
        (*pos) -= (s - text.size());
    }
    return text.toString();
}

void TcamSpinBoxImpl::clearCache() const
{
    m_cached_text.clear();
    m_cached_value = 0;
    m_cached_maximum_string = QString();
    m_cached_state = QValidator::Acceptable;
}

qlonglong TcamSpinBoxImpl::bound(const qlonglong& val, const qlonglong& old, int steps) const
{
    qlonglong v = val;
    if (!m_wrapping || (steps == 0) || (old == 0))
    {
        if (v < m_minimum)
        {
            v = m_wrapping ? m_maximum : m_minimum;
        }
        if (v > m_maximum)
        {
            v = m_wrapping ? m_minimum : m_maximum;
        }
    }
    else
    {
        bool wasMin = old == m_minimum;
        bool wasMax = old == m_maximum;
        bool oldcmp = v > old;
        bool maxcmp = v > m_maximum;
        bool mincmp = v > m_minimum;
        bool wrapped = (oldcmp && steps < 0) || (!oldcmp && steps > 0);
        if (maxcmp)
        {
            v = ((wasMax && !wrapped && steps > 0) || (steps < 0 && !wasMin && wrapped)) ?
                    m_minimum :
                    m_maximum;
        }
        else if (wrapped && (maxcmp || !mincmp))
        {
            v = ((wasMax && steps > 0) || (!wasMin && steps < 0)) ? m_minimum : m_maximum;
        }
        else if (!mincmp)
        {
            v = (!wasMax && !wasMin ? m_minimum : m_maximum);
        }
    }

    return v;
}

bool TcamSpinBoxImpl::specialValue() const
{
    return (m_value == m_minimum && !m_special_value_text.isEmpty());
}

qlonglong TcamSpinBoxImpl::valueFromText(const QString& text) const
{
    QString copy = text;
    int pos = p_parent->lineEdit()->cursorPosition();
    QValidator::State state = QValidator::Acceptable;
    return validateAndInterpret(copy, pos, state).toLongLong();
}

QAbstractSpinBox::StepEnabled TcamSpinBoxImpl::stepEnabled() const
{
    if (p_parent->isReadOnly())
    {
        return QAbstractSpinBox::StepEnabledFlag::StepNone;
    }

    if (m_wrapping)
    {
        return QAbstractSpinBox::StepEnabled(QAbstractSpinBox::StepEnabledFlag::StepUpEnabled
                                             | QAbstractSpinBox::StepEnabledFlag::StepDownEnabled);
    }
    QAbstractSpinBox::StepEnabled ret = QAbstractSpinBox::StepEnabledFlag::StepNone;
    if (m_value < m_maximum)
    {
        ret |= QAbstractSpinBox::StepEnabledFlag::StepUpEnabled;
    }
    if (m_value > m_minimum)
    {
        ret |= QAbstractSpinBox::StepEnabledFlag::StepDownEnabled;
    }
    return ret;
}

QVariant TcamSpinBoxImpl::validateAndInterpret(QString& input,
                                               int& pos,
                                               QValidator::State& state) const
{
    if (m_cached_text == input && !input.isEmpty())
    {
        state = m_cached_state;
        return m_cached_value;
    }

    QString copy = stripped(input, &pos);
    state = QValidator::Acceptable;
    auto num = m_minimum;

    if (m_maximum != m_minimum
        && (copy.isEmpty() || (m_minimum < 0 && copy == QLatin1String("-"))
            || (m_maximum >= 0 && copy == QLatin1String("+"))))
    {
        state = QValidator::Intermediate;
    }
    else if (copy.startsWith(QLatin1Char('-')) && m_minimum >= 0)
    {
        state = QValidator::Invalid;
    }
    else
    {
        bool ok = false;
        if (m_display_base != 10)
        {
            num = copy.toLongLong(&ok, m_display_base);
        }
        else
        {
            num = p_parent->locale().toLongLong(copy, &ok);
            if (!ok && (m_maximum >= 1000 || m_minimum <= -1000))
            {
                const QChar sep = p_parent->locale().groupSeparator();
                const QChar doubleSep[2] = { sep, sep };
                if (copy.contains(sep) && !copy.contains(QString(doubleSep, 2)))
                {
                    QString copy2 = copy;
                    copy2.remove(p_parent->locale().groupSeparator());
                    num = p_parent->locale().toLongLong(copy2, &ok);
                }
            }
        }
        if (!ok)
        {
            state = QValidator::Invalid;
        }
        else if (num >= m_minimum && num <= m_maximum)
        {
            state = QValidator::Acceptable;
        }
        else if (m_maximum == m_minimum)
        {
            state = QValidator::Invalid;
        }
        else
        {
            if ((num >= 0 && num > m_maximum) || (num < 0 && num < m_minimum))
            {
                state = QValidator::Invalid;
            }
            else
            {
                state = QValidator::Intermediate;
            }
        }
    }
    if (state != QValidator::Acceptable)
    {
        num = m_maximum > 0 ? m_minimum : m_maximum;
    }
    input = m_prefix + copy + m_suffix;
    m_cached_text = input;
    m_cached_state = state;
    m_cached_value = num;

    return m_cached_value;
}

QVariant TcamSpinBoxImpl::calculateAdaptiveDecimalStep(int steps) const
{
    qlonglong value = m_value;
    qlonglong abs_value = std::abs(value);

    if (abs_value < 100)
    {
        return 1;
    }

    const bool valueNegative = value < 0;
    const bool stepsNegative = steps < 0;
    const int signCompensation = (valueNegative == stepsNegative) ? 0 : 1;

    const int log = static_cast<int>(std::log10(abs_value - signCompensation)) - 1;
    return static_cast<qlonglong>(std::pow(10, log));
}

void TcamSpinBoxImpl::updateEdit()
{
    QString newText;
    if (specialValue())
    {
        newText = m_special_value_text;
    }
    else
    {
        newText = m_prefix + textFromValue(m_value) + m_suffix;
    }

    if (newText == p_parent->lineEdit()->displayText())
    {
        return;
    }

    const bool empty = p_parent->lineEdit()->text().isEmpty();
    int cursor = p_parent->lineEdit()->cursorPosition();
    int selsize = p_parent->lineEdit()->selectedText().size();
    const QSignalBlocker blocker(p_parent->lineEdit());
    p_parent->lineEdit()->setText(newText);

    if (!specialValue())
    {
        cursor = qBound(
            m_prefix.size(), cursor, p_parent->lineEdit()->displayText().size() - m_suffix.size());

        if (selsize > 0)
        {
            p_parent->lineEdit()->setSelection(cursor, selsize);
        }
        else
        {
            p_parent->lineEdit()->setCursorPosition(empty ? m_prefix.size() : cursor);
        }
    }
    p_parent->update();
}

void TcamSpinBoxImpl::setRange(qlonglong min, qlonglong max)
{
    clearCache();
    m_minimum = min;
    m_maximum = (min < max) ? max : min;
    m_cached_size_hint = QSize();
    m_cached_min_size_hint = QSize();
    m_cached_maximum_string = QString();

    if (!(bound(m_value) == m_value))
    {
        setValue(bound(m_value), SignalEmitPolicy::IfChanged);
    }
    else if (m_value == m_minimum && !m_special_value_text.isEmpty())
    {
        updateEdit();
    }

    p_parent->updateGeometry();
}

QString TcamSpinBoxImpl::longestAllowedString() const
{
    if (m_cached_maximum_string.isEmpty())
    {
        QString minStr = textFromValue(m_minimum);
        QString maxStr = textFromValue(m_maximum);
        const QFontMetrics fm(p_parent->fontMetrics());

        int advance_min;
        int advance_max;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
        advance_min = fm.horizontalAdvance(minStr);
        advance_max = fm.horizontalAdvance(maxStr);
#else
        advance_min = fm.width(minStr);
        advance_max = fm.width(maxStr);
#endif

        if (advance_min > advance_max)
        {
            m_cached_maximum_string = minStr;
        }
        else
        {
            m_cached_maximum_string = maxStr;
        }
    }
    return m_cached_maximum_string;
}

QSize TcamSpinBoxImpl::sizeHint() const
{
    if (m_cached_size_hint.isEmpty())
    {
        p_parent->ensurePolished();

        const QFontMetrics fm(p_parent->fontMetrics());
        int h = p_parent->lineEdit()->sizeHint().height();
        int w = 0;
        QString s;
        QString fixedContent = m_prefix + m_suffix + QLatin1Char(' ');
        s = longestAllowedString();
        s += fixedContent;

        int advance;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
        advance = fm.horizontalAdvance(s);
#else
        advance = fm.width(s);
#endif

        w = std::max(w, advance);

        if (m_special_value_text.size())
        {
            s = m_special_value_text;
            w = std::max(w, advance);
        }
        w += 2;

        QStyleOptionSpinBox opt;
        p_parent->initStyleOption(&opt);
        QSize hint(w, h);
        m_cached_size_hint = p_parent->style()
                                 ->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, p_parent)
                                 .expandedTo(QApplication::globalStrut());
    }
    return m_cached_size_hint;
}

QString TcamSpinBoxImpl::textFromValue(qlonglong value) const
{
    QString str;

    if (m_display_base != 10)
    {
        const QLatin1String prefix = value < 0 ? QLatin1String("-") : QLatin1String();
        str = prefix + QString::number(std::abs(value), m_display_base);
    }
    else
    {
        str = p_parent->locale().toString(value);
        if (!p_parent->isGroupSeparatorShown() && (std::abs(value) >= 1000 || value == INT_MIN))
        {
            str.remove(p_parent->locale().groupSeparator());
        }
    }

    return str;
}


TcamSpinBox::TcamSpinBox(QWidget* parent)
    : QAbstractSpinBox(parent), m_impl(new TcamSpinBoxImpl(this))
{
    qRegisterMetaType<qlonglong>("qlonglong");
    connectLineEdit();
}

TcamSpinBox::~TcamSpinBox() {}

qlonglong TcamSpinBox::value() const
{
    return m_impl->m_value;
}

void TcamSpinBox::slotEditorCursorPositionChanged(int oldpos, int newpos)
{
    if (!lineEdit()->hasSelectedText() && !m_impl->m_ignore_changed_cursor_position
        && !m_impl->specialValue())
    {
        m_impl->m_ignore_changed_cursor_position = true;

        bool allowSelection = true;
        int pos = -1;
        if (newpos < m_impl->m_prefix.size() && newpos != 0)
        {
            if (oldpos == 0)
            {
                allowSelection = false;
                pos = m_impl->m_prefix.size();
            }
            else
            {
                pos = oldpos;
            }
        }
        else if (newpos > lineEdit()->text().size() - m_impl->m_suffix.size()
                 && newpos != lineEdit()->text().size())
        {
            if (oldpos == lineEdit()->text().size())
            {
                pos = lineEdit()->text().size() - m_impl->m_suffix.size();
                allowSelection = false;
            }
            else
            {
                pos = lineEdit()->text().size();
            }
        }
        if (pos != -1)
        {
            const int selSize =
                lineEdit()->selectionStart() >= 0 && allowSelection ?
                    (lineEdit()->selectedText().size() * (newpos < pos ? -1 : 1)) - newpos + pos :
                    0;

            const QSignalBlocker blocker(lineEdit());
            if (selSize != 0)
            {
                lineEdit()->setSelection(pos - selSize, selSize);
            }
            else
            {
                lineEdit()->setCursorPosition(pos);
            }
        }
        m_impl->m_ignore_changed_cursor_position = false;
    }
}

void TcamSpinBox::slotEditorTextChanged(const QString& t)
{
    QString tmp = t;
    int pos = lineEdit()->cursorPosition();
    QValidator::State state = validate(tmp, pos);
    if (state == QValidator::Acceptable)
    {
        auto v = m_impl->valueFromText(tmp);
        m_impl->setValue(v, SignalEmitPolicy::IfChanged, tmp != t);
    }
}

void TcamSpinBox::setValue(qlonglong value)
{
    m_impl->setValue(value, SignalEmitPolicy::IfChanged, true);
}

QString TcamSpinBox::prefix() const
{
    return m_impl->m_prefix;
}

void TcamSpinBox::setPrefix(const QString& prefix)
{
    m_impl->m_prefix = prefix;
    m_impl->updateEdit();

    m_impl->m_cached_size_hint = QSize();
    m_impl->m_cached_min_size_hint = QSize();
    updateGeometry();
}

QString TcamSpinBox::suffix() const
{
    return m_impl->m_suffix;
}

void TcamSpinBox::setSuffix(const QString& suffix)
{
    m_impl->m_suffix = suffix;
    m_impl->updateEdit();

    m_impl->m_cached_size_hint = QSize();
    m_impl->m_cached_min_size_hint = QSize();
    updateGeometry();
}

QString TcamSpinBox::cleanText() const
{
    return m_impl->stripped(lineEdit()->displayText());
}

qlonglong TcamSpinBox::singleStep() const
{
    return m_impl->m_signel_step;
}

void TcamSpinBox::setSingleStep(qlonglong value)
{
    if (value >= 0)
    {
        m_impl->m_signel_step = value;
        m_impl->updateEdit();
    }
}

void TcamSpinBox::stepBy(int steps)
{
    auto old = m_impl->m_value;
    SignalEmitPolicy e = SignalEmitPolicy::IfChanged;
    qlonglong singleStep = m_impl->m_signel_step;
    switch (stepType())
    {
        case StepType::AdaptiveDecimalStepType:
        {
            singleStep = m_impl->calculateAdaptiveDecimalStep(steps).toLongLong();
            break;
        }
        default:
        {
            break;
        }
    }

    double tmp = (double)m_impl->m_value + (double)(singleStep * steps);
    qlonglong tmpValue = 0;
    if (tmp > maxAllowed())
    {
        tmpValue = maxAllowed();
    }
    else if (tmp < minAllowed())
    {
        tmpValue = minAllowed();
    }
    else
    {
        tmpValue = m_impl->m_value + (singleStep * steps);
    }

    m_impl->setValue(m_impl->bound(static_cast<qlonglong>(tmpValue), old, steps), e);
    selectAll();
}


bool TcamSpinBox::wrapping() const
{
    return m_impl->m_wrapping;
}

void TcamSpinBox::setWrapping(bool wrapping)
{
    m_impl->m_wrapping = wrapping;
}

qlonglong TcamSpinBox::minimum() const
{
    return m_impl->m_minimum;
}

void TcamSpinBox::setMinimum(qlonglong minimum)
{
    setRange(minimum, (m_impl->m_maximum > minimum) ? m_impl->m_maximum : minimum);
}

qlonglong TcamSpinBox::maximum() const
{
    return m_impl->m_maximum;
}

void TcamSpinBox::setMaximum(qlonglong maximum)
{
    setRange((m_impl->m_minimum < maximum) ? m_impl->m_minimum : maximum, maximum);
}

void TcamSpinBox::setRange(qlonglong min, qlonglong max)
{
    m_impl->setRange(min, max);
}

void TcamSpinBox::setStepType(StepType stepType)
{
    m_impl->m_step_type = stepType;
}

StepType TcamSpinBox::stepType() const
{
    return m_impl->m_step_type;
}

int TcamSpinBox::displayIntegerBase() const
{
    return m_impl->m_display_base;
}

void TcamSpinBox::setDisplayIntegerBase(int base)
{
    if (Q_UNLIKELY(base < 2 || base > 36))
    {
        qWarning("TcamSpinBox::setDisplayIntegerBase: base must be between 2 and 36");
        base = 10;
    }

    if (base != m_impl->m_display_base)
    {
        m_impl->m_display_base = base;
        m_impl->updateEdit();
    }
}

QSize TcamSpinBox::sizeHint() const
{
    return m_impl->sizeHint();
}

QValidator::State TcamSpinBox::validate(QString& text, int& pos) const
{
    QValidator::State state;
    m_impl->validateAndInterpret(text, pos, state);
    return state;
}


QAbstractSpinBox::StepEnabled TcamSpinBox::stepEnabled() const
{
    return m_impl->stepEnabled();
}


void TcamSpinBox::fixup(QString& input) const
{
    if (!isGroupSeparatorShown())
    {
        input.remove(locale().groupSeparator());
    }
}


void TcamSpinBox::connectLineEdit()
{
    if (!lineEdit())
    {
        return;
    }
    (void)connect(lineEdit(), &QLineEdit::textChanged, this, &TcamSpinBox::slotEditorTextChanged);
    (void)connect(lineEdit(),
                  &QLineEdit::cursorPositionChanged,
                  this,
                  &TcamSpinBox::slotEditorCursorPositionChanged);
    (void)connect(
        lineEdit(), &QLineEdit::cursorPositionChanged, this, &TcamSpinBox::updateMicroFocus);
}


} // namespace tcam::tools::capture
