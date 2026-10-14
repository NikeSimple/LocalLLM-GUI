#ifndef SETTINGS_PANEL_H
#define SETTINGS_PANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStringList>
#include "toggle_switch.h"

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);

    QString currentModel() const;
    double temperature() const;
    double topP() const;
    int context() const;
    int maxTokens() const;
    QString systemPrompt() const;
    bool streamingEnabled() const;

    void setAvailableModels(const QStringList &models);
    void applySystemPrompt(const QString &text);

signals:
    void temperatureChanged(double value);
    void topPChanged(double value);
    void maxTokensChanged(int value);

private:
    void setupModelSection(QVBoxLayout *layout);
    void setupParamsSection(QVBoxLayout *layout);
    void setupSystemPromptSection(QVBoxLayout *layout);
    void setupStreamingToggle(QVBoxLayout *layout);
    void setupModelInfoCard(QVBoxLayout *layout);
    void setupFooterButtons(QVBoxLayout *layout);

    void addSliderParam(QVBoxLayout *layout,
                        const QString &title,
                        const QString &hint,
                        int min, int max, int step, int defaultValue,
                        const QString &valueText,
                        QSlider **outSlider,
                        QLabel **outValue);

    QComboBox *m_modelSelector = nullptr;
    QLabel *m_modelMeta = nullptr;

    QSlider *m_temperatureSlider = nullptr;
    QLabel *m_temperatureValue = nullptr;
    QSlider *m_topPSlider = nullptr;
    QLabel *m_topPValue = nullptr;
    QSlider *m_ctxSlider = nullptr;
    QLabel *m_ctxValue = nullptr;
    QSlider *m_maxTokensSlider = nullptr;
    QLabel *m_maxTokensValue = nullptr;

    QPlainTextEdit *m_systemPrompt = nullptr;
    ToggleSwitch *m_streamToggle = nullptr;
};

#endif