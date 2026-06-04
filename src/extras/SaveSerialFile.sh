if  [ $# != 1 ]; then
	printf "Usage: $0 <port>\n"
	exit 2
fi

PORT="$1"

if  [ -e "$PORT" ]; then
	rm -f rawScanData
	mkfifo -m 777 rawScanData
	kill -SIGUSR1 "$PPID" 
#	stty -f "$PORT" 9600 cs8 -cstopb -parenb -ixon -ixoff
	cu -l "$PORT" -s 9600 > rawScanData
else
	printf "%s is either busy or wrong\n" "$PORT"
	kill -SIGUSR2 "$PPID"
	exit 1
fi
