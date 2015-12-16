#!/bin/bash
/etc/init.d/networking restart
ip addr flush dev eth0
ip addr flush dev eth1
ifconfig eth0 up
ifconfig eth0 172.16.60.254/24
ifconfig eth1 up
ifconfig eth1 172.16.61.253/24
route add default gw 172.16.61.254
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
