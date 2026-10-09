#include "dialog_list_widget.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QTime>
#include <QIcon>

DialogListWidget::DialogListWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("dialogListWidget");
    setMinimumWidth(280);
    setMaximumWidth(340);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    setupNewDialogButton(layout);
    setupSearch(layout);
    setupGroupHeader(layout, "ДИАЛОГИ");
    setupDialogsList(layout);

    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("footerSeparator");
    layout->addWidget(line);

    setupProfileFooter(layout);
}

void DialogListWidget::setupNewDialogButton(QVBoxLayout *layout)
{
    m_newDialogButton = new QPushButton("Новый диалог", this);
    m_newDialogButton->setObjectName("newDialogButton");
    m_newDialogButton->setIcon(QIcon(":/icons/plus.svg"));
    m_newDialogButton->setIconSize(QSize(18, 18));
    m_newDialogButton->setCursor(Qt::PointingHandCursor);
    m_newDialogButton->setMinimumHeight(44);
    connect(m_newDialogButton, &QPushButton::clicked,
            this, &DialogListWidget::onNewDialogClicked);
    layout->addWidget(m_newDialogButton);
}

void DialogListWidget::setupSearch(QVBoxLayout *layout)
{
    m_searchField = new QLineEdit(this);
    m_searchField->setObjectName("searchField");
    m_searchField->setPlaceholderText("Поиск по диалогам");
    m_searchField->setMinimumHeight(38);
    m_searchField->setClearButtonEnabled(true);
    m_searchField->addAction(QIcon(":/icons/search.svg"),
                              QLineEdit::LeadingPosition);
    connect(m_searchField, &QLineEdit::textChanged,
            this, &DialogListWidget::onSearchChanged);
    layout->addWidget(m_searchField);
}

void DialogListWidget::setupGroupHeader(QVBoxLayout *layout, const QString &title)
{
    QLabel *header = new QLabel(title, this);
    header->setObjectName("groupHeader");
    layout->addWidget(header);
}

void DialogListWidget::setupDialogsList(QVBoxLayout *layout)
{
    m_list = new QListWidget(this);
    m_list->setObjectName("dialogsList");
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setSpacing(4);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_list->setWordWrap(true);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setTextElideMode(Qt::ElideNone);
    m_list->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);

    connect(m_list, &QListWidget::itemClicked,
            this, &DialogListWidget::onItemClicked);

    layout->addWidget(m_list, 1);
}

void DialogListWidget::addDialogCard(const QString &title,
                                     const QString &preview,
                                     const QString &time)
{
    QListWidgetItem *item = new QListWidgetItem(m_list);
    item->setSizeHint(QSize(0, 76));

    QWidget *card = new QWidget(m_list);
    card->setObjectName("dialogCard");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(12, 10, 12, 10);
    cardLayout->setSpacing(4);

    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(8);

    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("dialogTitle");
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QLabel *timeLabel = new QLabel(time, card);
    timeLabel->setObjectName("dialogTime");
    timeLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    topRow->addWidget(titleLabel, 1);
    topRow->addWidget(timeLabel);

    QLabel *previewLabel = new QLabel(preview, card);
    previewLabel->setObjectName("dialogPreview");
    previewLabel->setWordWrap(true);

    cardLayout->addLayout(topRow);
    cardLayout->addWidget(previewLabel);

    m_list->setItemWidget(item, card);
}

void DialogListWidget::addDialogCardStatic(const QString &title,
                                            const QString &preview,
                                            const QString &time)
{
    addDialogCard(title, preview, time);
}

void DialogListWidget::updateDialogTitle(int index, const QString &newTitle)
{
    if (index < 0 || index >= m_list->count()) return;
    QListWidgetItem *item = m_list->item(index);
    QWidget *card = m_list->itemWidget(item);
    if (!card) return;

    QList<QLabel*> labels = card->findChildren<QLabel*>();
    for (QLabel *l : labels) {
        if (l->objectName() == "dialogTitle") { l->setText(newTitle); break; }
    }
}

