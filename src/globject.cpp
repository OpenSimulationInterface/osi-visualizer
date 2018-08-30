#define _USE_MATH_DEFINES

#include "globject.h"
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QPainter>
#include <QBitmap>
#include <math.h>

GLObject::GLObject(GLenum primitiveType,
                   QOpenGLFunctions_4_3_Core* functions,
                   QString id,
                   GLenum drawType)
    : id_(id)
    , vaoId_()
    , textureId_(-1)
    , text_()
    , halfSize_()
    , isVisible_(true)
    , forceInvisible_(false)
    , alreadyInObjectTree_(false)
    , modelMatrix_()
    , vertices_()
    , texCoords_()
    , velocity_()
    , acceleration_()
    , realPosition_()

    , vboId_()
    , vboTexCoordId_()
    , color_()
    , drawType_(drawType)
    , orientation_(0)
    , position_()
    , primitiveType_(primitiveType)
    , textObject_(nullptr)
    , isInitialized_(false)
    , lastVertexSize_(-1)
    , objectType_(ObjectType::None)
    , functions_(functions)
{

}

GLObject::~GLObject()
{
    if (isInitialized_)
    {
        if (!texCoords_.isEmpty())
        {
            functions_->glDeleteBuffers(1, &vboTexCoordId_);
        }
        functions_->glDeleteBuffers(1, &vboId_);
        functions_->glDeleteVertexArrays(1, &vaoId_);
    }
}

// Qt provides classes like QVertexArrayObject, for example.
// However, they caused unexpected behaviour, thus plain OpenGL commands are used.
void
GLObject::Init()
{
    if (isInitialized_)
    {
        return;
    }

    functions_->glGenVertexArrays(1, &vaoId_);
    functions_->glBindVertexArray(vaoId_);

    functions_->glGenBuffers(1, &vboId_);
    UpdateVertexBuffer();
    functions_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    functions_->glEnableVertexAttribArray(0);

    // Tex coord buffer
    if (!texCoords_.isEmpty())
    {
        int elementSize = 2;
        int sizeInBytes = elementSize * texCoords_.size() * sizeof(float);
        functions_->glGenBuffers(1, &vboTexCoordId_);
        functions_->glBindBuffer(GL_ARRAY_BUFFER, vboTexCoordId_);
        functions_->glBufferData(GL_ARRAY_BUFFER, sizeInBytes, texCoords_.constData(), GL_STATIC_DRAW);
        functions_->glVertexAttribPointer(1, elementSize, GL_FLOAT, GL_FALSE, 0, 0);
        functions_->glEnableVertexAttribArray(1);
    }

    isInitialized_ = true;
}

void
GLObject::UpdateVertexBuffer()
{
    int elementSize = 3;
    int sizeInBytes = elementSize * vertices_.size() * sizeof(float);
    functions_->glBindBuffer(GL_ARRAY_BUFFER, vboId_);

    if (lastVertexSize_ == vertices_.size())
    {
        functions_->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, vertices_.constData());
    }
    else
    {
        // glBufferData deletes the previous buffer data
        lastVertexSize_ = vertices_.size();
        functions_->glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices_.constData(), drawType_);
    }
}

void
GLObject::SetText(QString text)
{
    text_ = text;
    QFont font("Arial", 100);
    QFontMetrics fm(font);
    QSize pixelSize = fm.size(Qt::TextSingleLine, text);
    QImage image(pixelSize, QImage::Format_RGBA8888);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setPen(Qt::white);
    painter.setFont(font);
    painter.drawText(0, pixelSize.height() - 5, text);

    // TODO: make size public such the text can be shifted appropriately
    float halfLength = 1.0f;
    float halfWidth = halfLength * pixelSize.width() / pixelSize.height();

    textObject_ = std::make_shared<GLObject>(GL_QUADS, functions_);
    textObject_->vertices_ << QVector3D(-halfLength, 0, halfWidth)
                           << QVector3D(halfLength, 0, halfWidth)
                           << QVector3D(halfLength, 0, -halfWidth)
                           << QVector3D(-halfLength, 0, -halfWidth);

    textObject_->texCoords_ << QVector2D(1, 1)
                            << QVector2D(1, 0)
                            << QVector2D(0, 0)
                            << QVector2D(0, 1);

    textObject_->Init();

    textObject_->Translate(0, 0.01f, 0);
    textObject_->halfSize_.setWidth(halfLength);
    textObject_->halfSize_.setHeight(halfWidth);

    textObject_->SetTexture(image, false);
}

