



#pragma once
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


