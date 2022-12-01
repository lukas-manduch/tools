#/bin/zsh
if [ $# -ne 1 ]; then
	echo "Argument required - output file name, or 'syslog' or 'journal'"
	exit 1
fi

argument=$1

echo -n "Startig logging test at "
date 
echo "------------------------"
if command -v python3 &>/dev/null ; then
	python3 -m tlog $argument
else
	python -m tlog $argument
fi
echo "------------------------"

echo -n "finished at "
date 

echo

echo "Killing log servers:"
padding="                   "
log_servers=( "systemd-journal"  "rsyslogd", "ekanite")
for server in ${log_servers[@]}; do
	ps -A | grep $server > /dev/null
	if [ $? -ne 0 ]; then
		printf "%s%s not found \n" "$server" "${padding:${#server}}"
	else
		printf "%s%s FOUND  - " "$server" "${padding:${#server}}"
		ps -A | grep $server | awk '{$1=$1};1' | cut -f1 -d" " | xargs kill -9  2>/dev/null
		echo "Kill return code: $?"
	fi
done;

