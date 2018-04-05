#include "glwidget.h"
#include "glvehicle.h"
#include "gltriangle.h"
#include "glpoint.h"
#include "global.h"
#include "glfieldofview.h"
#include "customtreewidgetitem.h"

//#include <QMutex>
#include <QDebug>
#include <QOpenGLFunctions_4_3_Core>

#include <QWheelEvent>

//QMutex mutex;
//QMutex mutex2;

GLWidget::GLWidget(QWidget* parent,
                   IMessageSource* msgSource,
                   QMap<ObjectType, QTreeWidgetItem*>& treeNodes,
                   const AppConfig& config)
    : QOpenGLWidget(parent)
    , minCameraY_(0)
    , ignoreUpdate_(false)
    , isOpenGLInitizalized_(false)
    , uniformMvpLocation_(0)
    , uniformColorLocation_(0)
    , uniformUseTextureLocation_(0)
    , currentDataType_(DataType::Groundtruth)
    , camera_(nullptr)
    , mousePos_()
    , config_(config)
    , lanes_()
    , pressedKeys_()
    , sceneKeys_()
    , msgSource_(msgSource)
    , isFirstMsgReceived_(false)
    , selectedObject_(nullptr)
    , projectionMatrix_()
    , staticObjects_()
    , simulationObjects_()
    , shaderProgram_()
    , treeNodes_(treeNodes)
    , showFOV_(false)
    , objFOV_(nullptr)
{
    installEventFilter(this);
}

void
GLWidget::UpdateIMessageSource(IMessageSource* msgSource)
{
    msgSource_ = msgSource;
}

void
GLWidget::UpdateFOVPaint(const bool showFOV)
{
    showFOV_ = showFOV;
    this->update();
}

void
GLWidget::UpdateFOVParam(const float minRadius,
                         const float maxRadius,
                         const float azimuthPosAngle,
                         const float azimuthNegAngle)
{
    objFOV_->UpdateParameter(minRadius, maxRadius, azimuthPosAngle, azimuthNegAngle);
    objFOV_->UpdateVertexBuffer();
    UpdateFOVPaint(true);
}

void
GLWidget::initializeGL()
{
    if(!initializeOpenGLFunctions())
    {
        qDebug() << "failed to initialize OpenGL functions";
        return;
    }
    // Disabling the depth test.
    // We are simulating a plain 2D world, thus all elements have the same y value.
    // Elements that have the same y value would cause flickering on each other.
    // A solution would be to slightly differ the y values, but looking at different objects
    // from certain angles will look like they are slightly shifted in x/z direction.
    // By disabling the depth test (should be disabled by default anyway) we can overcome this issue.
    // It is important now to draw the objects in the correct order, the top one has to be rendered at the end.


    glDisable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // Not necessary for the current scenarios
    //glEnable(GL_CULL_FACE);

    //glEnable(GL_BLEND);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0, 0, 0, 1);



    // TODO: Check if min required versions of OpenGL and GLSL is available (4.3)
    QString version = QString::fromLatin1((const char*)glGetString(GL_VERSION));
    qDebug() << "OpenGL version: " + version;

    shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, config_.srcPath_ + "Resources/Shaders/VertexShader.vert");
    shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, config_.srcPath_ + "Resources/Shaders/FragmentShader.frag");
    shaderProgram_.link();
    shaderProgram_.bind();

    uniformMvpLocation_ = shaderProgram_.uniformLocation("mvpMatrix");
    uniformColorLocation_ = shaderProgram_.uniformLocation("color");
    uniformUseTextureLocation_ = shaderProgram_.uniformLocation("useTexture");

    camera_ = new Camera(BirdsEye);
    camera_->resetPosition_.setY(140);
    camera_->ResetAll();

    GLGrid* grid = new GLGrid(this, config_.srcPath_);
    grid->Init();
    // If checkbox is not checked initially, no event will be triggered
    grid->isVisible_ = config_.showGrid_;
    staticObjects_.append(grid);

    GLTriangle* center = new GLTriangle(this);
    center->SetColor(Qt::red);
    center->Init();
    staticObjects_.append(center);

    objFOV_ = new GLFieldOfView(this, 0, 1, 1, -1);
    objFOV_->SetColor(Qt::yellow);
    objFOV_->SetOrientation(M_PI_2);
    objFOV_->Init();

    isOpenGLInitizalized_ = true;
}

void
GLWidget::resizeGL(int width, int height)
{
    if(isOpenGLInitizalized_)
    {
        projectionMatrix_.setToIdentity();
        projectionMatrix_.perspective(45.0f, (float)width / height, 0.001f, 1000.0f);
        glViewport(0, 0, width, height);
    }
}