void
GLObject::SetTexture(QImage image, bool generateMipMaps, GLint wrapping)
{
    if (textureId_ > -1)
    {
        functions_->glDeleteTextures(1, &textureId_);
    }

    // glActiveTexture not needed atm
    functions_->glGenTextures(1, &textureId_);
    functions_->glBindTexture(GL_TEXTURE_2D, textureId_);
    functions_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             image.width(), image.height(), 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);

    if (generateMipMaps)
    {
        functions_->glGenerateMipmap(GL_TEXTURE_2D);
        functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // TODO: A website stated that Mag filter could only use Nearest and Linear
        functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // TODO: Will these result in deletion of the image?
    image.detach();
}

void
GLObject::SetPosition(float x, float y, float z, bool updateMatrix)
{
    SetPosition(QVector3D(x, y, z), updateMatrix);
}

void
GLObject::SetPosition(QVector3D position, bool updateMatrix)
{
    position_ = position;
    Update(updateMatrix);
}

void
GLObject::SetOrientation(float orientation, bool updateMatrix)
{
    orientation_ = orientation;
    Update(updateMatrix);
}

void
GLObject::Translate(float x, float y, float z, bool updateMatrix)
{
    Translate(QVector3D(x, y, z), updateMatrix);
}

void
GLObject::Translate(QVector3D translation, bool updateMatrix)
{
    position_ += translation;
    Update(updateMatrix);
}

void
GLObject::RotateAroundYAxis(float rotation, bool updateMatrix)
{
    orientation_ += rotation;
    orientation_ = fmod(orientation_, 2 * M_PI);
    Update(updateMatrix);
}

void GLObject::Update(bool updateMatrix)
{
    if (!updateMatrix)
    {
        return;
    }

    //double cosr = 1;
    //double sinr = 0;
    double cosp = cos(orientation_);
    double sinp = sin(orientation_);
    //double cosy = 1;
    //double siny = 0;

    // TODO: Why has QMatrix3x3 not the same constructor?
    // (QMatrix3x3 derives from QGenericMatrix, 4x4 does not)
    /*
    QMatrix4x4 rotX(1, 0, 0, 0,
                    0, cosr, -sinr, 0,
                    0, sinr, cosr, 0,
                    0, 0, 0, 1);
    */

    QMatrix4x4 rotY(cosp, 0, sinp, 0,
                    0, 1, 0, 0,
                    -sinp, 0, cosp, 0,
                    0, 0, 0, 1);

    /*
    QMatrix4x4 rotZ(cosy, -siny, 0, 0,
                    siny, cosy, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
    */

    // TODO: Correct order? Is this pre or post multiply?
    QMatrix4x4 rotMatrix = rotY;// rotX * rotY * rotZ;

    // TODO: Will this cause a memory leak?
    modelMatrix_ = QMatrix4x4(rotMatrix(0, 0), rotMatrix(0, 1), rotMatrix(0, 2), position_.x(),
                              rotMatrix(1, 0), rotMatrix(1, 1), rotMatrix(1, 2), position_.y(),
                              rotMatrix(2, 0), rotMatrix(2, 1), rotMatrix(2, 2), position_.z(),
                              0, 0, 0, 1);

    //modelMatrix = tempModelMatrix;
}

