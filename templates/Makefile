default:
	yq --from-file sonoff-s31.yq sonoff-s31-values.yaml -s '"../" + .esphome.name'
	rename .yml .yaml ../*.yml

view:
	yq --from-file sonoff-s31.yq sonoff-s31-values.yaml