void
GLWidget::paintGL()
{
    // This is already done at this point
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Depth test is disabled, thus the static objects (i.e. grid) will be drawn first.
    // Afterwards the lanes, then the vehicles etc. are drawn, the texts are the last ones.
    foreach (GLObject* staticObject, staticObjects_)
    {
        RenderObject(staticObject);
    }

    //mutex2.lock();
    if (!ignoreUpdate_)
    {
        foreach (Lane* lane, lanes_)
        {
            foreach (GLLaneMarking* laneMarking, lane->glLaneMarkings_)
            {
                RenderObject(laneMarking);
            }
        }
    }
    //mutex2.unlock();

    // TODO: Mutex for object list?
    //mutex.lock();
    foreach (GLObject* object, simulationObjects_)
    {
        RenderObject(object);
    }
    foreach (GLObject* object, simulationObjects_)
    {
        if (object->GetTextObject()) {
            RenderObject(object->GetTextObject());
        }
    }

    if(showFOV_)
        RenderObject(objFOV_);
    //mutex.unlock();
}

void
GLWidget::RenderObject(GLObject* object)
{
    if (!object->isVisible_ || object->forceInvisible_)
    {
        return;
    }

    bool useTexture = (object->textureId_ != -1);
    shaderProgram_.setUniformValue(uniformUseTextureLocation_,
                                   useTexture);

    shaderProgram_.setUniformValue(uniformMvpLocation_,
                                   projectionMatrix_ * camera_->viewMatrix_ * object->modelMatrix_);

    if (useTexture)
    {
        glBindTexture(GL_TEXTURE_2D, object->textureId_);
    }
    else
    {
        QColor color = object->GetColor();
        if (object->GetObjectType() != ObjectType::None)
        {
            color = config_.typeColors_.value(object->GetObjectType());
        }
        shaderProgram_.setUniformValue(uniformColorLocation_, color);
    }

    glBindVertexArray(object->vaoId_);
    glDrawArrays(object->GetPrimitiveType(), 0, object->vertices_.size());
}

void
GLWidget::KeyPressed(QSet<int> pressedKeys)
{
    /*
    if (pressedKeys.contains(Qt::Key::Key_F2))
    {
        ResetCameraAll();
    }
    else if (pressedKeys.contains(Qt::Key::Key_F2))
    {
        ResetCameraOrient();
    }
    */
}

void
GLWidget::MouseWheel(QWheelEvent* event)
{
    float delta = -event->angleDelta().y() / 10.0f;
    camera_->Translate(0, delta, 0);
    // TODO: Should the camera trigger an event whenever its position has changed? (signal/slot)
    //grid->SetPosition(0, -camera->GetPosition().y() / 30, 0);
    this->update();

    /*
    QVector3D a = (projectionMatrix * camera->viewMatrix) * QVector3D(0, 0, 0);
    QVector3D b = (projectionMatrix * camera->viewMatrix) * QVector3D(1, 0, 0);
    float length = (a - b).length();
    qDebug() << length;
    */
}

// TODO: Move this somewhere else
int signum(int val) {
    if ((0 < val) - (val < 0) == 1)
    {
        return 1;
    }
    return -1;
}

