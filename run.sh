
for numberOfNodes in 100 200 300
do
	for lambda in 1 2 5 10
	do
		for linkCapacity in 100 150 200	
		do
			cat $1 | tr "NUMBER_OF_TERMINALS 300" "NUMBER_OF_TERMINALS $numberOfNodes" | tr "EXPERIMENT_CONFIG_SERVER_LINK_DATA_RATE 13300000" "EXPERIMENT_CONFIG_SERVER_LINK_DATA_RATE $linkCapacity" | tr "EXPERIMENT_SENDER_DOWNTIME_LAMBDA 1" "EXPERIMENT_SENDER_DOWNTIME_LAMBDA $lambda" > configOnOff.h
			cd ..
			./waf 
			./waf --run csmaBridgeOnOff 2>&1 | cat > tmp_${numberOfNodes}_${lambda}_${linkCapacity}

		done
	done
done

