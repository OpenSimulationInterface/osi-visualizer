#pragma once
#include "globject.h"


class GLPointCloud : public GLObject
{
public:
	GLPointCloud(QOpenGLFunctions_4_3_Core* functions, const QString& srcPath);
};
