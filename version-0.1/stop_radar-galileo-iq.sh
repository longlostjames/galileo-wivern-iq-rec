
pkill -f run_galileo-iqdata_acquisition
sleep 2
pkill -SIGINT -f radar-galileo-iq-rec
echo "stopped radar-galileo-iq recording"

sleep 2

