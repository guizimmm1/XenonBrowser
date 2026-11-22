cmake_minimum_required(VERSION 3.20)

project(XenonServer C)

add_executable(XenonServer
    main.c
)

xsdk_add_xex(XenonServer
    TITLE_ID 0x58534252
    NAME "Xenon Server"
    VERSION 1.0.0.0
)
