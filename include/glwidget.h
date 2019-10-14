///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_WIDGET_H
#define OSI_VISUALIZER_GL_WIDGET_H
#include "appconfig.h"
#include "camera.h"
#include "glgrid.h"
#include "globject.h"
#include "imessagesource.h"
#include "lane.h"
#include "types.h"

#include "glfieldofview.h"

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTreeWidgetItem>

class QWheelEvent;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

  public:
    GLWidget(QWidget* parent,
             IMessageSource* msgSource,
             QMap<ObjectType, QTreeWidgetItem*>& treeNodes,
             const AppConfig& config);

    void UpdateIMessageSource(IMessageSource* msgSource);

    void UpdateFOVPaint(const bool showFOV);

    void UpdateFOVParam(const float minRadius,
                        const float maxRadius,
                        const float azimuthPosAngle,
                        const float azimuthNegAngle);

  signals:
    void DisplayObjectInformation(GLObject* object);
    void SetTrackingEnabled(bool enable);

  public slots:

    void UpdateGrid();
    void Disconnected();
    void StartTracking();
    void ResetCameraAll();
    void ResetCameraOrient();
    void Connected(DataType dataType);
    void TreeItemChanged(QTreeWidgetItem* item, int column);
    void TreeItemClicked(QTreeWidgetItem* item, int column);
    void MessageParsed(const Message& message, const LaneMessage& laneMessage);

  protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);
    bool eventFilter(QObject* obj, QEvent* event);

  private:
    void ResetObjectTextOrientations();
    void RenderObject(GLObject* object);
    void MouseWheel(QWheelEvent* event);
    void KeyPressed(QSet<int> pressedKeys);

    float minCameraY_;
    bool ignoreUpdate_;
    bool isOpenGLInitizalized_;
    int uniformMvpLocation_;
    int uniformColorLocation_;
    int uniformUseTextureLocation_;
    DataType currentDataType_;

    Camera* camera_;
    QPoint mousePos_;
    const AppConfig& config_;
    QVector<Lane*> lanes_;
    QSet<int> pressedKeys_;
    QList<int> sceneKeys_;
    IMessageSource* msgSource_;
    bool isFirstMsgReceived_;
    GLObject* selectedObject_;
    QMatrix4x4 projectionMatrix_;
    QVector<GLObject*> staticObjects_;
    QVector<GLObject*> simulationObjects_;
    QOpenGLShaderProgram shaderProgram_;
    QMap<ObjectType, QTreeWidgetItem*>& treeNodes_;

    bool showFOV_;
    GLFieldOfView* objFOV_;
};
#endif  // OSI_VISUALIZER_GL_WIDGET_H