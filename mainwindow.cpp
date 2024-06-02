#include "mainwindow.h"
#include "chatsitemwidget.h"
#include "logger_util.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QEasingCurve>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>
#include <QIcon>
#include <QListWidget>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSize>
#include <QString>
#include <chat-api.h>
#include <json.hpp>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  // Initialize logger
  auto logger = getLogger();
  spdlog::set_level(spdlog::level::debug);

  // Log MainWindow initialization
  logger->info("MainWindow initialized");

  // Customizing
  setWindowTitle("This Is Fine");
  setWindowIcon(QIcon(":/assets/thisisfinelogo.png"));
  logger->info("Window title and icon set");

  // Apply white background color to the main window
  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, Qt::white);
  setPalette(palette);
  logger->info("Applied white background color to main window");

  // Apply a style sheet to the entire application
  qApp->setStyleSheet(
      "QLabel {color: black; font-family: 'Montserrat'; }"
      "QLineEdit {color: black; font-family: 'Montserrat'; }"
      "QTextEdit {color: black; font-family: 'Montserrat'; }"
      "QPushButton { background-color: #FCCD4A; font-family: 'Montserrat'; "
      "color: white; border: 2px solid #FCCD4A; border-radius: 10px; padding: "
      "5px 15px; }"
      "QPushButton:hover { background-color: #FFD971; }"
      "QListWidget { color: black; font-family: 'Montserrat'; border: none; }"
      "QLineEdit:focus { border: 1px solid black; }");
  logger->info("Applied application-wide stylesheet");

  // Add shadows to buttons
  QList<QPushButton *> buttons = {
      ui->createChatButton,
      ui->homeButton,
      ui->chatButton,
      ui->settingsButton,
      ui->sendMessageButton,
      ui->settingsUpdateButton,
  };
  foreach (QPushButton *button, buttons) {
    addShadow(button, 20, 3);

    // Add press and release animations to buttons
    connect(button, &QPushButton::pressed, this,
            &MainWindow::animateButtonPress);
    connect(button, &QPushButton::released, this,
            &MainWindow::animateButtonRelease);
    logger->info("Connected animations to button {}",
                 button->objectName().toStdString());
  }

  // Add shadows to widgets
  addShadow(ui->welcomeWidget, 30, 3);
  addShadow(ui->chatsWidget, 30, 3);
  addShadow(ui->chatWidget, 30, 3);
  addShadow(ui->chatInput2Widget, 20, 3);
  addShadow(ui->titleLabel, 30, 3);
  addShadow(ui->chatsTitleLabel, 30, 3);
  addShadow(ui->chatTitleLabel, 30, 3);
  addShadow(ui->tokenInputWidget, 30, 3);
  logger->info("Added shadows to main widgets");

  // Animations to pages
  // connect(ui->createChatButton, &QPushButton::clicked, this, [=]() {
  // animatePageTransition(1); }); connect(ui->page3BackButton,
  // &QPushButton::clicked, this, [=]() { animatePageTransition(0); });
  // connect(ui->page3Button, &QPushButton::clicked, this, [=]() {
  // animatePageTransition(2); }); connect(ui->homeButton,
  // &QPushButton::clicked, this, [=]() { animatePageTransition(0); });
  // connect(ui->chatButton, &QPushButton::clicked, this, [=]() {
  // animatePageTransition(3); }); connect(ui->settingsButton,
  // &QPushButton::clicked, this, [=]() { animatePageTransition(4); });
  // connect(ui->page4Button, &QPushButton::clicked, this, [=]() {
  // animatePageTransition(5); });

  connect(chatCreateWindow, &ChatCreateWindow::createChat, this,
          &MainWindow::handleCreateChat);
  logger->info("Connected chat creation signal");

  ui->chatslistWidget->setStyleSheet(R"(
        QListWidget {
            background-color: white;
            border: 1px solid lightgray;
            padding: 15px;
            border-radius: 15px;
        }
        QListWidget::item {
            background-color: white;
            border: 1px solid lightgray;
            padding: 10px;
            border-radius: 5px;
        }
        QListWidget::item:selected {
            background-color: lightgray;
            color: black;
            border: 1px solid lightgray;
        }
        QListWidget::item:hover {
            background-color: lightgray;
        }
    )");
  logger->info("Set style sheet for chatslistWidget");
}

