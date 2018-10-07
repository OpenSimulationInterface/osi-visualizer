#include "appconfig.h"
#include "global.h"
#include <QFile>
#include <QDebug>
#include <QColor>
#include <QDomDocument>
#include <QXmlStreamWriter>

AppConfig::AppConfig(QString fileName)
    : ch1IPAddress_("")
    , ch1PortNum_("")
    , ch1DataType_(DataType::SensorView)
    , ch1FMURxCheck_(false)
    , ch1LoadFMURx_("")
    , ch1LoadFile_("")
    , ch1PlaybackDataType_(DataType::SensorView)
    , ch1DeltaDelay_(0)
    , ch1EnableSendOut_(false)
    , ch1SendOutPortNum_("5564")
    , ch1FMUTxCheck_(false)
    , ch1LoadFMUTx_("")
    , ch1ShowFOV_(false)
    , ch1MinRadius_(0.1f)
    , ch1MaxRadius_(100)
    , ch1AzimuthPos_(180)
    , ch1AzimuthNeg_(-180)

    , ch2IPAddress_("")
    , ch2PortNum_("")
    , ch2DataType_(DataType::SensorView)
    , ch2FMURxCheck_(false)
    , ch2LoadFMURx_("")
    , ch2LoadFile_("")
    , ch2PlaybackDataType_(DataType::SensorView)
    , ch2DeltaDelay_(0)
    , ch2EnableSendOut_(false)
    , ch2SendOutPortNum_("5564")
    , ch2FMUTxCheck_(false)
    , ch2LoadFMUTx_("")
    , ch2ShowFOV_(false)
    , ch2MinRadius_(0.1f)
    , ch2MaxRadius_(100)
    , ch2AzimuthPos_(180)
    , ch2AzimuthNeg_(-180)

    , combineChannel_(false)
    , showGrid_(true)
    , showObjectDetails_(false)
    , lockCamera_(false)
    , laneType_(LaneType::BoundaryLanes)
    , typeColors_()

    , osiMsgSaveThreshold_(1000000)
    , srcPath_("./")

    , configFileName_(fileName)
{

}

