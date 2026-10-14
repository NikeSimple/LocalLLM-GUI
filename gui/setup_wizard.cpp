#include "setup_wizard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>

SetupWizard::SetupWizard(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Мастер настройки LocalLLM-GUI");
    setMinimumSize(520, 380);
    setModal(true);

    m_installer = new OllamaInstaller(this);
    m_downloader = new ModelDownloader(this);

    connect(m_installer, &OllamaInstaller::downloadProgress,
            this, &SetupWizard::onDownloadProgress);
    connect(m_installer, &OllamaInstaller::downloadFinished,
            this, &SetupWizard::onDownloadFinished);
    connect(m_installer, &OllamaInstaller::installFinished,
            this, &SetupWizard::onInstallFinished);
    connect(m_installer, &OllamaInstaller::errorOccurred,
            this, &SetupWizard::onError);

    connect(m_downloader, &ModelDownloader::progress,
            this, &SetupWizard::onModelProgress);
    connect(m_downloader, &ModelDownloader::finished,
            this, &SetupWizard::onModelFinished);
    connect(m_downloader, &ModelDownloader::error,
            this, &SetupWizard::onError);

    m_stack = new QStackedWidget(this);

    buildStep1();
    buildStep2();
    buildStep3();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_stack);

    if (m_installer->isOllamaInstalled()) {
        m_stack->setCurrentIndex(1);
        m_step2Status->setText("Ollama уже установлена. Выберите модель для скачивания.");
    }
}

void SetupWizard::buildStep1()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    QLabel *title = new QLabel("<h2>Шаг 1 из 2 — Установка Ollama</h2>", page);
    layout->addWidget(title);

    QLabel *info = new QLabel(
        "Ollama — это программа, которая запускает нейросети на вашем ПК.\n\n"
        "Приложение не нашло Ollama на этом компьютере.\n"
        "Установщик (~700 МБ) будет скачан с официального сайта ollama.com "
        "и установлен автоматически.",
        page);
    info->setWordWrap(true);
    layout->addWidget(info);

    layout->addStretch();

    m_step1Progress = new QProgressBar(page);
    m_step1Progress->setVisible(false);
    layout->addWidget(m_step1Progress);

    m_step1Status = new QLabel("", page);
    m_step1Status->setWordWrap(true);
    layout->addWidget(m_step1Status);

    QHBoxLayout *btns = new QHBoxLayout();
    m_step1InstallBtn = new QPushButton("Установить Ollama", page);
    m_step1InstallBtn->setMinimumHeight(40);
    connect(m_step1InstallBtn, &QPushButton::clicked,
            this, &SetupWizard::onStep1InstallClicked);
    btns->addWidget(m_step1InstallBtn);

    m_step1SkipBtn = new QPushButton("Пропустить", page);
    m_step1SkipBtn->setMinimumHeight(40);
    connect(m_step1SkipBtn, &QPushButton::clicked,
            this, &SetupWizard::onStep1SkipClicked);
    btns->addWidget(m_step1SkipBtn);

    layout->addLayout(btns);

    m_stack->addWidget(page);
}

void SetupWizard::buildStep2()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    QLabel *title = new QLabel("<h2>Шаг 2 из 2 — Скачивание модели</h2>", page);
    layout->addWidget(title);

    QLabel *info = new QLabel(
        QString("Для работы нужна хотя бы одна нейросеть. "
                "Мы предлагаем скачать <b>%1</b> (~5 ГБ).\n\n"
                "Модель скачается автоматически и будет доступна в списке.")
            .arg(m_currentModel),
        page);
    info->setWordWrap(true);
    layout->addWidget(info);

    layout->addStretch();

    m_step2Progress = new QProgressBar(page);
    m_step2Progress->setVisible(false);
    m_step2Progress->setRange(0, 100);
    layout->addWidget(m_step2Progress);

    m_step2Status = new QLabel("Ollama установлена. Скачать модель?", page);
    m_step2Status->setWordWrap(true);
    layout->addWidget(m_step2Status);

    QHBoxLayout *btns = new QHBoxLayout();
    m_step2DownloadBtn = new QPushButton("Скачать модель", page);
    m_step2DownloadBtn->setMinimumHeight(40);
    connect(m_step2DownloadBtn, &QPushButton::clicked,
            this, &SetupWizard::onStep2DownloadClicked);
    btns->addWidget(m_step2DownloadBtn);

    m_step2SkipBtn = new QPushButton("Пропустить", page);
    m_step2SkipBtn->setMinimumHeight(40);
    connect(m_step2SkipBtn, &QPushButton::clicked,
            this, &SetupWizard::onStep2SkipClicked);
    btns->addWidget(m_step2SkipBtn);

    layout->addLayout(btns);

    m_stack->addWidget(page);
}

