##
# ESPhome config for our home
#
# @file
# @version 0.1

file = office.yaml
file = m5stick-c1.yaml
docker_opts = -e TZ=America/Vancouver --network=host --rm -i
ifneq ("$(wildcard /dev/ttyUSB0)","")
	docker_opts += --device=/dev/ttyUSB0
endif

default: run

run:
	docker run $(docker_opts) -v $$PWD:/config esphome/esphome run $(file)

upload:
	docker run $(docker_opts) -v $$PWD:/config esphome/esphome run $(file) --no-logs

logs:
	docker run $(docker_opts) -v $$PWD:/config esphome/esphome logs $(file)

compile:
	docker run $(docker_opts) -v $$PWD:/config esphome/esphome compile  $(file)

%.target:
	$(MAKE) file=$(basename $@).yaml upload

update: office-atom-neokey.target office.target
# m5stick-c1.target

# end
