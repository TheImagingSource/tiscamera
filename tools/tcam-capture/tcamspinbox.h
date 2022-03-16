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

#pragma once

#include <QAbstractSpinBox>
#include <cinttypes>
#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
enum StepTypeDef
{
    DefaultStepType,
    AdaptiveDecimalStepType
};
using StepType = StepTypeDef;
#else
using StepType = QAbstractSpinBox::StepType;
#endif


namespace tcam::tools::capture
{

class TcamSpinBoxImpl;

class TcamSpinBox : public QAbstractSpinBox
{
    friend class TcamSpinBoxImpl;
#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
    Q_ENUM(StepType)
#endif
    Q_OBJECT

    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString cleanText READ cleanText)
    Q_PROPERTY(qlonglong minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(qlonglong maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(qlonglong singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(StepType stepType READ stepType WRITE setStepType)
    Q_PROPERTY(qlonglong value READ value WRITE setValue NOTIFY valueChanged USER true)
    Q_PROPERTY(qlonglong displayIntegerBase READ displayIntegerBase WRITE setDisplayIntegerBase)
    Q_PROPERTY(bool wrapping READ wrapping WRITE setWrapping)
public:
    explicit TcamSpinBox(QWidget* parent = nullptr);
    TcamSpinBox(const TcamSpinBox&) = delete;
    TcamSpinBox& operator=(const TcamSpinBox&) = delete;

private:
    virtual ~TcamSpinBox();

public:
    qlonglong value() const;

    qlonglong singleStep() const;
    void setSingleStep(qlonglong val);

    qlonglong minimum() const;
    void setMinimum(qlonglong min);

    qlonglong maximum() const;
    void setMaximum(qlonglong max);

    void setRange(qlonglong min, qlonglong max);

    QString prefix() const;
    void setPrefix(const QString& prefix);

    QString suffix() const;
    void setSuffix(const QString& suffix);
    
    int displayIntegerBase() const;
    void setDisplayIntegerBase(int base);

    virtual void stepBy(int steps) override;

    StepType stepType() const;
    void setStepType(StepType stepType);

    static constexpr qlonglong maxAllowed()
    {
        return std::numeric_limits<qlonglong>::max();
    }
    static constexpr qlonglong minAllowed()
    {
        return std::numeric_limits<qlonglong>::min();
    }

    bool wrapping() const;
    void setWrapping(bool wrapping);

    QString cleanText() const;


    QSize sizeHint() const override;

    QValidator::State validate(QString& input, int& pos) const override;
    virtual void fixup(QString& str) const override;
    virtual QAbstractSpinBox::StepEnabled stepEnabled() const override;

public slots:

    void setValue(qlonglong val);
    void slotEditorCursorPositionChanged(int oldpos, int newpos);
    void slotEditorTextChanged(const QString& t);

signals:

    void valueChanged(qlonglong v);
    void valueChanged(const QString& v);

protected:
    void connectLineEdit();

private:
    TcamSpinBoxImpl* m_impl;
};

} // namespace tcam::tools::capture
