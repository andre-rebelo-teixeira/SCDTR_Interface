#include "scdtr_interface.h"
#include "ui_scdtr_interface.h"

#include <json.hpp>

SCDTR_Interface::SCDTR_Interface(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SCDTR_Interface) {
  ui->setupUi(this);

  serial_port = new QSerialPort();   // this->ui->console->setReadOnly(true);
  this->ui->console->document()->setMaximumBlockCount(200);

  this->ui->command_out->setReadOnly(true);
  this->ui->command_out->setMaximumBlockCount(200);

  this->presence_timer = new QTimer();
  connect(this->presence_timer, &QTimer::timeout, this,
          &SCDTR_Interface::send_presence);

  this->udp_server_timer = new QTimer();
  connect(this->udp_server_timer, &QTimer::timeout, this,
          &SCDTR_Interface::send_udp_server_presence);

  this->initialize_command_in();
  this->ui->udp_server_ip_combo->setEditable(true);
  this->ui->udp_server_port_combo->setEditable(true);

  _interfaceSocket = new QUdpSocket();
}

SCDTR_Interface::~SCDTR_Interface() {
    write_to_csv();
    delete ui;
}

// private functions

void SCDTR_Interface::write_to_csv() {

    auto clock = QDateTime::currentDateTime();
    QString date = clock.date().toString();

    foreach (auto const & sig_name , signal_map.keys()) {
        auto values = signal_map[sig_name];

        QFile file (date + sig_name +  ".csv");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            std::cout << "Error creating file for " << sig_name.toStdString() << std::endl;
            continue;
        }

        QTextStream f_(&file);

        f_ << "timestamp , " << "sig_val" << '\n';

        foreach(auto const & val , values) {
            f_ << val.first << "," << val.second << '\n';
        }

        file.close();

    }
}

void SCDTR_Interface::initialize_command_in() {
    this->ui->udp_server_ip_combo->addItem("127.0.0.1");
    this->ui->udp_server_port_combo->addItem("58000");




  this->desired_commands["d <i> <val>"] =
      "Set directly the duty cycle of luminaire i";
  this->desired_commands["g d <i>"] = "Get current duty cycle of luminaire i";
  this->desired_commands["r <i> <val>"] =
      "Set the illuminance reference of luminaire i";
  this->desired_commands["g r <i>"] =
      "Get current illuminance reference of luminaire i";
  this->desired_commands["g l <i>"] = "Measure the illuminance of luminaire i";
  this->desired_commands["o <i> <val>"] =
      "Set the current occupancy state of desk <i>";
  this->desired_commands["g o <i>"] =
      "Get the current occupancy statE of desk <i>";
  this->desired_commands["a <i> <val>"] = "Set anti-windup state of desk <i>";
  this->desired_commands["g a <i>"] = "Get anti-windup state of desk <i>";
  this->desired_commands["k <i> <val>"] = "Set feedback on/off of desk <i>";
  this->desired_commands["g k <i>"] = "Get feedback state of desk <i>";
  this->desired_commands["g x <i>"] =
      "Get current external illuminance of desk <i>";
  this->desired_commands["g p <i>"] =
      "Get instantaneous power consumption of desk <i>";
  this->desired_commands["g t <i>"] =
      "Get the elapsed time since the last restart";
  this->desired_commands["s <x> <i>"] =
      "Start the stream of the real-time variable <x> of desk <i>. <x> can be "
      "'l' or 'd'.";
  this->desired_commands["S <x> <i>"] =
      "Stop the stream of the real-time variable <x> of desk <i>. <x> can be "
      "'l' or 'd'.";
  this->desired_commands["g b <x> <i>"] =
      "Get the last minute buffer of the variable <x> of the desk <i>. <x> can "
      "be 'l' or 'd'.";

  this->desired_commands["g e <i>"] =
      "Get the average energy consumption at the desk <i> since the last "
      "system restart.";
  this->desired_commands["g v <i>"] = "Get the average visibility error at "
                                      "desk <i> since the last system restart";
  this->desired_commands["g f <i>"] = "Get the average flicker error on desk "
                                      "<i> since the last system restart.";

  this->desired_commands["g O <i>"] = "<val> is a number that expresses the lower bound of illuminance for the occupied state on the desk <i> in lux.";
  this->desired_commands["O <i> <val>"] = "Set lower bound on illumincance at state desk <i>";
      this->desired_commands["g U <i>"] = "Get lower bound for unoccupied desk <i>";
      this->desired_commands["U <i> <val>"] =  "Set lower bound on illuminance for the unoccupied state at dest <i>";
  this->desired_commands["g L <i>"] = "Get energy current illuminance lower bound at desk <i>";
      this->desired_commands["g c <i>"] = "<val> is the number that expresses current cost of energy";
      this->desired_commands["c <i> <val>"]  = "<val> is the number that expresss current cost in energy";
  this->desired_commands["r"] = "Reset all values and recalibrate";



  auto keys = this->desired_commands.keys();
  for (auto const &com : keys) {
    this->ui->command_in->addItem(com);
  }
}

