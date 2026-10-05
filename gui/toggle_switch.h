#ifndef TOGGLE_SWITCH_H
#define TOGGLE_SWITCH_H

#include <QWidget>
#include <QPropertyAnimation>

class ToggleSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal knobPosition READ knobPosition WRITE setKnobPosition)

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);

    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

    qreal knobPosition() const { return m_knobPosition; }
    void setKnobPosition(qreal pos);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    QSize sizeHint() const override { return QSize(46, 26); }

private:
    bool m_checked = false;
    qreal m_knobPosition = 0.0;
    QPropertyAnimation *m_animation = nullptr;
};

#endif