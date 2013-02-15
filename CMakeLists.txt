# ########## Project setup ##########
PROJECT(vireqx)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# ######### General setup ##########
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

SUBDIRS("doc")

# ########## vireqx executables ##########
# Sources:
SET(vireqx_executable_SRCS
    vireq.c
    vimkidx.c
)

# Headers:
SET(vireqx_executable_HDRS
    vireq.h
    version.h
)

# actual targets:
ADD_EXECUTABLE(vireqx vireq.c)
ADD_EXECUTABLE(vimkidx vimkidx.c)

# add install targets:
INSTALL(TARGETS vireqx DESTINATION /opt/vireqx)
INSTALL(TARGETS vimkidx DESTINATION /opt/vireqx)

INSTALL(FILES README DESTINATION /opt/vireqx/doc)
INSTALL(FILES COPYING DESTINATION /opt/vireqx/doc)
INSTALL(FILES whatsnew DESTINATION /opt/vireqx/doc)

