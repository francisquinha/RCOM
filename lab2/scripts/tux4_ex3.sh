#!/bin/bash
ifconfig eth1 up
ifconfig eth1 172.16.61.253/24
route add -net 172.16.61.0/24 gw 172.16.61.253
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