void DialogListWidget::updateDialogPreview(int index, const QString &preview)
{
    if (index < 0 || index >= m_list->count()) return;
    QListWidgetItem *item = m_list->item(index);
    QWidget *card = m_list->itemWidget(item);
    if (!card) return;

    QString trimmed = preview;
    if (trimmed.length() > 60) trimmed = trimmed.left(60) + "…";
    trimmed.replace("\n", " ");

    QList<QLabel*> labels = card->findChildren<QLabel*>();
    for (QLabel *l : labels) {
        if (l->objectName() == "dialogPreview") { l->setText(trimmed); break; }
    }
}

void DialogListWidget::setupProfileFooter(QVBoxLayout *layout)
{
    QWidget *footer = new QWidget(this);
    footer->setObjectName("profileFooter");
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(0, 6, 0, 0);
    footerLayout->setSpacing(8);

    m_avatar = new QLabel("B", footer);
    m_avatar->setObjectName("avatar");
    m_avatar->setAlignment(Qt::AlignCenter);
    m_avatar->setFixedSize(36, 36);

    QWidget *userInfo = new QWidget(footer);
    QVBoxLayout *userLayout = new QVBoxLayout(userInfo);
    userLayout->setContentsMargins(0, 0, 0, 0);
    userLayout->setSpacing(0);

    m_userName = new QLabel("boba.mix999", userInfo);
    m_userName->setObjectName("userName");
    m_userStatus = new QLabel("SQLite · локально", userInfo);
    m_userStatus->setObjectName("userStatus");

    userLayout->addWidget(m_userName);
    userLayout->addWidget(m_userStatus);

    m_themeButton = new QPushButton(footer);
    m_themeButton->setObjectName("iconButton");
    m_themeButton->setIcon(QIcon(":/icons/moon.svg"));
    m_themeButton->setIconSize(QSize(20, 20));
    m_themeButton->setFixedSize(36, 36);
    m_themeButton->setCursor(Qt::PointingHandCursor);
    m_themeButton->setToolTip("Переключить тему");
    connect(m_themeButton, &QPushButton::clicked,
            this, &DialogListWidget::themeToggleRequested);

    m_settingsButton = new QPushButton(footer);
    m_settingsButton->setObjectName("iconButton");
    m_settingsButton->setIcon(QIcon(":/icons/settings.svg"));
    m_settingsButton->setIconSize(QSize(20, 20));
    m_settingsButton->setFixedSize(36, 36);
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    m_settingsButton->setToolTip("Настройки");
    connect(m_settingsButton, &QPushButton::clicked,
            this, &DialogListWidget::settingsRequested);

    footerLayout->addWidget(m_avatar);
    footerLayout->addWidget(userInfo, 1);
    footerLayout->addWidget(m_themeButton);
    footerLayout->addWidget(m_settingsButton);

    layout->addWidget(footer);
}

int DialogListWidget::currentRow() const { return m_list->currentRow(); }
int DialogListWidget::count() const { return m_list->count(); }
void DialogListWidget::setCurrentRow(int row) { m_list->setCurrentRow(row); }
void DialogListWidget::createNewDialog() { onNewDialogClicked(); }

void DialogListWidget::setUserName(const QString &name)
{
    if (m_userName) m_userName->setText(name);
}

void DialogListWidget::clearAllDialogs()
{
    m_list->clear();
}

void DialogListWidget::onNewDialogClicked()
{
    QString time = QTime::currentTime().toString("HH:mm");
    addDialogCard("Новый диалог", "Пустой диалог", time);

    int newIndex = m_list->count() - 1;
    m_list->setCurrentRow(newIndex);

    emit newDialogRequested();
    emit dialogSelected(newIndex);
}

void DialogListWidget::onItemClicked()
{
    int row = m_list->currentRow();
    emit dialogSelected(row);
}

void DialogListWidget::onSearchChanged(const QString &text)
{
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem *item = m_list->item(i);
        QWidget *card = m_list->itemWidget(item);
        QString searchable;
        if (card) {
            QList<QLabel*> labels = card->findChildren<QLabel*>();
            for (QLabel *l : labels) searchable += l->text() + " ";
        }
        item->setHidden(!text.isEmpty() &&
                        !searchable.contains(text, Qt::CaseInsensitive));
    }
}