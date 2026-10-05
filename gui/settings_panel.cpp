#include "settings_panel.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>

SettingsPanel::SettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("settingsPanel");
    setMinimumWidth(320);
    setMaximumWidth(360);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setObjectName("settingsScroll");
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget *content = new QWidget(scroll);
    content->setObjectName("settingsContent");
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(14);

    setupModelSection(layout);
    setupParamsSection(layout);
    setupSystemPromptSection(layout);
    setupStreamingToggle(layout);
    setupModelInfoCard(layout);
    setupFooterButtons(layout);

    layout->addStretch();

    scroll->setWidget(content);

    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

void SettingsPanel::setupModelSection(QVBoxLayout *layout)
{
    QLabel *label = new QLabel("Какая модель отвечает", this);
    label->setObjectName("settingsLabel");
    layout->addWidget(label);

    m_modelSelector = new QComboBox(this);
    m_modelSelector->setObjectName("modelSelector");
    m_modelSelector->setMinimumHeight(44);
    m_modelSelector->addItem("llama3.1:8b");
    layout->addWidget(m_modelSelector);

    m_modelMeta = new QLabel(this);
    m_modelMeta->setObjectName("modelMeta");
    m_modelMeta->setText("локальная модель · Ollama");
    layout->addWidget(m_modelMeta);
}

void SettingsPanel::setupParamsSection(QVBoxLayout *layout)
{
    QLabel *sectionTitle = new QLabel("КАК ОТВЕЧАЕТ МОДЕЛЬ", this);
    sectionTitle->setObjectName("settingsSectionTitle");
    layout->addWidget(sectionTitle);

    // === Temperature ===
    addSliderParam(layout,
        "Креативность ответов",
        "Ниже — точнее и строже, выше — свободнее и неожиданнее",
        0, 200, 1, 70, "0.70",
        &m_temperatureSlider, &m_temperatureValue);

    connect(m_temperatureSlider, &QSlider::valueChanged, this, [this](int v){
        m_temperatureValue->setText(QString::number(v / 100.0, 'f', 2));
        emit temperatureChanged(v / 100.0);
    });

    // === Top-p ===
    addSliderParam(layout,
        "Разнообразие слов",
        "Чем выше, тем богаче и разнообразнее формулировки",
        0, 100, 1, 90, "0.90",
        &m_topPSlider, &m_topPValue);

    connect(m_topPSlider, &QSlider::valueChanged, this, [this](int v){
        m_topPValue->setText(QString::number(v / 100.0, 'f', 2));
        emit topPChanged(v / 100.0);
    });

    // === Context ===
    addSliderParam(layout,
        "Память диалога",
        "Сколько переписки модель помнит (в токенах)",
        512, 32768, 512, 8192, "8192",
        &m_ctxSlider, &m_ctxValue);

    connect(m_ctxSlider, &QSlider::valueChanged, this, [this](int v){
        m_ctxValue->setText(QString::number(v));
    });

    // === Max tokens ===
    addSliderParam(layout,
        "Максимальная длина ответа",
        "Ответ не будет длиннее этого значения (в токенах)",
        64, 8192, 64, 2048, "2048",
        &m_maxTokensSlider, &m_maxTokensValue);

    connect(m_maxTokensSlider, &QSlider::valueChanged, this, [this](int v){
        m_maxTokensValue->setText(QString::number(v));
        emit maxTokensChanged(v);
    });
}

void SettingsPanel::addSliderParam(QVBoxLayout *layout,
                                   const QString &title,
                                   const QString &hint,
                                   int min, int max, int step, int defaultValue,
                                   const QString &valueText,
                                   QSlider **outSlider,
                                   QLabel **outValue)
{
    QWidget *block = new QWidget(this);
    QVBoxLayout *blockLayout = new QVBoxLayout(block);
    blockLayout->setContentsMargins(0, 0, 0, 0);
    blockLayout->setSpacing(6);

    QHBoxLayout *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(6);

    QLabel *titleLabel = new QLabel(title, block);
    titleLabel->setObjectName("paramLabel");
    titleLabel->setWordWrap(true);

    QLabel *valueLabel = new QLabel(valueText, block);
    valueLabel->setObjectName("paramValue");
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setMinimumWidth(58);
    valueLabel->setMaximumWidth(70);

    header->addWidget(titleLabel, 1);
    header->addWidget(valueLabel, 0);
    blockLayout->addLayout(header);

    QSlider *slider = new QSlider(Qt::Horizontal, block);
    slider->setObjectName("paramSlider");
    slider->setRange(min, max);
    slider->setSingleStep(step);
    slider->setPageStep(step * 4);
    slider->setValue(defaultValue);
    blockLayout->addWidget(slider);

    QLabel *hintLabel = new QLabel(hint, block);
    hintLabel->setObjectName("paramHint");
    hintLabel->setWordWrap(true);
    blockLayout->addWidget(hintLabel);

    layout->addWidget(block);

    *outSlider = slider;
    *outValue = valueLabel;
}

