cmake_minimum_required(VERSION 3.9)
project(PRO_KIT)

message("\n\tQt使用qmake管理更加专业\n")

#-----------------------------------------------------------------------------
ADD_CUSTOM_TARGET(SELF_CMD ALL
        COMMENT "Create Application"
        )

ADD_CUSTOM_COMMAND(TARGET SELF_CMD
        POST_BUILD #PRE_BUILD PRE_LINK POST_BUILD
        COMMENT "\nCreate QT Makefile & make\n "
        COMMAND qmake ${PROJECT_SOURCE_DIR}/src/ui -o ${PROJECT_SOURCE_DIR}/src/ui/Makefile
        COMMAND make -C ${PROJECT_SOURCE_DIR}/src/ui
        )
#-----------------------------------------------------------------------------
