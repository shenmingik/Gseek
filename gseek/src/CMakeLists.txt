# 定义了SRC_LIST变量，包含当前目录下所有的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./sql SQL_LIST)
aux_source_directory(./utf UTF_LIST)

# 指定生成可执行文件
add_executable(gseek ${UTF_LIST} ${SRC_LIST} ${SQL_LIST} )
# 指定可执行文件链接时需要的库文件
target_link_libraries(gseek sqlite3 expat)