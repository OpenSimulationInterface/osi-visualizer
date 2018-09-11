#pragma once
#include "types.h"
#include "glpointcloud.h"
#include <QVector3D>
#include <QOpenGLFunctions_4_3_Core>
#include <memory>

class PointCloud
{
public:
	PointCloud(int id, QOpenGLFunctions_4_3_Core* functions, const QString& srcPath);

	void UpdatePointCloud(const QVector<PointStruct>& pm);	// Creates new GLPointCloud object from the PointStruct-list

	int pointcloudId_;
	bool isVisible_;
	std::unique_ptr<GLPointCloud> glPointCloud_;
	QImage colorscheme;				// Color scheme of the point cloud. The line from texture coordinates (0,0) to (1,1) defines color(intensity)

private:
	QOpenGLFunctions_4_3_Core* functions_;
	QString srcPath_;
};
