#include "glpointcloud.h"

GLPointCloud::GLPointCloud(QOpenGLFunctions_4_3_Core* functions, const QString& srcPath)
	: GLObject(GL_POINTS, functions, "", GL_STREAM_DRAW)
{
	
}
