#include "mainwindow.h"
#include "tcpreceiver.h"
#include <QApplication>

#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"

Q_DECLARE_METATYPE(DataType)
Q_DECLARE_METATYPE(Message)
Q_DECLARE_METATYPE(LaneMessage)
Q_DECLARE_METATYPE(osi3::SensorData)
Q_DECLARE_METATYPE(osi3::SensorView)

int main(int argc, char* argv[])
{
    qRegisterMetaType<Message>();
    qRegisterMetaType<LaneMessage>();
    qRegisterMetaType<DataType>();
    qRegisterMetaType<osi3::SensorData>();
    qRegisterMetaType<osi3::SensorView>();

    QApplication app(argc, argv);
    //    Q_INIT_RESOURCE();
    MainWindow window;
    window.show();
    window.LocalUpdate();

    return app.exec();
}
