# IMAGE BUILDS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
build-image-osi-visualizer:
	docker build -f Dockerfile.osi_vis -t osi-vis .

# IMAGE BUILDS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


# CONTAINERS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
start-osi-visualizer:
	docker-compose run --rm osi_vis osi-visualizer

# CONTAINERS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
