##########################################################################
# File Name: run.sh
# Author: dengqi
# mail: 906262038@qq.com
# Created Time: 2019年07月22日 星期一 16时57分08秒
#########################################################################
#!/bin/bash
PATH=/home/dengqi/bin:/home/dengqi/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin
export PATH
for ((i=13; i<=23; i=i+1))
do 
	./main.out ${i}
done