void SettingsPanel::setupSystemPromptSection(QVBoxLayout *layout)
{
    QLabel *sectionTitle = new QLabel("ИНСТРУКЦИЯ ДЛЯ МОДЕЛИ", this);
    sectionTitle->setObjectName("settingsSectionTitle");
    layout->addWidget(sectionTitle);

    QLabel *hint = new QLabel("Опишите, как модель должна себя вести: роль, тон, язык ответов.", this);
    hint->setObjectName("paramHint");
    hint->setWordWrap(true);
    layout->addWidget(hint);

    m_systemPrompt = new QPlainTextEdit(this);
    m_systemPrompt->setObjectName("systemPrompt");
    m_systemPrompt->setPlainText("Ты — полезный ассистент. Отвечай кратко, по делу и на русском языке. Код оформляй в блоках.");
    m_systemPrompt->setMinimumHeight(100);
    layout->addWidget(m_systemPrompt);

    QPushButton *saveBtn = new QPushButton("Сохранить инструкцию как шаблон", this);
    saveBtn->setObjectName("secondaryButton");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setMinimumHeight(36);
    layout->addWidget(saveBtn);
}

void SettingsPanel::setupStreamingToggle(QVBoxLayout *layout)
{
    QWidget *row = new QWidget(this);
    row->setObjectName("streamingRow");
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(12, 10, 12, 10);
    rowLayout->setSpacing(10);

    QWidget *textBlock = new QWidget(row);
    QVBoxLayout *textLayout = new QVBoxLayout(textBlock);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    QLabel *title = new QLabel("Показывать ответ по мере набора", textBlock);
    title->setObjectName("paramLabel");
    title->setWordWrap(true);
    QLabel *sub = new QLabel("Текст появляется сразу, а не целиком в конце", textBlock);
    sub->setObjectName("paramHint");
    sub->setWordWrap(true);

    textLayout->addWidget(title);
    textLayout->addWidget(sub);

    m_streamToggle = new ToggleSwitch(row);
    m_streamToggle->setChecked(true);

    rowLayout->addWidget(textBlock, 1);
    rowLayout->addWidget(m_streamToggle, 0, Qt::AlignVCenter);

    layout->addWidget(row);
}

void SettingsPanel::setupModelInfoCard(QVBoxLayout *layout)
{
    QFrame *card = new QFrame(this);
    card->setObjectName("modelInfoCard");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(12, 10, 12, 10);
    cardLayout->setSpacing(8);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *modelName = new QLabel("llama3.1:8b", card);
    modelName->setObjectName("modelInfoName");
    QLabel *status = new QLabel("●  Загружена", card);
    status->setObjectName("modelInfoStatus");
    headerRow->addWidget(modelName, 1);
    headerRow->addWidget(status);
    cardLayout->addLayout(headerRow);

    QHBoxLayout *statsRow = new QHBoxLayout();
    statsRow->setSpacing(6);

    const char* titles[3] = { "Размер", "Сжатие", "RAM" };
    const char* values[3] = { "4.9 ГБ", "Q4_K_M", "6.2 ГБ" };

    for (int i = 0; i < 3; ++i) {
        QFrame *stat = new QFrame(card);
        stat->setObjectName("modelInfoStat");
        QVBoxLayout *statLayout = new QVBoxLayout(stat);
        statLayout->setContentsMargins(4, 6, 4, 6);
        statLayout->setSpacing(2);

        QLabel *t = new QLabel(titles[i], stat);
        t->setObjectName("modelInfoStatTitle");
        t->setAlignment(Qt::AlignCenter);
        QLabel *v = new QLabel(values[i], stat);
        v->setObjectName("modelInfoStatValue");
        v->setAlignment(Qt::AlignCenter);

        statLayout->addWidget(t);
        statLayout->addWidget(v);
        statsRow->addWidget(stat, 1);
    }

    cardLayout->addLayout(statsRow);
    layout->addWidget(card);
}

void SettingsPanel::setupFooterButtons(QVBoxLayout *layout)
{
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(8);

    QPushButton *saveBtn = new QPushButton("Запомнить", this);
    saveBtn->setObjectName("primaryButton");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setMinimumHeight(40);

    QPushButton *resetBtn = new QPushButton("Сбросить", this);
    resetBtn->setObjectName("secondaryButton");
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setMinimumHeight(40);
    resetBtn->setMinimumWidth(100);

    connect(resetBtn, &QPushButton::clicked, this, [this](){
        m_temperatureSlider->setValue(70);
        m_topPSlider->setValue(90);
        m_ctxSlider->setValue(8192);
        m_maxTokensSlider->setValue(2048);
        m_systemPrompt->setPlainText("Ты — полезный ассистент. Отвечай кратко, по делу и на русском языке. Код оформляй в блоках.");
    });

    row->addWidget(saveBtn, 1);
    row->addWidget(resetBtn);
    layout->addLayout(row);
}

// ============================================================
// Геттеры для MainWindow
// ============================================================

QString SettingsPanel::currentModel() const
{
    return m_modelSelector->currentText();
}

double SettingsPanel::temperature() const
{
    return m_temperatureSlider->value() / 100.0;
}

double SettingsPanel::topP() const
{
    return m_topPSlider->value() / 100.0;
}

int SettingsPanel::context() const
{
    return m_ctxSlider->value();
}

int SettingsPanel::maxTokens() const
{
    return m_maxTokensSlider->value();
}

QString SettingsPanel::systemPrompt() const
{
    return m_systemPrompt->toPlainText();
}

bool SettingsPanel::streamingEnabled() const
{
    return m_streamToggle->isChecked();
}

void SettingsPanel::setAvailableModels(const QStringList &models)
{
    QString current = m_modelSelector->currentText();
    m_modelSelector->clear();
    m_modelSelector->addItems(models);

    int idx = m_modelSelector->findText(current);
    if (idx >= 0) m_modelSelector->setCurrentIndex(idx);
}