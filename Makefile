##
# ESPhome config for our home
#
# @file
# @version 0.1

default: run

run:
	docker run --network=host --rm -v $$PWD:/config -it esphome/esphome run office.yaml

logs:
	docker run --network=host --rm -v $$PWD:/config -it esphome/esphome logs office.yaml

# end
