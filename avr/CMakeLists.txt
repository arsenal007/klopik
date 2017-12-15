cmake_minimum_required(VERSION 2.8.4)


project(klopik)

set(PROGRAMMER usbasp)
set(MMCU atmega168a)
set(FREQ 12000000)

SET(SRC_DIR src)
set(SOURCE_FILES 
	${SRC_DIR}/main.c 
	${SRC_DIR}/i2c_master.c 
	${SRC_DIR}/kenwood.c 
	${SRC_DIR}/si5351.c 
	${SRC_DIR}/uart.c
)

include(gcc-avr.cmake)