bool
AppConfig::Load()
{
    QFile file(configFileName_);
    if (!file.open(QIODevice::ReadOnly))
    {
        // TODO: Don't use qDebug for printing
        qDebug() << "Error while loading config file: '" << configFileName_ << "'";
        return false;
    }

    QDomDocument dom;
    dom.setContent(&file);
    file.close();

    QDomElement root = dom.documentElement().toElement();

    ch1IPAddress_  = root.elementsByTagName("CH1IpAddress").at(0).toElement().text();
    ch1PortNum_    = root.elementsByTagName("CH1PortNumber").at(0).toElement().text();
    ch1DataType_   = (DataType)root.elementsByTagName("CH1DataType").at(0).toElement().text().toInt();
    ch1FMURxCheck_ = root.elementsByTagName("CH1FMURxCheck").at(0).toElement().text() == "1" ? true : false;
    ch1LoadFMURx_  = root.elementsByTagName("CH1LoadFMURx").at(0).toElement().text();
    ch1LoadFile_   = root.elementsByTagName("CH1LoadFile").at(0).toElement().text();
    ch1PlaybackDataType_ = (DataType)root.elementsByTagName("CH1PlaybackDataType").at(0).toElement().text().toInt();
    ch1DeltaDelay_       = root.elementsByTagName("CH1DeltaDelay").at(0).toElement().text().toInt();
    ch1EnableSendOut_    = root.elementsByTagName("CH1EnableSendOut").at(0).toElement().text() == "1" ? true : false;
    ch1SendOutPortNum_   = root.elementsByTagName("CH1SendOutPortNumber").at(0).toElement().text();
    ch1FMUTxCheck_       = root.elementsByTagName("CH1FMUTxCheck").at(0).toElement().text() == "1" ? true : false;
    ch1LoadFMUTx_        = root.elementsByTagName("CH1LoadFMUTx").at(0).toElement().text();
    ch1ShowFOV_          = root.elementsByTagName("CH1ShowFOV").at(0).toElement().text() == "1" ? true : false;
    ch1MinRadius_        = root.elementsByTagName("CH1MinRadius").at(0).toElement().text().toFloat();
    ch1MaxRadius_        = root.elementsByTagName("CH1MaxRadius").at(0).toElement().text().toFloat();
    ch1AzimuthPos_       = root.elementsByTagName("CH1AzimuthPos").at(0).toElement().text().toFloat();
    ch1AzimuthNeg_       = root.elementsByTagName("CH1AzimuthNeg").at(0).toElement().text().toFloat();

    ch2IPAddress_  = root.elementsByTagName("CH2IpAddress").at(0).toElement().text();
    ch2PortNum_    = root.elementsByTagName("CH2PortNumber").at(0).toElement().text();
    ch2DataType_   = (DataType)root.elementsByTagName("CH2DataType").at(0).toElement().text().toInt();
    ch2FMURxCheck_ = root.elementsByTagName("CH2FMUCheck").at(0).toElement().text() == "1" ? true : false;
    ch2LoadFMURx_  = root.elementsByTagName("CH2LoadFMU").at(0).toElement().text();
    ch2LoadFile_   = root.elementsByTagName("CH2LoadFile").at(0).toElement().text();
    ch2PlaybackDataType_ = (DataType)root.elementsByTagName("CH2PlaybackDataType").at(0).toElement().text().toInt();
    ch2DeltaDelay_       = root.elementsByTagName("CH2DeltaDelay").at(0).toElement().text().toInt();
    ch2EnableSendOut_    = root.elementsByTagName("CH2EnableSendOut").at(0).toElement().text() == "1" ? true : false;
    ch2SendOutPortNum_   = root.elementsByTagName("CH2SendOutPortNumber").at(0).toElement().text();
    ch2FMUTxCheck_       = root.elementsByTagName("CH2FMUTxCheck").at(0).toElement().text() == "1" ? true : false;
    ch2LoadFMUTx_        = root.elementsByTagName("CH2LoadFMUTx").at(0).toElement().text();
    ch2ShowFOV_          = root.elementsByTagName("CH2ShowFOV").at(0).toElement().text() == "1" ? true : false;
    ch2MinRadius_        = root.elementsByTagName("CH2MinRadius").at(0).toElement().text().toFloat();
    ch2MaxRadius_        = root.elementsByTagName("CH2MaxRadius").at(0).toElement().text().toFloat();
    ch2AzimuthPos_       = root.elementsByTagName("CH2AzimuthPos").at(0).toElement().text().toFloat();
    ch2AzimuthNeg_       = root.elementsByTagName("CH2AzimuthNeg").at(0).toElement().text().toFloat();

    combineChannel_    = root.elementsByTagName("CombineChannel").at(0).toElement().text() == "1" ? true : false;
    showGrid_          = root.elementsByTagName("ShowGrid").at(0).toElement().text() == "1" ? true : false;
    showObjectDetails_ = root.elementsByTagName("ShowObjectDetails").at(0).toElement().text() == "1" ? true : false;
    lockCamera_        = root.elementsByTagName("LockCamera").at(0).toElement().text() == "1" ? true : false;
    laneType_          = root.elementsByTagName("LaneType").at(0).toElement().text() == "0" ? LaneType::BoundaryLanes : LaneType::CenterLanes;

    osiMsgSaveThreshold_ = root.elementsByTagName("OSIMsgSaveThreshold").at(0).toElement().text().toInt();

    QList<ObjectType> types = Global::GetAllObjectTypes();
    foreach (ObjectType type, types)
    {
        QString typeName = Global::GetObjectTypeName(type);
        QString color = root.elementsByTagName(typeName).at(0).toElement().text();
        typeColors_.insert(type, color);
    }

    return true;
}

