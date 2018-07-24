# Copyright 2018, Edge Case Research LLC
# Confidential
# Permission to use this file is defined by a License Agreement between
# Edge Case Research and ANSYS dated 25-June-2018.

# IMAGE BUILDS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
build-image-osi-visualizer:
	docker build -f Dockerfile.osi_vis -t osi-vis .

# IMAGE BUILDS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


# CONTAINERS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
start-osi-visualizer:
	docker-compose run --rm osi_vis osi-visualizer

# CONTAINERS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