MainWindow::~MainWindow() {
  saveMessageHistory();
  auto logger = getLogger();
  logger->info("MainWindow destroyed and message history saved");
  delete ui;
}

//  Animations to pages
// void MainWindow::animatePageTransition(int newIndex) {
//     QWidget* currentWidget = ui->stackedWidget->currentWidget();
//     QWidget* nextWidget = ui->stackedWidget->widget(newIndex);

//     int duration = 1000; // Duration of the animation in milliseconds
//     QEasingCurve curve = QEasingCurve::InOutQuad;

//     // Set the new index
//     ui->stackedWidget->setCurrentIndex(newIndex);

//     // Slide out the current widget
//     QPropertyAnimation* slideOut = new QPropertyAnimation(currentWidget,
//     "pos"); slideOut->setDuration(duration);
//     slideOut->setStartValue(currentWidget->pos());
//     slideOut->setEndValue(QPoint(-currentWidget->width(),
//     currentWidget->pos().y())); slideOut->setEasingCurve(curve);

//     // Slide in the next widget
//     nextWidget->move(nextWidget->width(), nextWidget->pos().y());
//     QPropertyAnimation* slideIn = new QPropertyAnimation(nextWidget, "pos");
//     slideIn->setDuration(duration);
//     slideIn->setStartValue(QPoint(nextWidget->width(),
//     nextWidget->pos().y())); slideIn->setEndValue(QPoint(0,
//     nextWidget->pos().y())); slideIn->setEasingCurve(curve);

//     // Create an animation group to run both animations in parallel
//     QParallelAnimationGroup* group = new QParallelAnimationGroup;
//     group->addAnimation(slideOut);
//     group->addAnimation(slideIn);

//     // Start the animation
//     group->start(QAbstractAnimation::DeleteWhenStopped);
// }

// Add shadow to elements
void MainWindow::addShadow(QWidget *widget, int blur, int offset) {
  auto logger = getLogger();
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
  shadow->setBlurRadius(blur);
  shadow->setOffset(0, offset);
  shadow->setColor(Qt::gray);
  widget->setGraphicsEffect(shadow);
  logger->info("Added shadow effect to widget {}",
               widget->objectName().toStdString());
}

// Animate buttons
void MainWindow::animateButtonPress() {
  auto logger = getLogger();
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  if (button) {
    QGraphicsDropShadowEffect *effect =
        qobject_cast<QGraphicsDropShadowEffect *>(button->graphicsEffect());
    if (effect) {
      effect->setOffset(0, 0);
      effect->setBlurRadius(20);
    }
    logger->info("Animated button press for {}",
                 button->objectName().toStdString());
  }
}

void MainWindow::animateButtonRelease() {
  auto logger = getLogger();
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  if (button) {
    QGraphicsDropShadowEffect *effect =
        qobject_cast<QGraphicsDropShadowEffect *>(button->graphicsEffect());
    if (effect) {
      effect->setOffset(0, 3);
      effect->setBlurRadius(20);
    }
    logger->info("Animated button release for {}",
                 button->objectName().toStdString());
  }
}

// Settings page button
void MainWindow::on_settingsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

// Settings token button
void MainWindow::on_settingsUpdateButton_clicked()
{
    QString qtoken = ui->tokenInputWidget->text();
    std::string token = qtoken.toStdString();
    const std::string base_url = token;
    ChatClient client(base_url);
}

// Home page button
void MainWindow::on_homeButton_clicked() {
    auto logger = getLogger();
    ui->stackedWidget->setCurrentIndex(0);
    logger->info("Navigated to Home page");
}

