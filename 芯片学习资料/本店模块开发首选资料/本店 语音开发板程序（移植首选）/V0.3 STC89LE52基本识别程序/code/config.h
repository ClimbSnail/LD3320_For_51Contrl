/******************** 乐声电子 ***************
* File Name          : config.h
* Author             : http://yuesheng001.taobao.com/
* Version						 : zds0.0.3
* Date First Issued  : 10/12/2011
* Description        : public header file
*******************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H
/*****相关宏定义***/
    /* exact-width signed integer types */
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;
typedef   signed       long int int64_t;

    /* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;
typedef unsigned       long int uint64_t;

/****头文件包含*****/
#include <intrins.h>
#include "reg52.H"
#include "LDChip.h"
#include "Reg_RW.h"
#include "usart.h"




#endif
