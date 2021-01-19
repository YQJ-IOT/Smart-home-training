#ifndef __PBULIC_H__
#define __PBULIC_H__
#include<stdbool.h>

//注意：使用时把该文件复制一份到QT项目
//项目需要的指令数据，温湿度数据封装在该结构体
struct PBULIC{
	char type[16];	       //该字符串=QT表示QT数据，=LIB表示数据库数据			       
	bool pz_cmd;           //拍照指令,ture--拍照
	bool wd_cmd;           //温度数据
	bool sd_cmd;           //湿度数据
	bool light_cmd;        //光照采集
	bool ledon_cmd;        //led1灯泡
	bool ledoff_cmd;       //led1灯泡
	bool labaon_cmd;       //喇叭
	bool labaoff_cmd;      //喇叭
	bool fensanon_cmd;     //风扇启动命令
	bool fensanoff_cmd;    //风扇启动命令
	float wd_shuju;        //温度数据
	float sd_shuju;        //湿度数据
	float light;           //光照数据
};

#endif
