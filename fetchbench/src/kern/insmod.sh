#!/bin/bash

sudo rm /dev/tester
sudo rmmod tester
make
sudo insmod tester.ko
sudo mknod /dev/tester c 100 0
