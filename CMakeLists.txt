cmake_minimum_required(VERSION 3.9)
project(PRO_KIT)

message("\n\tQt使用qmake管理更加专业\n")

#-----------------------------------------------------------------------------
ADD_CUSTOM_TARGET(BUILD_KIT ALL
        COMMENT "Create Application"
        )

ADD_CUSTOM_COMMAND(TARGET BUILD_KIT
        POST_BUILD #PRE_BUILD PRE_LINK POST_BUILD
        COMMENT "\nCreate QT Makefile & make\n"
        COMMAND make -C ${PROJECT_SOURCE_DIR}/src/ui clean
        COMMAND qmake ${PROJECT_SOURCE_DIR}/src/ui -o ${PROJECT_SOURCE_DIR}/src/ui/Makefile
        #COMMAND qmake-qt4 ${PROJECT_SOURCE_DIR}/src/ui -o ${PROJECT_SOURCE_DIR}/src/ui/Makefile
        COMMAND make -C ${PROJECT_SOURCE_DIR}/src/ui
        COMMAND cp ${PROJECT_SOURCE_DIR}/src/ui/kit ~/share
        )
#-----------------------------------------------------------------------------
