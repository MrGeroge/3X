#!/bin/bash
IPHEAD=192.168.0.
for i in  63 64
do
echo $i
        scp /home/cq/xnode root@$IPHEAD$i:/home/cq
        scp /home/cq/config root@$IPHEAD$i:/home/cq
done


