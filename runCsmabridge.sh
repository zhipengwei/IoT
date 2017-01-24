packets_per_flow=10

rm -rf ../traces
mkdir ../traces

for numberOfNodes in 100 200 300
do
	for lambda in 1 2 5 10
	do
		for linkCapacity in 100 150 200	
		do
			cat $1 | tr "NUMBER_OF_TERMINALS 300" "NUMBER_OF_TERMINALS $numberOfNodes" | tr "EXPERIMENT_CONFIG_SERVER_LINK_DATA_RATE 133000000" "EXPERIMENT_CONFIG_SERVER_LINK_DATA_RATE $linkCapacity" | tr "EXPERIMENT_SENDER_DOWNTIME_MEAN 1" "EXPERIMENT_SENDER_DOWNTIME_MEAN $lambda" | tr "EXPERIMENT_SENDER_PACKETS_PER_SHORT_FLOW 10" "EXPERIMENT_SENDER_PACKETS_PER_SHORT_FLOW $packets_per_flow"> config.h 
			echo " " >> csmabridge.cc
			cd ..
			./waf 
			echo "Compilation finished!"
			./waf --run csmabridge 2>&1 | cat > traces/tmp_${numberOfNodes}_${lambda}_${linkCapacity} 
			cd scratch

		done
	done
done

