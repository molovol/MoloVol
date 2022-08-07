#!/bin/bash
# This script launches the headless version of the application by creating a container in docker and then instantly kill it after getting the result
docker run -it --rm bsvogler/molovol:back-end $@
