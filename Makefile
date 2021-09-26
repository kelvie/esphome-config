##
# ESPhome config for our home
#
# @file
# @version 0.1

file = office.yaml
file = m5stick-c1.yaml

default: run

run:
	docker run --network=host --rm -v $$PWD:/config -it esphome/esphome run $(file)

logs:
	docker run --network=host --rm -v $$PWD:/config -it esphome/esphome logs $(file)

compile:
	docker run --network=host --rm -v $$PWD:/config -it esphome/esphome compile  $(file)
# end
