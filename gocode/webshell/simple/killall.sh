pids=$(ps | awk '$4=="./server" || /a\.out/ {print $1}');
for p in $pids
do 
	kill $p;
done	 