bool
AppConfig::Save()
{
    QFile file(configFileName_);
    if (!file.open(QIODevice::WriteOnly))
    {
        // TODO: Don't use qDebug for printing
        qDebug() << "Error while saving config file: '" << configFileName_ << "'";
        return false;
    }

    QString output;
    QXmlStreamWriter writer(&output);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);

    writer.writeStartDocument();
    writer.writeStartElement("AppConfig");

    writer.writeTextElement("CH1IpAddress", ch1IPAddress_);
    writer.writeTextElement("CH1PortNumber", ch1PortNum_);
    writer.writeTextElement("CH1DataType", QString::number(static_cast<int>(ch1DataType_)));
    writer.writeTextElement("CH1FMURxCheck", QString::number(ch1FMURxCheck_));
    writer.writeTextElement("CH1LoadFMURx", ch1LoadFMURx_);
    writer.writeTextElement("CH1LoadFile", ch1LoadFile_);
    writer.writeTextElement("CH1PlaybackDataType", QString::number(static_cast<int>(ch1PlaybackDataType_)));
    writer.writeTextElement("CH1DeltaDelay", QString::number(static_cast<int>(ch1DeltaDelay_)));
    writer.writeTextElement("CH1EnableSendOut", QString::number(ch1EnableSendOut_));
    writer.writeTextElement("CH1SendOutPortNumber", ch1SendOutPortNum_);
    writer.writeTextElement("CH1FMUTxCheck", QString::number(ch1FMUTxCheck_));
    writer.writeTextElement("CH1LoadFMUTx", ch1LoadFMUTx_);
    writer.writeTextElement("CH1ShowFOV", QString::number(ch1ShowFOV_));
    writer.writeTextElement("CH1MinRadius", QString::number(static_cast<float>(ch1MinRadius_)));
    writer.writeTextElement("CH1MaxRadius", QString::number(static_cast<float>(ch1MaxRadius_)));
    writer.writeTextElement("CH1AzimuthPos", QString::number(static_cast<float>(ch1AzimuthPos_)));
    writer.writeTextElement("CH1AzimuthNeg", QString::number(static_cast<float>(ch1AzimuthNeg_)));

    writer.writeTextElement("CH2IpAddress", ch2IPAddress_);
    writer.writeTextElement("CH2PortNumber", ch2PortNum_);
    writer.writeTextElement("CH2DataType", QString::number(static_cast<int>(ch2DataType_)));
    writer.writeTextElement("CH2FMUCheck", QString::number(ch2FMURxCheck_));
    writer.writeTextElement("CH2LoadFMU", ch2LoadFMURx_);
    writer.writeTextElement("CH2LoadFile", ch2LoadFile_);
    writer.writeTextElement("CH2PlaybackDataType", QString::number(static_cast<int>(ch2PlaybackDataType_)));
    writer.writeTextElement("CH2DeltaDelay", QString::number(static_cast<int>(ch2DeltaDelay_)));
    writer.writeTextElement("CH2EnableSendOut", QString::number(ch2EnableSendOut_));
    writer.writeTextElement("CH2SendOutPortNumber", ch2SendOutPortNum_);
    writer.writeTextElement("CH2FMUTxCheck", QString::number(ch2FMUTxCheck_));
    writer.writeTextElement("CH2LoadFMUTx", ch2LoadFMUTx_);
    writer.writeTextElement("CH2ShowFOV", QString::number(ch2ShowFOV_));
    writer.writeTextElement("CH2MinRadius", QString::number(static_cast<float>(ch2MinRadius_)));
    writer.writeTextElement("CH2MaxRadius", QString::number(static_cast<float>(ch2MaxRadius_)));
    writer.writeTextElement("CH2AzimuthPos", QString::number(static_cast<float>(ch2AzimuthPos_)));
    writer.writeTextElement("CH2AzimuthNeg", QString::number(static_cast<float>(ch2AzimuthNeg_)));

    writer.writeTextElement("CombineChannel", QString::number(combineChannel_));
    writer.writeTextElement("ShowGrid", QString::number(showGrid_));
    writer.writeTextElement("ShowObjectDetails", QString::number(showObjectDetails_));
    writer.writeTextElement("LockCamera", QString::number(lockCamera_));
    writer.writeTextElement("LaneType", QString::number(static_cast<int>(laneType_)));

    writer.writeTextElement("OSIMsgSaveThreshold", QString::number(static_cast<int>(osiMsgSaveThreshold_)));

    foreach (ObjectType key, typeColors_.keys())
    {
        writer.writeTextElement(Global::GetObjectTypeName(key), typeColors_.value(key).name());
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    QTextStream stream(&file);
    stream << output;
    file.close();

    return true;
}

