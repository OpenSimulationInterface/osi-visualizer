
#include "mainwindow.h"
#include "tcpreceiver.h"
#include <QApplication>

#include"osi_sensordata.pb.h"

Q_DECLARE_METATYPE(DataType)
Q_DECLARE_METATYPE(Message)
Q_DECLARE_METATYPE(LaneMessage)
Q_DECLARE_METATYPE(osi::SensorData)


int main(int argc, char *argv[])
{
    qRegisterMetaType<Message>();
    qRegisterMetaType<LaneMessage>();
    qRegisterMetaType<DataType>();
    qRegisterMetaType<osi::SensorData>();

    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    window.LocalUpdate();

    return app.exec();
}
