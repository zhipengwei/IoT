set -x
trap read debug
packets_per_flow=20

rm -rf ../traces
mkdir ../traces

for numberOfNodes in 300
do
	for lambda in 0.1
	do
		for linkCapacity in 100000000
		do
			cat $1 > config.h
			echo "#define NUMBER_OF_TERMINALS $numberOfNodes" >> config.h
			echo "#define EXPERIMENT_CONFIG_SERVER_LINK_DATA_RATE $linkCapacity" >> config.h
			echo "#define EXPERIMENT_CONFIG_SENDER_DOWNTIME_MEAN $lambda" >> config.h
		        echo "#define EXPERIMENT_CONFIG_SENDER_PACKETS_PER_SHORT_FLOW $packets_per_flow" >> config.h
			echo " " >> csmabridge.cc
			cd ..
			./waf 
			echo "Compilation finished!"
			./waf --run csmabridge 2>&1 | cat > traces/tmp_${numberOfNodes}_${lambda}_${linkCapacity} 
			cd scratch

		done
	done
done

