#include "toggle_switch.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

ToggleSwitch::ToggleSwitch(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(46, 26);
    setCursor(Qt::PointingHandCursor);

    m_animation = new QPropertyAnimation(this, "knobPosition", this);
    m_animation->setDuration(180);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void ToggleSwitch::setChecked(bool checked)
{
    if (m_checked == checked) return;
    m_checked = checked;

    m_animation->stop();
    m_animation->setStartValue(m_knobPosition);
    m_animation->setEndValue(checked ? 1.0 : 0.0);
    m_animation->start();

    emit toggled(m_checked);
}

void ToggleSwitch::setKnobPosition(qreal pos)
{
    m_knobPosition = pos;
    update();
}

void ToggleSwitch::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        event->accept();
    }
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        setChecked(!m_checked);
        event->accept();
    }
}

void ToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Фон дорожки
    QColor trackOff(60, 70, 110, 56);   // полупрозрачный серо-синий
    QColor trackOn(123, 168, 255);      // акцент

    QColor trackColor = m_checked
        ? QColor(
            trackOff.red() + (trackOn.red() - trackOff.red()) * m_knobPosition,
            trackOff.green() + (trackOn.green() - trackOff.green()) * m_knobPosition,
            trackOff.blue() + (trackOn.blue() - trackOff.blue()) * m_knobPosition,
            trackOff.alpha() + (255 - trackOff.alpha()) * m_knobPosition)
        : trackOff;

    QPainterPath track;
    track.addRoundedRect(rect(), 13, 13);
    p.fillPath(track, trackColor);

    // Кружок
    int knobSize = 20;
    int margin = 3;
    int travel = width() - knobSize - margin * 2;
    int x = margin + static_cast<int>(travel * m_knobPosition);
    int y = margin;

    QPainterPath knob;
    knob.addEllipse(x, y, knobSize, knobSize);

    p.setPen(Qt::NoPen);
    p.fillPath(knob, QColor(255, 255, 255));

    // Лёгкая тень под кружком
    p.setPen(QPen(QColor(40, 60, 120, 60), 1));
    p.drawEllipse(x, y, knobSize, knobSize);
}