#!/bin/bash


SYSARCH_BRANCH="master"
SYSARCH_BASEDIR="Middlewares/Third_Party/LwIP/system"
SYSARCH_BASEREPO=https://raw.githubusercontent.com/STMicroelectronics/STM32CubeF7/${SYSARCH_BRANCH}/${SYSARCH_BASEDIR}
SYSARCH_HEADER_REPO=${SYSARCH_BASEREPO}/arch
SYSARCH_SRCS_REPO=${SYSARCH_BASEREPO}/OS

function download_sysarch_srcs()
{
	if [ -d Third_Party/STM32CubeF7/${SYSARCH_BASEDIR} ]
	then
		:
	else
		wget "${SYSARCH_HEADER_REPO}/cc.h"
		wget "${SYSARCH_HEADER_REPO}/cpu.h"
		wget "${SYSARCH_HEADER_REPO}/sys_arch.h"
		wget "${SYSARCH_SRCS_REPO}/sys_arch.c"
		mkdir -p Third_Party/STM32CubeF7/${SYSARCH_BASEDIR}/arch
		mkdir -p Third_Party/STM32CubeF7/${SYSARCH_BASEDIR}/OS
		mv cc.h Third_Party/STM32CubeF7/${SYSARCH_BASEDIR}/arch/
		mv cpu.h Third_Party/STM32CubeF7/${SYSARCH_BASEDIR}/arch/
		mv sys_arch.h Third_Party/STM32CubeF7/${SYSARCH_BASEDIR}/arch/
		mv sys_arch.c Third_Party/STM32CubeF7/${SYSARCH_BASEDIR}/OS/
	fi
}

download_sysarch_srcs
