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
	docker run $(docker_opts) --network=host --rm -v $$PWD:/config -i esphome/esphome run $(file)

upload:
	docker run $(docker_opts) --network=host --rm -v $$PWD:/config -i esphome/esphome run $(file) --no-logs

logs:
	docker run $(docker_opts) --network=host --rm -v $$PWD:/config -i esphome/esphome logs $(file)

compile:
	docker run $(docker_opts) --network=host --rm -v $$PWD:/config -i esphome/esphome compile  $(file)

%.target:
	$(MAKE) file=$(basename $@).yaml upload

update: office-atom-neokey.target office.target
# update: m5stick-c1.yaml

# end