void SCDTR_Interface::send_presence() {
  auto clock = QDateTime::currentDateTime();

  json j;
  j["presence"] = clock.toMSecsSinceEpoch();

  std::string send_string = j.dump();

  this->_udpSocket.write((const char *)send_string.c_str(),
                         strlen(send_string.c_str()));
}

void SCDTR_Interface::handleRead(QString msg) {
  QTextCharFormat format_;

    this->ui->console->moveCursor(QTextCursor::End);

    if (msg.contains("[RESPONSE]"))
    {
        this->ui->command_out->appendPlainText(msg);

        QString msg_cpy = msg.remove("[RESPONSE]");
        // Stream variabable
        if (msg.startsWith("s ")) {
            std::cout << "WIll save to an array" << std::endl;
            // split by  the \n
            QStringList msg_split = msg_cpy.split(" ");
            if (msg_split.size() != 5) {
                return;
            }

            QString msg_name = msg_split[1] + msg_split[2];
            double value = msg_split[3].toDouble();
            qint64 timestamp = static_cast<qint64>(msg_split[4].toDouble() * 1000);


            if (this->connect_to_plotjuggler) {
                this->sendToPlotjuggler(msg_name,value	);
            }

            this->save_to_array(msg_name, timestamp, value);
        }
    }
    else {
        this->ui->console->append(msg);
    }

    return;

    if (msg.contains("[INFO]")) {
    format_.setForeground(Qt::gray);

    this->ui->console->setCurrentCharFormat(format_);
    this->ui->console->append(msg);

    if (this->connect_to_plotjuggler) {
        this->sendToPlotjuggler(
        QTextCodec::codecForName("UTF-8")->toUnicode(msg.toUtf8()));
    }

    this->save_to_array(msg);

    } else if (msg.contains("[WARNING]")) {
        format_.setForeground(Qt::yellow);
        this->ui->console->setCurrentCharFormat(format_);
        this->ui->console->append(msg);
    } else if (msg.contains("[ERROR]")) {

        format_.setForeground(Qt::red);
        this->ui->console->setCurrentCharFormat(format_);
        this->ui->console->append(msg);
    } else if (msg.contains("[RESPONSE]")) {
        this->ui->command_out->appendPlainText(msg);
    }
    else if (msg.contains("[VALUE]")) {
        format_.setForeground(Qt::green);
        if (this->connect_to_plotjuggler) {
            this->sendToPlotjuggler(
            QTextCodec::codecForName("UTF-8")->toUnicode(msg.toUtf8()));
        }
    }
}

void SCDTR_Interface::sendToPlotjuggler(QString msg, double value) {
    json j;
    auto clock = QDateTime::currentDateTime();

    j[msg.toUtf8().constData()] = value;
    j["timestamp"] = clock.toMSecsSinceEpoch();

    std::string dic = j.dump();

    this->_udpSocket.write((const char *)dic.c_str(), strlen(dic.c_str()));
}

void SCDTR_Interface::sendToPlotjuggler(QString msg) {
  QStringList split_string = msg.split(":");
  auto clock = QDateTime::currentDateTime();
  json j;

  if (split_string.size() != 3) {
    return;
  }

  QString signal_name = split_string[1];
  QString signal_value = split_string[2];

  j[signal_name.toUtf8().constData()] = signal_value.toUtf8().toDouble();
  j["timestamp"] = clock.toMSecsSinceEpoch();

  std::string dic = j.dump();

  this->_udpSocket.write((const char *)dic.c_str(), strlen(dic.c_str()));
}

// private slots
void SCDTR_Interface::on_messages_check_box_clicked(bool checked) {
  this->show_messages = checked;
}

