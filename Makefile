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

ifneq ("$(wildcard /dev/ttyACM1)","")
	docker_opts += --device=/dev/ttyACM1
endif

esphome_cmd = . venv/bin/activate; esphome
# Uncomment to use docker
# esphome_cmd = docker run $(docker_opts) -v $$PWD:/config esphome/esphome


default: run

run:
	$(esphome_cmd) run $(file)

upload:
	$(esphome_cmd) run $(file) --no-logs

logs:
	$(esphome_cmd) logs $(file)

compile:
	$(esphome_cmd) compile $(file)

ips:
	grep -h static_ip *.yaml | sort -n

%.target:
	$(MAKE) file=$(basename $@).yaml upload

FILES_TO_UPDATE = $(patsubst %.yaml,%.target,$(wildcard sonoff*.yaml))
update: $(FILES_TO_UPDATE)
# m5stick-c1.target

# end
