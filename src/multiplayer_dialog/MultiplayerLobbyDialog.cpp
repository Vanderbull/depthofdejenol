#include "MultiplayerLobbyDialog.h"
#include "src/network_manager/NetworkManager.h"
#include "src/audio/audioManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QIntValidator>
#include <QProcess>
#include <QDir>

MultiplayerLobbyDialog::MultiplayerLobbyDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Multiplayer Lobby");
    setMinimumSize(600, 500);
    setupUI();
    setupConnections();
}

MultiplayerLobbyDialog::~MultiplayerLobbyDialog() {}

void MultiplayerLobbyDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- Connection group ---
    QGroupBox *connGroup = new QGroupBox("Server Connection");
    QFormLayout *connForm = new QFormLayout();

    m_hostInput = new QLineEdit("127.0.0.1");
    m_portInput = new QLineEdit("12345");
    m_portInput->setValidator(new QIntValidator(1, 65535, this));
    m_usernameInput = new QLineEdit("Player");

    connForm->addRow("Host:", m_hostInput);
    connForm->addRow("Port:", m_portInput);
    connForm->addRow("Username:", m_usernameInput);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_connectButton = new QPushButton("Connect");
    m_disconnectButton = new QPushButton("Disconnect");
    m_disconnectButton->setEnabled(false);
    m_startServerButton = new QPushButton("Start Server");
    btnLayout->addWidget(m_connectButton);
    btnLayout->addWidget(m_disconnectButton);
    btnLayout->addWidget(m_startServerButton);
    connForm->addRow("", btnLayout);

    m_statusLabel = new QLabel("Disconnected");
    m_statusLabel->setStyleSheet("color: red;");
    connForm->addRow("Status:", m_statusLabel);

    connGroup->setLayout(connForm);
    mainLayout->addWidget(connGroup);

    // --- Players group ---
    QGroupBox *playersGroup = new QGroupBox("Online Players");
    QVBoxLayout *playersLayout = new QVBoxLayout();
    m_playerList = new QListWidget();
    m_playerList->setMaximumHeight(100);
    playersLayout->addWidget(m_playerList);
    playersGroup->setLayout(playersLayout);
    mainLayout->addWidget(playersGroup);

    // --- Chat group ---
    QGroupBox *chatGroup = new QGroupBox("Chat");
    QVBoxLayout *chatLayout = new QVBoxLayout();

    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    chatLayout->addWidget(m_chatDisplay);

    QHBoxLayout *chatInputLayout = new QHBoxLayout();
    m_chatInput = new QLineEdit();
    m_chatInput->setPlaceholderText("Type a message...");
    m_chatInput->setEnabled(false);
    m_sendButton = new QPushButton("Send");
    m_sendButton->setEnabled(false);
    chatInputLayout->addWidget(m_chatInput);
    chatInputLayout->addWidget(m_sendButton);
    chatLayout->addLayout(chatInputLayout);

    chatGroup->setLayout(chatLayout);
    mainLayout->addWidget(chatGroup);

    // --- Close button ---
    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_moveButton = new QPushButton("Move");
    m_moveButton->setEnabled(false);
    m_fightButton = new QPushButton("Fight");
    m_fightButton->setEnabled(false);
    actionLayout->addWidget(m_moveButton);
    actionLayout->addWidget(m_fightButton);
    mainLayout->addLayout(actionLayout);

    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    mainLayout->addWidget(closeButton);
}

void MultiplayerLobbyDialog::setupConnections() {
    NetworkManager *nm = NetworkManager::instance();

    connect(m_connectButton, &QPushButton::clicked, this, &MultiplayerLobbyDialog::onConnectClicked);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MultiplayerLobbyDialog::onDisconnectClicked);
    connect(m_startServerButton, &QPushButton::clicked, this, &MultiplayerLobbyDialog::onStartServerClicked);
    connect(m_sendButton, &QPushButton::clicked, this, &MultiplayerLobbyDialog::onChatSend);
    connect(m_chatInput, &QLineEdit::returnPressed, this, &MultiplayerLobbyDialog::onChatSend);
    connect(m_moveButton, &QPushButton::clicked, this, &MultiplayerLobbyDialog::onMoveClicked);
    connect(m_fightButton, &QPushButton::clicked, this, &MultiplayerLobbyDialog::onFightClicked);

    connect(nm, &NetworkManager::connected, this, &MultiplayerLobbyDialog::onConnected);
    connect(nm, &NetworkManager::disconnected, this, &MultiplayerLobbyDialog::onDisconnected);
    connect(nm, &NetworkManager::chatReceived, this, &MultiplayerLobbyDialog::onChatReceived);
    connect(nm, &NetworkManager::playerJoined, this, &MultiplayerLobbyDialog::onPlayerJoined);
    connect(nm, &NetworkManager::playerLeft, this, &MultiplayerLobbyDialog::onPlayerLeft);
    connect(nm, &NetworkManager::connectionAttemptFailed, this, &MultiplayerLobbyDialog::onConnectionAttemptFailed);
    connect(nm, &NetworkManager::connectionReestablished, this, &MultiplayerLobbyDialog::onConnectionReestablished);
    connect(nm, &NetworkManager::zoneUpdateReceived, this, &MultiplayerLobbyDialog::onZoneUpdateReceived);
}