void SCDTR_Interface::on_plotjuggler_check_box_clicked(bool checked) {
  this->connect_to_plotjuggler = checked;
  if (checked == true) {
    // send presence message to usp socket
    this->presence_timer->start(std::chrono::milliseconds(1000));

    // connect to plotjuggler
    QString ip_ = "127.0.0.1";
    QString port_ = "9870";

    this->_udpSocket.connectToHost(ip_, port_.toUShort());
  } else {
    this->presence_timer->stop();
    this->_udpSocket.disconnectFromHost();
  }
}

void SCDTR_Interface::on_send_cmd_released() {
  QString com = this->ui->command_in->currentText();
  QString lum = this->ui->luminaire_line_edit->text();
  QString arg = this->ui->extra_arg_line_edit->text();

  if (lum == "<empty>") {
    std::cout << "what command will the luminaire go to?" << std::endl;
    return;
  }
  if ((com.contains("<val>") || com.contains("<x>")) && arg == "<empty>") {
    std::cout << "Missing argument, not sending command " << std::endl;
    return;
  }

  if (com.contains("<i>")) {
    com.replace("<i>", lum);
  }

  if (com.contains("<val>")) {
    com.replace("<val>", arg);
  }

  if (com.contains("<x>") && arg != "empty") {
    com.replace("<x>", arg);
  }

  com += '\n';

  _interfaceSocket->writeDatagram(com.toUtf8(), server_addr, server_port);

}

void SCDTR_Interface::on_command_in_currentTextChanged(const QString &arg1) {
  this->ui->comment_label->setText(this->desired_commands[arg1]);
}

void SCDTR_Interface::read() {
  auto data = serial_port->readAll();

  handleRead(data);
}
/*
void SCDTR_Interface::on_serial_port_connect_clicked(bool checked)
{}*/

void SCDTR_Interface::on_udp_server_connect_clicked(bool checked) {

    if (checked) {
        QString udp_ip = this->ui->udp_server_ip_combo->currentText();
        QString udp_port = this->ui->udp_server_port_combo->currentText();

        server_addr = QHostAddress(udp_ip);
        server_port = udp_port.toUShort();

        _interfaceSocket->connectToHost(QHostAddress::LocalHost, 0);
        connect(_interfaceSocket, &QUdpSocket::readyRead,
                this, &SCDTR_Interface::read_from_udp_socket);

        udp_server_timer->start(std::chrono::seconds(1));
    }  else  {
        this->_interfaceSocket->disconnectFromHost();
        this->udp_server_timer->stop();
    }
}

void SCDTR_Interface::save_to_array(QString msg_name, qint64 timestamp, double value){
    auto pair = qMakePair(timestamp, value);

    if (signal_map.contains(msg_name)) {
        signal_map[msg_name].push_back(pair);
    } else {
        signal_map[msg_name] = QVector<QPair<qint64, double>>();
        signal_map[msg_name].push_back(pair);
    }

    return;
}

void SCDTR_Interface::save_to_array(QString msg) {
    auto clock = QDateTime::currentDateTime();
    QStringList split_string =  msg.split(":");
    if (split_string.size() != 3) {
        return;
    }

    QString signal_name = split_string[1];
    QString signal_value = split_string[2];

    auto pair = qMakePair( clock.toMSecsSinceEpoch() - start_time, signal_value.toDouble());

    if (signal_map.contains(signal_name)) {
        signal_map[signal_name].push_back(pair);
    } else {
        signal_map[signal_name] = QVector<QPair<qint64, double>>();
        signal_map[signal_name].push_back(pair);
    }
}

void SCDTR_Interface::read_from_udp_socket(){

    while(_interfaceSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(_interfaceSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderport;

        _interfaceSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderport);

        last_msg +=  datagram.toStdString();

        // split by the \n
        std::size_t pos = 0;
        while ((pos = last_msg.find("\n")) != std::string::npos) {
            std::string token_ = last_msg.substr(0, pos);
            last_msg.erase(0, pos + 1);
            // Receive a full message -> handle it
            handleRead(QString::fromStdString(token_));
        }
    }
}

void SCDTR_Interface::send_udp_server_presence() {
    std::cout << "Presence timer" << std::endl;
    if (_interfaceSocket == NULL) {
        return;
    }

    _interfaceSocket->writeDatagram("Presence", server_addr, server_port);
    return;
}
