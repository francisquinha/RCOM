#!/bin/bash
/etc/init.d/networking restart
ip addr flush dev eth0
ifconfig eth0 up
ifconfig eth0 172.16.61.1/24
route add -net 172.16.60.0/24 gw 172.16.61.253
route add default gw 172.16.61.254
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
echo 1 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 1 > /proc/sys/net/ipv4/conf/all/accept_redirects
