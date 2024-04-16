#include "scdtr_interface.h"

#include <QApplication>

#include <boost/asio.hpp>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SCDTR_Interface main_window;
    main_window.show();

    return a.exec();
}
