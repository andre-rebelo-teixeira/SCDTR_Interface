#ifndef SCDTR_INTERFACE_H
#define SCDTR_INTERFACE_H

#include <nlohmann/json.hpp>

#include <boost/asio.hpp>
#include <chrono>
#include <iostream>

#include <QDateTime>
#include <QFile>
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QSerialPort>
#include <QString>
#include <QStringList>
#include <QTextCharFormat>
#include <QTextCodec>
#include <QTime>
#include <QTimer>
#include <QVector>

#include <QHostAddress>
#include <QUdpSocket>

using boost::asio::buffer;
using boost::asio::io_service;
using boost::asio::serial_port;
using boost::system::error_code;

using boost::asio::io_service;
using boost::asio::serial_port;
using nlohmann::json;

/**
 * @brief The SCDTR_Interface class
 */
QT_BEGIN_NAMESPACE
namespace Ui {
class SCDTR_Interface;
}
QT_END_NAMESPACE

class SCDTR_Interface : public QMainWindow {
  Q_OBJECT

public:
  /**
   * @brief SCDTR_Interface constructor
   *
   * @param parent - QWidget parent
   *
   * @return void
   */
  SCDTR_Interface(QWidget *parent = nullptr);
  /**
   * @brief SCDTR_Interface destructor
   *
   * @return void
   */
  ~SCDTR_Interface();

  /**
   * @brief Private slots -> The private slots are the functions that will be
   * called when interraction with the ui happens, this means each slot will be
   * responsible for a specific action that will be trigerred by the user, such
   * as clicking a button, changing a value, etc.
   *
   * @note some user interactions with the ui are not handlet instantly and
   * there for do not need a slot for them, such as changing the the values in
   * the combo boxes, this will be handled by latter callbacks when the
   * information is usefull, since this saves time in run time
   *
   * @return void
   */
private slots:

  /**
   * @brief on_send_cmd_released -> This function is called when the user
   * clicks the send button, it will send the command that is currently
   * selected in the combo box
   *
   * @return void
   */
  void on_send_cmd_released();

  /**
   * @brief on_messages_check_box_clicked -> This function is called when the
   * user clicks the messages check box, it will show or hide the messages that
   * are being received from the udp socket
   *
   * @param checked - bool value that indicates if the check box is checked or
   * not
   */
  void on_messages_check_box_clicked(bool checked);

  /**
   * @brief on_plotjuggler_check_box_clicked -> This function is called when the
   * user clicks the plotjuggler check box, it will enable or disable the
   * connection to the plotjuggler
   *
   * @param checked - bool value that indicates if the check box is checked or
   * not
   */
  void on_plotjuggler_check_box_clicked(bool checked);

  /**
   * @brief on_udp_server_connect_clicked -> This function is called when the
   * user clicks the udp server connect button, it will enable or disable the
   * connection to the udp server
   *
   * @param checked - bool value that indicates if the check box is checked or
   * not
   */
  void on_udp_server_connect_clicked(bool checked);

  /**
   * @brief on_command_in_currentTextChanged -> This function is called when the
   * user changes the value in the combo box, it will update the command that
   * will be sent
   *
   * @param arg1 - QString value that indicates the current value in the combo
   * box
   */
  void on_command_in_currentTextChanged(const QString &arg1);

private:
  /**
   * @brief handleRead -> This function is called when a message is received
   * from the socket, this handles all the processing we do with this current
   * messaging, such as, saving it to a vector for lated save in a csv file,
   * sending it to the plotjuggler, etc.
   *
   * @param msg_data - QString value that indicates the message that was
   * received
   *
   * @return void
   */
  void handleRead(QString msg_data);

  /**
   * @brief sendToPlotjuggler -> This function is called when we want to send a
   * message to the plotjuggler, this is done by sending a message to the
   * localhost on the port 9870
   *
   * @param msg non split message to be send
   *
   * @return void
   */
  void sendToPlotjuggler(QString msg);

  /**
   * @brief sendToPlotjuggler This function sends data to plot juggler
   * @param msg_name message to send to plot juggler
   * @param value value
   * @return void
   */
  void sendToPlotjuggler(QString msg_name, double value);

  /**
   * @brief send_presence -> This function is called when we want to send a
   * presence message to the udp server, this is done by sending a message to
   * the server address and port
   *
   * @return void
   */
  void send_presence();

  /**
   * @brief initialize_command_in -> This function is called when we want to
   * initialize the combo box with the desired commands
   *
   * @return void
   */
  void initialize_command_in();

  /**
   * @brief read -> This function is called when we want to read the messages
   * from the serial port
   *
   * @return void
   */
  void read();

  /**
   * @brief save_to_array -> This function is called when we want to save the
   * message to the vector for later saving in a csv file
   *
   * @return void
   */
  void save_to_array(QString msg);

/**
 * @brief save to array -> Overloading of the previous save to array functions, to receive data alread parsed insteads of the full message
 *
 * @param msg_name name of the message we want to save
 * @param timestamp timestamp from the pico
 * @param value signal value
 *
 * @return void
 */
  void save_to_array(QString msg_name, qint64 timestamp, double value);

  /**
   * @brief write_to_csv -> This function is called when we want to save the
   * messages in the vector to a csv file
   *
   * @return void
   */
  void write_to_csv();

  /**
   * @brief read_from_udp_socket -> This function is called when we want to read
   * the messages from the udp socket
   *
   * @return void
   */
  void read_from_udp_socket();

  /**
   * @brief send_udp_server_presence -> This function is called when we want to
   * send a presence message to the udp server, this is done by sending a
   * message to the server address and port
   *
   * @return void
   */
  void send_udp_server_presence();
  // variables

  QTimer *read_timer;
  QTimer *presence_timer;
  QTimer *udp_server_timer;

  QSerialPort *serial_port;
  QVector<QString> message_vector;

  std::string last_msg = "";

  QHostAddress server_addr;
  quint16 server_port;

  QUdpSocket _udpSocket;
  QUdpSocket *_interfaceSocket;

  Ui::SCDTR_Interface *ui;

  bool connect_to_plotjuggler = false;
  bool show_messages = true;

  QMap<QString, QString> desired_commands;

  qint64 start_time;

  QMap<QString, QVector<QPair<qint64, double>>> signal_map;
};
#endif // SCDTR_INTERFACE_H
