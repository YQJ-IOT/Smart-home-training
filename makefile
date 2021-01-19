#项目产生如下两个可执行文件
OBJ: sock-tcp-ser se-xiang-tou

#编译这两个执行文件目标
sock-tcp-ser:sock-tcp-ser.c
	gcc sock-tcp-ser.c -o sock-tcp-ser -l pthread -g
se-xiang-tou:se-xiang-tou.c
	gcc se-xiang-tou.c -o se-xiang-tou -g

#伪目标
.phony:clean
clean:
	rm -rf sock-tcp-ser se-xiang-tou text*
	#删除项目产生的两个可执行文件与text.jgp