void SetupWizard::buildStep3()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    m_step3Label = new QLabel("<h2>Готово!</h2>", page);
    layout->addWidget(m_step3Label);

    QLabel *info = new QLabel(
        "Настройка завершена. Теперь вы можете работать с приложением:\n"
        "выбирать модели, отправлять сообщения, сохранять диалоги.",
        page);
    info->setWordWrap(true);
    layout->addWidget(info);

    layout->addStretch();

    m_step3FinishBtn = new QPushButton("Начать работу", page);
    m_step3FinishBtn->setMinimumHeight(40);
    connect(m_step3FinishBtn, &QPushButton::clicked, this, [this](){
        emit setupCompleted();
        accept();
    });
    layout->addWidget(m_step3FinishBtn);

    m_stack->addWidget(page);
}

void SetupWizard::onStep1InstallClicked()
{
    m_step1InstallBtn->setEnabled(false);
    m_step1SkipBtn->setEnabled(false);
    m_step1Progress->setVisible(true);
    m_step1Progress->setRange(0, 100);
    m_step1Status->setText("Скачивание установщика...");

    m_installer->downloadInstaller();
}

void SetupWizard::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        int percent = static_cast<int>((received * 100) / total);
        m_step1Progress->setValue(percent);
        double receivedMb = received / (1024.0 * 1024.0);
        double totalMb = total / (1024.0 * 1024.0);
        m_step1Status->setText(
            QString("Скачано: %1 МБ из %2 МБ (%3%)")
                .arg(receivedMb, 0, 'f', 1)
                .arg(totalMb, 0, 'f', 1)
                .arg(percent));
    }
}

void SetupWizard::onDownloadFinished(const QString &path)
{
    m_step1Status->setText("Установка Ollama...");
    m_step1Progress->setRange(0, 0);

    if (!m_installer->runSilentInstall(path)) {
        onError("Не удалось запустить установку");
    }
}

void SetupWizard::onInstallFinished(bool success)
{
    if (success) {
        m_step1Progress->setRange(0, 100);
        m_step1Progress->setValue(100);
        m_step1Status->setText("Ollama установлена. Переходим к следующему шагу...");

        m_installer->startOllamaServe();

        QTimer::singleShot(3000, this, [this](){
            m_stack->setCurrentIndex(1);
        });
    } else {
        onError("Установка не удалась. Установите Ollama вручную с ollama.com");
    }
}

void SetupWizard::onStep1SkipClicked()
{
    m_stack->setCurrentIndex(1);
}

void SetupWizard::onStep2DownloadClicked()
{
    m_step2DownloadBtn->setEnabled(false);
    m_step2SkipBtn->setEnabled(false);
    m_step2Progress->setVisible(true);
    m_step2Progress->setRange(0, 100);
    m_step2Status->setText("Скачивание модели...");

    m_downloader->downloadModel("http://localhost:11434", m_currentModel);
}

void SetupWizard::onModelProgress(const QString &status, int percent)
{
    m_step2Status->setText(status);
    if (percent >= 0) {
        m_step2Progress->setValue(percent);
    }
}

void SetupWizard::onModelFinished(const QString &modelName)
{
    m_step2Progress->setValue(100);
    m_step2Status->setText(QString("Модель %1 успешно скачана!").arg(modelName));

    QTimer::singleShot(1500, this, [this](){
        m_stack->setCurrentIndex(2);
    });
}

void SetupWizard::onStep2SkipClicked()
{
    m_stack->setCurrentIndex(2);
}

void SetupWizard::onError(const QString &message)
{
    m_step1Progress->setVisible(false);
    m_step2Progress->setVisible(false);

    QMessageBox::warning(this, "Ошибка", message);

    m_step1InstallBtn->setEnabled(true);
    m_step1SkipBtn->setEnabled(true);
    m_step2DownloadBtn->setEnabled(true);
    m_step2SkipBtn->setEnabled(true);
}