// Chat page button
void MainWindow::on_chatButton_clicked() {
  auto logger = getLogger();
  ui->stackedWidget->setCurrentIndex(1);
  logger->info("Navigated to Chat page");
}

// Add chat
void MainWindow::handleCreateChat(const QString &title, const QString &datetime,
                                  const QString &model) {
  auto logger = getLogger();
  QString itemText = QString("%1 | %2 | %3").arg(title, datetime, model);
  QListWidgetItem *item = new QListWidgetItem(ui->chatslistWidget);
  ChatsItemWidget *chatsItemWidget = new ChatsItemWidget(itemText);

  // connect(chatsItemWidget, &ChatsItemWidget::editClicked, this,
  // &MainWindow::chatEditClicked);
  connect(chatsItemWidget, &ChatsItemWidget::deleteClicked, this,
          &MainWindow::chatDeleteClicked);

  item->setSizeHint(chatsItemWidget->sizeHint());
  ui->chatslistWidget->setItemWidget(item, chatsItemWidget);
  logger->info("Created new chat: {}", itemText.toStdString());
}

// Enter chat
void MainWindow::on_chatslistWidget_itemClicked(QListWidgetItem *item) {
  auto logger = getLogger();
  ChatsItemWidget *chatsItemWidget =
      qobject_cast<ChatsItemWidget *>(ui->chatslistWidget->itemWidget(item));

  QString itemTitle = chatsItemWidget->getTitle();
  QStringList title = itemTitle.split(" | ");
  ui->stackedWidget->setCurrentIndex(2);
  ui->chatTitleLabel->setText(title[0]);
  ui->chatModelLabel->setText(title[2]);
  logger->info("Entered chat: {}", itemTitle.toStdString());
}

// Edit chat
// void MainWindow::chatEditClicked() {
//     ChatsItemWidget *chatsItemWidget = qobject_cast<ChatsItemWidget
//     *>(sender()); QListWidgetItem *item =
//     ui->chatslistWidget->itemAt(chatsItemWidget->pos());

// }

// Delete chat
void MainWindow::chatDeleteClicked() {
    auto logger = getLogger();
    ChatsItemWidget *chatsItemWidget = qobject_cast<ChatsItemWidget *>(sender());
    QListWidgetItem *item = ui->chatslistWidget->itemAt(chatsItemWidget->pos());
    if (QMessageBox::question(this, tr("Deleting a chat"), tr("Are you sure you want to delete this chat?")) == QMessageBox::Yes) {
        logger->info("Deleted chat: {}", chatsItemWidget->getTitle().toStdString());
        delete item;
        delete chatsItemWidget;
    } else {
        logger->info("Cancelled deletion of chat: {}", chatsItemWidget->getTitle().toStdString());
    }
}

// Chat create button
void MainWindow::on_createChatButton_clicked() {
    auto logger = getLogger();
    chatCreateWindow->show();
    logger->info("Chat create window shown");
}

// Chat send button
void MainWindow::on_sendMessageButton_clicked() {
    auto logger = getLogger();
    QString message = ui->chatInput2Widget->text();
    if (!message.isEmpty()) {
        addMessage(true, message);
        addMessage(false, message);
        ui->chatInput2Widget->clear();
        logger->info("Message added: {}", message.toStdString());
    } else {
        logger->warn("Attempted to add an empty message");
    }
}

// Chat add message
void MainWindow::addMessage(bool isUser, const QString &message) {
    auto logger = getLogger();
    QString sender = isUser ? "You" : "Bot";

    ui->chatWindowWidget->append(
        QString(
            "<div style='margin: 10px; padding: 15px;'><b> %1</b><br>%2</div>")
            .arg(sender, message));
    logger->info("Added message from {}: {}", sender.toStdString(),
                 message.toStdString());
}

// History
void MainWindow::loadMessageHistory() {
    auto logger = getLogger();
    // Implement loading logic here
    logger->info("Loaded message history");
}

void MainWindow::saveMessageHistory() {
    auto logger = getLogger();
    // Implement saving logic here
    logger->info("Saved message history");
}
