##
# ESPhome config for our home
#
# @file
# @version 0.1

file = office.yaml
file = m5stick-c1.yaml
docker_opts = --device=/dev/ttyUSB0
docker_opts =

default: run

run:
	docker run $(docker_opts) --network=host --rm -v $$PWD:/config -it esphome/esphome run $(file)

logs:
	docker run $(docker_opts) --network=host --rm -v $$PWD:/config -it esphome/esphome logs $(file)

compile:
	docker run $(docker_opts) --network=host --rm -v $$PWD:/config -it esphome/esphome compile  $(file)
# end
