///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_FIELD_OF_VIEW_H
#define OSI_VISUALIZER_GL_FIELD_OF_VIEW_H
#include "globject.h"

class GLFieldOfView : public GLObject
{
  public:
    GLFieldOfView(QOpenGLFunctions_4_3_Core* functions,
                  const float minRadius,
                  const float maxRadius,
                  const float azimuthPosAngle,
                  const float azimuthNegAngle);

    void UpdateParameter(const float minRadius,
                         const float maxRadius,
                         const float azimuthPosAngle,
                         const float azimuthNegAngle);
};
#endif  // OSI_VISUALIZER_GL_FIELD_OF_VIEW_H