void MultiplayerLobbyDialog::onConnectClicked() {
    QString host = m_hostInput->text().trimmed();
    QString portStr = m_portInput->text().trimmed();
    QString username = m_usernameInput->text().trimmed();

    if (host.isEmpty() || portStr.isEmpty() || username.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please fill in all fields.");
        return;
    }

    bool ok;
    quint16 port = portStr.toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid Port", "Port must be a number between 1 and 65535.");
        return;
    }

    NetworkManager::instance()->sendAction("identify", {{"username", username}});
    NetworkManager::instance()->connectToServer(host, port);
    updateStatus("Connecting...");
}

void MultiplayerLobbyDialog::onStartServerClicked() {
    QString portStr = m_portInput->text().trimmed();
    bool ok;
    quint16 port = portStr.toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid Port", "Port must be a number between 1 and 65535.");
        return;
    }

    QString serverPath = QDir::currentPath() + "/tools/multiplayer/ref_server.py";
    if (!QFile::exists(serverPath)) {
        QMessageBox::warning(this, "Server Not Found", "ref_server.py not found at: " + serverPath);
        return;
    }

    QProcess *serverProcess = new QProcess(this);
    serverProcess->start("python3", QStringList() << serverPath << "--port" << QString::number(port));
    connect(serverProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) {
        if (exitCode != 0) {
            QMessageBox::warning(this, "Server Error", "Reference server crashed or failed to start.");
        }
    });
    QMessageBox::information(this, "Server Started", "Reference server started on port " + QString::number(port) + ".");
}

void MultiplayerLobbyDialog::onMoveClicked() {
    if (!m_connected) return;
    NetworkManager::instance()->sendAction("move", {{"direction", "forward"}});
}

void MultiplayerLobbyDialog::onFightClicked() {
    if (!m_connected) return;
    NetworkManager::instance()->sendAction("fight", {{"target", "nearest"}});
}

void MultiplayerLobbyDialog::onDisconnectClicked() {
    NetworkManager::instance()->sendAction("disconnect", {});
    updateStatus("Disconnected");
    m_connected = false;
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
    m_chatInput->setEnabled(false);
    m_sendButton->setEnabled(false);
}

void MultiplayerLobbyDialog::onChatSend() {
    if (!m_connected) return;
    QString msg = m_chatInput->text().trimmed();
    if (msg.isEmpty()) return;

    NetworkManager::instance()->sendAction("chat", {{"message", msg}});
    m_chatInput->clear();
}

void MultiplayerLobbyDialog::onConnected() {
    m_connected = true;
    updateStatus("Connected");
    m_connectButton->setEnabled(false);
    m_disconnectButton->setEnabled(true);
    m_chatInput->setEnabled(true);
    m_sendButton->setEnabled(true);
    m_moveButton->setEnabled(true);
    m_fightButton->setEnabled(true);
    refreshPlayerList();
}

void MultiplayerLobbyDialog::onDisconnected() {
    m_connected = false;
    updateStatus("Disconnected");
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
    m_chatInput->setEnabled(false);
    m_sendButton->setEnabled(false);
    m_moveButton->setEnabled(false);
    m_fightButton->setEnabled(false);
}

void MultiplayerLobbyDialog::onChatReceived(const QString &from, const QString &message) {
    addChatMessage(from, message);
    audioManager::instance()->playSound("qrc:/sounds/chat_notify.wav");
}

void MultiplayerLobbyDialog::onPlayerJoined(const QString &playerName) {
    m_playerList->addItem(playerName + " joined");
    refreshPlayerList();
}

void MultiplayerLobbyDialog::onPlayerLeft(const QString &playerName) {
    m_playerList->addItem(playerName + " left");
    refreshPlayerList();
}

void MultiplayerLobbyDialog::onConnectionAttemptFailed(int attempt, int backoffMs) {
    updateStatus(QString("Reconnecting... attempt %1 (next in %2ms)").arg(attempt).arg(backoffMs));
}

void MultiplayerLobbyDialog::onConnectionReestablished() {
    updateStatus("Connected (reestablished)");
}

void MultiplayerLobbyDialog::onZoneUpdateReceived(const QVariantMap &zoneData) {
    QString actor = zoneData.value("actor").toString();
    QString action = zoneData.value("action").toString();
    if (!actor.isEmpty() && !action.isEmpty()) {
        addChatMessage("Zone", QString("%1: %2").arg(actor).arg(action));
    }
}

void MultiplayerLobbyDialog::updateStatus(const QString &status) {
    m_statusLabel->setText(status);
    if (status.contains("Connected")) {
        m_statusLabel->setStyleSheet("color: green;");
    } else if (status.contains("Disconnected")) {
        m_statusLabel->setStyleSheet("color: red;");
    } else {
        m_statusLabel->setStyleSheet("color: orange;");
    }
}

void MultiplayerLobbyDialog::addChatMessage(const QString &from, const QString &message) {
    QString formatted = QString("[%1] %2: %3")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(from, message);
    m_chatDisplay->append(formatted);
}

void MultiplayerLobbyDialog::refreshPlayerList() {
    m_playerList->clear();
    m_playerList->addItem("You are connected");
}
