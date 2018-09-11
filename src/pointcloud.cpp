#include "pointcloud.h"

PointCloud::PointCloud(int id, QOpenGLFunctions_4_3_Core* functions, const QString& srcPath)
	: pointcloudId_(id),
	  isVisible_(true),
	  functions_(functions),
	  srcPath_(srcPath)
{
	if (!colorscheme.load(srcPath + "Resources/Images/PointCloudColors.png")) {
		qDebug() << "Failed to open point cloud color scheme (" << srcPath << "Resources/Images/PointCloudColors.png)";
	}
	colorscheme = colorscheme.convertToFormat(QImage::Format::Format_RGBA8888); // convert to the texture format we use
}

void
PointCloud::UpdatePointCloud(const QVector<PointStruct>& pm)
{
	glPointCloud_ = std::make_unique<GLPointCloud>(functions_, srcPath_);
	glPointCloud_->id_ = "point_cloud_" + QString::number(pointcloudId_);
	// Copy data from pm to glPointCloud_
	for (const PointStruct& p : pm) {
		glPointCloud_->vertices_.append(p.position);
		glPointCloud_->texCoords_.append(QVector2D(p.color, p.color));
	}
	// set up the globject
	glPointCloud_->Init();
	glPointCloud_->SetColor(Qt::white);
	glPointCloud_->SetTexture(colorscheme, false, GL_CLAMP_TO_EDGE);
	glPointCloud_->SetObjectType(ObjectType::None);
	glPointCloud_->UpdateVertexBuffer();
}
