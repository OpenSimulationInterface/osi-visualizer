
#include "displayobjectdialog.h"
#include "global.h"


#include <QGridLayout>

DisplayObjectDialog::DisplayObjectDialog(QWidget* parent)
    : QDialog(parent)
    , positionX_(new QLabel(tr("-"), this))
    , positionY_(new QLabel(tr("-"), this))
    , positionZ_(new QLabel(tr("-"), this))
    , velocityX_(new QLabel(tr("-"), this))
    , velocityY_(new QLabel(tr("-"), this))
    , velocityZ_(new QLabel(tr("-"), this))
    , accelerationX_(new QLabel(tr("-"), this))
    , accelerationY_(new QLabel(tr("-"), this))
    , accelerationZ_(new QLabel(tr("-"), this))
{
    setWindowTitle(tr("Object Information"));

    QGridLayout* gLayout = new QGridLayout(this);
    QFont font;
    font.setBold(true);

    QLabel* pos = new QLabel(tr("Position"), this);
    pos->setFont(font);
    gLayout->addWidget(pos, 0, 0, 1, 2);

    gLayout->addWidget(new QLabel(tr("  X: "), this), 1, 0);
    gLayout->addWidget(positionX_, 1, 1);
    gLayout->addWidget(new QLabel(tr("m"), this), 1, 2);

    gLayout->addWidget(new QLabel(tr("  Y: "), this), 2, 0);
    gLayout->addWidget(positionY_, 2, 1);
    gLayout->addWidget(new QLabel(tr("m"), this), 2, 2);

    gLayout->addWidget(new QLabel(tr("  Z: "), this), 3, 0);
    gLayout->addWidget(positionZ_, 3, 1);
    gLayout->addWidget(new QLabel(tr("m"), this), 3, 2);

    gLayout->addWidget(new QLabel(tr(""), this), 4, 0);

    QLabel* vel = new QLabel(tr("Velocity"), this);
    vel->setFont(font);
    gLayout->addWidget(vel, 5, 0, 1, 2);

    gLayout->addWidget(new QLabel(tr("  X: "), this), 6, 0);
    gLayout->addWidget(velocityX_, 6, 1);
    gLayout->addWidget(new QLabel(tr("m/s"), this), 6, 2);

    gLayout->addWidget(new QLabel(tr("  Y: "), this), 7, 0);
    gLayout->addWidget(velocityY_, 7, 1);
    gLayout->addWidget(new QLabel(tr("m/s"), this), 7, 2);

    gLayout->addWidget(new QLabel(tr("  Z: "), this), 8, 0);
    gLayout->addWidget(velocityZ_, 8, 1);
    gLayout->addWidget(new QLabel(tr("m/s"), this), 8, 2);

    gLayout->addWidget(new QLabel(tr(""), this), 9, 0);

    QLabel* acc = new QLabel(tr("Acceleration"), this);
    acc->setFont(font);
    gLayout->addWidget(acc, 10, 0, 1, 2);

    gLayout->addWidget(new QLabel(tr("  X: "), this), 11, 0);
    gLayout->addWidget(accelerationX_, 11, 1);
    gLayout->addWidget(new QLabel(tr("m/s<sup>2</sup>"), this), 11, 2);

    gLayout->addWidget(new QLabel(tr("  Y: "), this), 12, 0);
    gLayout->addWidget(accelerationY_, 12, 1);
    gLayout->addWidget(new QLabel(tr("m/s<sup>2</sup>"), this), 12, 2);

    gLayout->addWidget(new QLabel(tr("  Z: "), this), 13, 0);
    gLayout->addWidget(accelerationZ_, 13, 1);
    gLayout->addWidget(new QLabel(tr("m/s<sup>2</sup>"), this), 13, 2);

    positionX_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    positionX_->setMinimumWidth(100);
    setMaximumSize(200, 320);
}

void
DisplayObjectDialog::UpdateObjectInformation(const GLObject& object)
{
    QString type = Global::GetObjectTypeName(object.GetObjectType());
    setWindowTitle(type + " - " + object.text_);

    positionX_->setText(QString::number(object.realPosition_.x()));
    positionY_->setText(QString::number(object.realPosition_.y()));
    positionZ_->setText(QString::number(object.realPosition_.z()));

    velocityX_->setText(QString::number(object.velocity_.x()));
    velocityY_->setText(QString::number(object.velocity_.y()));
    velocityZ_->setText(QString::number(object.velocity_.z()));

    accelerationX_->setText(QString::number(object.acceleration_.x()));
    accelerationY_->setText(QString::number(object.acceleration_.y()));
    accelerationZ_->setText(QString::number(object.acceleration_.z()));
}


