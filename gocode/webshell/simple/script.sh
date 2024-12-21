mkfifo ctos
mkfifo stoc
./testclient/ticker <stoc >ctos&
./server <ctos >stoc&
