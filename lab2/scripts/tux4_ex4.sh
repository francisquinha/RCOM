#!/bin/bash
route del -net 172.16.61.0/24 gw 172.16.61.253
route add -net 172.16.61.0/24 gw 172.16.61.254
route del default gw 172.16.60.254
route add default gw 172.16.61.254
route -n