bool
GLWidget::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::KeyPress)
    {
        int key = ((QKeyEvent*)event)->key();
        if (sceneKeys_.contains(key))
        {
            pressedKeys_ += key;
            KeyPressed(pressedKeys_);
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else if(event->type() == QEvent::KeyRelease)
    {
        int key = ((QKeyEvent*)event)->key();
        if (sceneKeys_.contains(key))
        {
            pressedKeys_ -= key;
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else if (event->type() == QEvent::Wheel)
    {
        MouseWheel((QWheelEvent*)event);
        event->accept();
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;
        mousePos_ = mouseEvent->pos();
        if (mouseEvent->buttons().testFlag(Qt::MiddleButton))
        {
            ResetCameraAll();
        }
    }
    // This event is only triggered if a mouse button is pressed at the same time
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;
        QPoint newMousePos = mouseEvent->pos();
        int mouseDeltaX = newMousePos.x() - mousePos_.x();
        int mouseDeltaY = newMousePos.y() - mousePos_.y();
        mousePos_ = newMousePos;

        if (mouseEvent->buttons().testFlag(Qt::LeftButton))
        {
            if (camera_->trackedObject_ && !msgSource_->isPaused_ && msgSource_->isConnected_)
            {
                // TODO: Print a message?
                return false;
            }

            float sensivity = 0.002f;
            float cameraY = camera_->GetPosition().y();
            QVector3D cameraDeltaX = sensivity * mouseDeltaY * cameraY * camera_->GetUp();
            QVector3D cameraDeltaZ = sensivity * -mouseDeltaX * cameraY * camera_->GetRight();

            camera_->Translate(cameraDeltaX + cameraDeltaZ);
            this->update();
        }
        else if (mouseEvent->buttons().testFlag(Qt::RightButton))
        {
            if (msgSource_->isPaused_ || (!config_.lockCamera_ || (config_.lockCamera_ && !camera_->trackedObject_)))
            {
                int sign = -signum(mouseDeltaX);
                if (fabs(mouseDeltaY) > fabs(mouseDeltaX))
                {
                    sign = signum(mouseDeltaY);
                }

                float sensivity = 0.0005f;
                float rotation = sensivity * sign * (mouseDeltaX * mouseDeltaX + mouseDeltaY * mouseDeltaY);
                camera_->RotateAroundYAxis(rotation);
                foreach (GLObject* object, simulationObjects_)
                {
                    if (object->GetTextObject())
                    {
                        object->GetTextObject()->SetOrientation(camera_->GetOrientation());
                    }
                }
                this->update();
            }
        }
    }

    // TODO: Is this equivalent to accept/ignore? Use it here and in MainWindow
    return false;
}

void
GLWidget::ResetCameraAll()
{
    camera_->ResetAll();
    ResetObjectTextOrientations();
    this->update();
}

void
GLWidget::ResetCameraOrient()
{
    camera_->ResetOrientation();
    ResetObjectTextOrientations();
    this->update();
}

void
GLWidget::ResetObjectTextOrientations()
{
    //mutex.lock();
    foreach(GLObject* object, simulationObjects_)
    {
        if (object->GetTextObject())
        {
            object->GetTextObject()->SetOrientation(0);
        }
    }
    //mutex.unlock();
}


void
GLWidget::MessageParsed(const Message& message,
                        const LaneMessage& laneMessage)
{
    // The check for a connected receiver is necessary because the signals for received messages
    // the receiver emits are asynchronous. It sometimes happens that after the receiver disconnected,
    // a "MessageReceived" message is still waiting for being processed. That means messages will be
    // received despite the reciever already emitted its "Disconnected" signal, leaving the application
    // in a more or less "undefined" state.

    if (!this->isOpenGLInitizalized_ || !msgSource_->isConnected_)
    {
        return;
    }

    // The QOpenGLWidget seems to unbind the context between the "paintGL" calls
    this->makeCurrent();

    QVector<GLObject*> objectControlList(simulationObjects_);
    QVector<Lane*> laneControlList(lanes_);

    for (const auto &msg: message)
    {
        bool found = false;
        GLObject* currentObject = nullptr;

        if (!msg.id.isEmpty())
        {
            foreach (GLObject* object, simulationObjects_) {
                if (object->id_ == msg.id)
                {
                    found = true;
                    currentObject = object;
                    if (camera_->trackedObject_ == object)
                    {
                        camera_->SetPosition(msg.position.x(), camera_->GetPosition().y(), msg.position.z());
                        if (config_.lockCamera_)
                        {
                            camera_->SetOrientation(msg.orientation + M_PI);
                        }
                    }

                    object->isVisible_ = true;
                    object->SetPosition(msg.position, false);

                    if(msg.basePoly.size() == 4)
                    {
                        object->vertices_.clear();
                        object->vertices_ << msg.basePoly[0] << msg.basePoly[1] << msg.basePoly[2] << msg.basePoly[3];
                        object->SetOrientation(msg.orientation);
                    }
                    else
                    {
                        osi::Dimension3d dimension = msg.dimension;
                        object->vertices_.clear();
                        object->vertices_ << QVector3D(-dimension.width()/2, 0, dimension.length()/2)
                                          << QVector3D(dimension.width()/2, 0, dimension.length()/2)
                                          << QVector3D(dimension.width()/2, 0, -dimension.length()/2)
                                          << QVector3D(-dimension.width()/2, 0, -dimension.length()/2);
                        object->SetOrientation(msg.orientation + M_PI_2);
                    }
                    object->UpdateVertexBuffer();

                    if (object->GetTextObject())
                    {
                        object->GetTextObject()->isVisible_ = true;
                    }

                    GLObject* textObject = object->GetTextObject();
                    if (textObject)
                    {
                        float length = msg.dimension.length() / 2 + 1.0f;
                        float width = msg.dimension.width() / 2 + 0.1f;
                        float offsetX = textObject->halfSize_.width();
                        float offsetY = textObject->halfSize_.height();
                        textObject->SetOrientation(camera_->GetOrientation());
                        textObject->SetPosition(msg.position + QVector3D(width + offsetX, 0.1f, length + offsetY));
                    }

                    objectControlList.removeOne(object);
                    break;
                }
            }
        }

        if (!found)
        {
            ObjectType type = msg.type;
            if (type == ObjectType::None)
            {
                continue;
            }

            GLObject* newObject;

            if ( type == ObjectType::Car ||
                 type == ObjectType::Truck ||
                 type == ObjectType::MotorBike ||
                 type == ObjectType::Bicycle )
            {
                if(msg.basePoly.size() == 4)
                    newObject = new GLVehicle(this, msg.id, msg.basePoly[0], msg.basePoly[1], msg.basePoly[2], msg.basePoly[3]);
                else
                    newObject = new GLVehicle(this, msg.id, msg.dimension.length(), msg.dimension.width());
            }
            else
            {
                newObject = new GLPoint(this, msg.id);
            }

            currentObject = newObject;
            newObject->Init();
            newObject->SetObjectType(type);
            newObject->SetPosition(msg.position, false);
            newObject->SetOrientation(msg.orientation);
            newObject->SetText(msg.name);
            newObject->GetTextObject()->SetOrientation(camera_->GetOrientation());
            // TODO: Actually, the text position has to be set here already
            // But the next data update will most propably directly follow
            //mutex.lock();
            simulationObjects_.append(newObject);
            //mutex.unlock();
        }

        // No "Set" method here, these values do not affect the simulation
        currentObject->velocity_ = msg.velocitie;
        currentObject->acceleration_ = msg.acceleration;
        currentObject->realPosition_ = msg.realPosition;

        if (!currentObject->alreadyInObjectTree_)
        {
            CustomTreeWidgetItem* item = new CustomTreeWidgetItem(currentObject);
            item->setText(0, msg.name);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            // Even if the box should not be checked from the beginning,
            // "Unchecked" would have to be set, otherwise the checkbox will never be displayed
            item->setCheckState(0, Qt::Checked);
            treeNodes_[currentObject->GetObjectType()]->addChild(item);
            currentObject->alreadyInObjectTree_ = true;
        }

        if (currentDataType_ != DataType::SensorData && isFirstMsgReceived_ && msg.isEgoVehicle)
        {
            isFirstMsgReceived_ = false;
            selectedObject_ = currentObject;
            StartTracking();
        }
    }

    for (const auto &laneMsg: laneMessage)
    {
        bool found = false;
        QVector<QVector<QVector3D> > laneMarkers;

        if(config_.laneType_ == LaneType::CenterLanes)
        {
            laneMarkers = laneMsg.centerLanes;
        }
        else
        {
            laneMarkers = laneMsg.boundaryLanes;
        }

        foreach (Lane* lane, lanes_)
        {
            if (lane->laneId_ == laneMsg.id)
            {
                found = true;
                ignoreUpdate_ = true;
                lane->UpdateLaneMarkings(laneMarkers);
                ignoreUpdate_ = false;

                lane->isVisible_ = true;
                laneControlList.removeOne(lane);
                break;
            }
        }

        if (!found)
        {
            //qDebug() << "New lane: " << laneMessage.ids.at(i);
            Lane* newLane = new Lane(laneMsg.id, laneMarkers, this);
            //mutex2.lock();
            lanes_.append(newLane);
            //mutex2.unlock();
        }
    }

    // TODO: Remove ununsed objects from object tree, also stop tracking etc
    // Hide unused objects
    foreach (GLObject* object, objectControlList)
    {
        if (!object->id_.isEmpty())
        {
            object->isVisible_ = false;
            QTreeWidgetItem* node = treeNodes_[object->GetObjectType()];
            for (int i = 0; i < node->childCount(); ++i)
            {
                if (((CustomTreeWidgetItem*)node->child(i))->glObject_ == object)
                {
                    node->removeChild(node->child(i));
                    object->alreadyInObjectTree_ = false;
                    if (camera_->trackedObject_ == object)
                    {
//                        StopTracking();
                        camera_->trackedObject_ = nullptr;
                    }
                    break;
                }
            }

            if (object->GetTextObject())
            {
               object->GetTextObject()->isVisible_ = false;
            }
        }
    }

    foreach (Lane* lane, laneControlList)
    {
        lane->isVisible_ = false;
    }

    if (selectedObject_)
    {
        emit DisplayObjectInformation(selectedObject_);
    }

    this->update();
}

void
GLWidget::TreeItemChanged(QTreeWidgetItem* item, int column)
{
    CustomTreeWidgetItem* customItem = (CustomTreeWidgetItem*)item;
    GLObject* glObject = customItem->glObject_;

    glObject->forceInvisible_ = item->checkState(column) == Qt::Unchecked;
    if (glObject->GetTextObject())
    {
        glObject->GetTextObject()->forceInvisible_ = glObject->forceInvisible_;
    }

    // The check event is fired before the click event
    customItem->ignoreClick_ = true;
    this->update();
}

// Qt does not prevent the "Click" event when "DoubleClick" occurs.
// Thus we have to determine a double click by ourselves.
void
GLWidget::TreeItemClicked(QTreeWidgetItem* item, int column)
{
    if (treeNodes_.values().contains(item))
    {
        return;
    }

    CustomTreeWidgetItem* customItem = (CustomTreeWidgetItem*)item;
    if (customItem->ignoreClick_)
    {
        customItem->ignoreClick_ = false;
        return;
    }

    QElapsedTimer* timer = customItem->lastClickTimer_;
    GLObject* object = customItem->glObject_;
    selectedObject_ = object;

    bool setCameraToObjectPos = false;

    if (timer)
    {
        long msElapsed = timer->nsecsElapsed() / 1000000;
        // 500 ms is Windows default
        if (msElapsed < 500)
        {
            StartTracking();
        }
        else
        {
            setCameraToObjectPos = true;
        }

        timer->restart();
    }
    else
    {
        timer = new QElapsedTimer();
        timer->start();

        setCameraToObjectPos = true;
        customItem->lastClickTimer_ = timer;
    }

    if (setCameraToObjectPos)
    {
        if (!camera_->trackedObject_)
        {
            //camera->trackedObject_ = nullptr;
            camera_->SetToObjectPosition(object);
        }
    }

    emit DisplayObjectInformation(object);
    if (selectedObject_)
    {
        emit SetTrackingEnabled(selectedObject_ != camera_->trackedObject_);
    }
    else
    {
        emit SetTrackingEnabled(false);
    }

    this->update();
}

void
GLWidget::Connected(DataType dataType)
{
    selectedObject_ = nullptr;
    currentDataType_ = dataType;
    camera_->ResetAll();

    foreach (QTreeWidgetItem* item, treeNodes_)
    {
        while (item->childCount() > 0)
        {
            item->removeChild(item->child(0));
        }
    }

    QVector<GLObject*> objectsToRemove;

    foreach (GLObject* object, simulationObjects_)
    {
        delete object;
        objectsToRemove.append(object);
    }

    //mutex.lock();
    foreach (GLObject* object, objectsToRemove)
    {
        simulationObjects_.removeAll(object);
    }
    //mutex.unlock();

    this->update();
}

void
GLWidget::Disconnected()
{
    //selectedObject = nullptr;
    isFirstMsgReceived_ = true;

    foreach (QTreeWidgetItem* item, treeNodes_)
    {
        while (item->childCount() > 0)
        {
            item->removeChild(item->child(0));
        }
    }

    QVector<GLObject*> objectsToRemove;
    foreach (GLObject* object, simulationObjects_)
    {
        delete object;
        objectsToRemove.append(object);
    }

    //mutex.lock();
    foreach (GLObject* object, objectsToRemove)
    {
        simulationObjects_.removeAll(object);
    }
    //mutex.unlock();

//    StopTracking();
    camera_->trackedObject_ = nullptr;

    this->update();
}

void
GLWidget::StartTracking()
{
    if( selectedObject_ != nullptr &&
        camera_->trackedObject_ == selectedObject_ )
    {
        return;
    }

    QTreeWidgetItem* node = treeNodes_[selectedObject_->GetObjectType()];
    node->setExpanded(true);

    for (int i = 0; i < node->childCount(); ++i)
    {
        if (((CustomTreeWidgetItem*)node->child(i))->glObject_ == selectedObject_)
        {
            node->child(i)->setSelected(true);
            break;
        }
    }

    camera_->trackedObject_ = selectedObject_;
    emit SetTrackingEnabled(false);
}

//void
//GLWidget::StopTracking()
//{
//    camera_->trackedObject_ = nullptr;
//    emit SetTrackingEnabled(true);
//}

void
GLWidget::UpdateGrid()
{
    if (isOpenGLInitizalized_)
    {
        staticObjects_.at(0)->isVisible_ = config_.showGrid_;
        this->update();
    }
}


