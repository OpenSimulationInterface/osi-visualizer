///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "types.h"
#include <QColor>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions_4_3_Core>

class GLObject
{
    public:
        GLObject(GLenum primitiveType,
                 QOpenGLFunctions_4_3_Core* functions,
                 QString id = "",
                 GLenum drawType = GL_STATIC_DRAW);

        ~GLObject();

        void Init();
        void UpdateVertexBuffer();
        void Translate(QVector3D position, bool updateMatrix = true);
        void Translate(float x, float y, float z, bool updateMatrix = true);
        void RotateAroundYAxis(float rotation, bool updateMatrix = true);

        void SetText(QString text);

        void SetTexture(QImage image, bool generateMipMaps);

        void SetColor(QColor color) { color_ = color; }
        QColor GetColor() const { return color_; }

        void SetPosition(QVector3D position, bool updateMatrix = true);
        void SetPosition(float x, float y, float z, bool updateMatrix = true);
        QVector3D GetPosition() const { return position_; }

        void SetOrientation(float orientation, bool updateMatrix = true);
        float GetOrientation() const { return orientation_; }

        GLenum GetPrimitiveType() const { return primitiveType_; }
        GLObject* GetTextObject() { return textObject_.get(); }

        void SetObjectType(ObjectType type) { objectType_ = type; }
        ObjectType GetObjectType() const { return objectType_; }


        QString id_;
        GLuint vaoId_;
        GLuint textureId_;
        QString text_;
        QSize halfSize_;
        bool isVisible_;
        bool forceInvisible_;
        bool alreadyInObjectTree_;
        QMatrix4x4 modelMatrix_;
        QVector<QVector3D> vertices_;
        QVector<QVector2D> texCoords_;
        QVector3D velocity_;
        QVector3D acceleration_;
        QVector3D realPosition_;

    private:

        void Update(bool updateMatrix);
        void GenerateDescriptionTexture();

        GLuint vboId_;
        GLuint vboTexCoordId_;
        QColor color_;
        GLenum drawType_;
        float orientation_;
        QVector3D position_;
        GLenum primitiveType_;
        std::shared_ptr<GLObject> textObject_;
        bool isInitialized_;
        int lastVertexSize_;
        ObjectType objectType_;
        QOpenGLFunctions_4_3_Core* functions_;

};
