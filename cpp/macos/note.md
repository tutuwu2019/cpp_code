实际场景是这样的，我需要编译一个动态库 libjsonprobuf.dylib，而这个动态库依赖 protobu、abself (因为 probuf 动态库还会依赖很多 absl 的库)、json，
而这三个库已经放在了 third_party 对应的子库目录中，最后，我需要写一个可执行程序来调用 libjsonprobuf.dylib，
事实上，最后，我不仅要调用 libjsonprobuf.dylib，我还要调用 protobuf、json（因为这个可执行程序涉及相应的 库调用）

JsonProtobuf 项目完整 CMake 配置
```text
# ============================================
# 项目结构：
# MyProject/
# ├── CMakeLists.txt (这个文件)
# ├── third_party/
# │   ├── protobuf/
# │   │   ├── lib/
# │   │   │   ├── libprotobuf.dylib
# │   │   │   └── (可能还有其他 protobuf 库)
# │   │   └── include/
# │   ├── abseil/
# │   │   ├── lib/
# │   │   │   ├── libabsl_*.dylib (很多个)
# │   │   └── include/
# │   └── json/
# │       ├── lib/
# │       │   └── libjson.dylib (或 nlohmann_json)
# │       └── include/
# ├── libjsonprotobuf/
# │   ├── CMakeLists.txt
# │   ├── include/
# │   │   └── jsonprotobuf.h
# │   └── src/
# │       └── jsonprotobuf.cpp
# └── app/
#     ├── CMakeLists.txt
#     └── main.cpp
# ============================================

cmake_minimum_required(VERSION 3.15)
project(JsonProtobufProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MACOSX_RPATH TRUE)

# ============================================
# 全局配置
# ============================================

# 定义 third_party 根目录
set(THIRD_PARTY_ROOT "${CMAKE_SOURCE_DIR}/third_party")

# 安装目录配置
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install prefix")

# ============================================
# 第一部分：导入第三方库
# ============================================

# --- Protobuf ---
set(PROTOBUF_ROOT "${THIRD_PARTY_ROOT}/protobuf")

# 查找 protobuf 主库
find_library(PROTOBUF_LIBRARY
    NAMES protobuf libprotobuf
    PATHS "${PROTOBUF_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

message(STATUS "Found Protobuf: ${PROTOBUF_LIBRARY}")

add_library(protobuf SHARED IMPORTED)
set_target_properties(protobuf PROPERTIES
    IMPORTED_LOCATION "${PROTOBUF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PROTOBUF_ROOT}/include"
)

# 如果有其他 protobuf 库（如 libprotobuf-lite.dylib）
# find_library(PROTOBUF_LITE_LIBRARY ...)

# --- Abseil (重要：可能有很多个库) ---
set(ABSEIL_ROOT "${THIRD_PARTY_ROOT}/abseil")

# 方法1：自动查找所有 absl 库
file(GLOB ABSEIL_LIBRARIES "${ABSEIL_ROOT}/lib/libabsl_*.dylib")

if(NOT ABSEIL_LIBRARIES)
    message(WARNING "No Abseil libraries found in ${ABSEIL_ROOT}/lib")
endif()

message(STATUS "Found ${list(LENGTH ABSEIL_LIBRARIES)} Abseil libraries")

# 创建一个 INTERFACE 库来管理所有 absl 依赖
add_library(abseil INTERFACE)
target_include_directories(abseil INTERFACE "${ABSEIL_ROOT}/include")

# 为每个 absl 库创建导入目标
set(ABSEIL_IMPORTED_TARGETS "")
foreach(absl_lib ${ABSEIL_LIBRARIES})
    get_filename_component(absl_name ${absl_lib} NAME_WE)
    string(REPLACE "lib" "" absl_target_name ${absl_name})
    
    add_library(${absl_target_name} SHARED IMPORTED)
    set_target_properties(${absl_target_name} PROPERTIES
        IMPORTED_LOCATION ${absl_lib}
    )
    
    list(APPEND ABSEIL_IMPORTED_TARGETS ${absl_target_name})
    
    # 将每个 absl 库链接到 abseil INTERFACE
    target_link_libraries(abseil INTERFACE ${absl_target_name})
endforeach()

message(STATUS "Abseil targets: ${ABSEIL_IMPORTED_TARGETS}")

# 方法2：如果你知道具体需要哪些 absl 库（更精确）
# set(REQUIRED_ABSL_LIBS
#     absl_strings
#     absl_status
#     absl_synchronization
#     # ... 添加其他需要的
# )
# 
# foreach(absl_name ${REQUIRED_ABSL_LIBS})
#     find_library(${absl_name}_LIBRARY
#         NAMES ${absl_name} lib${absl_name}
#         PATHS "${ABSEIL_ROOT}/lib"
#         NO_DEFAULT_PATH
#         REQUIRED
#     )
#     
#     add_library(${absl_name} SHARED IMPORTED)
#     set_target_properties(${absl_name} PROPERTIES
#         IMPORTED_LOCATION "${${absl_name}_LIBRARY}"
#     )
#     
#     target_link_libraries(abseil INTERFACE ${absl_name})
# endforeach()

# --- JSON (nlohmann_json 或其他) ---
set(JSON_ROOT "${THIRD_PARTY_ROOT}/json")

# 如果是 header-only 的 nlohmann_json
if(EXISTS "${JSON_ROOT}/include/nlohmann/json.hpp")
    add_library(json INTERFACE)
    target_include_directories(json INTERFACE "${JSON_ROOT}/include")
    message(STATUS "Using header-only nlohmann_json")
    set(JSON_IS_HEADER_ONLY TRUE)
else()
    # 如果是编译的库版本
    find_library(JSON_LIBRARY
        NAMES json nlohmann_json libjson
        PATHS "${JSON_ROOT}/lib"
        NO_DEFAULT_PATH
        REQUIRED
    )
    
    add_library(json SHARED IMPORTED)
    set_target_properties(json PROPERTIES
        IMPORTED_LOCATION "${JSON_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${JSON_ROOT}/include"
    )
    message(STATUS "Found JSON library: ${JSON_LIBRARY}")
    set(JSON_IS_HEADER_ONLY FALSE)
endif()

# ============================================
# 第二部分：构建 libjsonprotobuf.dylib
# ============================================

add_subdirectory(libjsonprotobuf)

# 或者直接在这里定义：
add_library(jsonprotobuf SHARED
    libjsonprotobuf/src/jsonprotobuf.cpp
)

target_include_directories(jsonprotobuf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libjsonprotobuf/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libjsonprotobuf/src
)

# 链接第三方库
target_link_libraries(jsonprotobuf
    PUBLIC
        protobuf    # PUBLIC: 因为你的头文件可能暴露 protobuf 类型
        json        # PUBLIC: 同理
    PRIVATE
        abseil      # PRIVATE: 内部实现使用
)

# 收集所有依赖库的路径（用于 RPATH）
set(ALL_DEPENDENCY_LIBS 
    ${PROTOBUF_LIBRARY}
    ${ABSEIL_LIBRARIES}
)

if(NOT JSON_IS_HEADER_ONLY)
    list(APPEND ALL_DEPENDENCY_LIBS ${JSON_LIBRARY})
endif()

# 提取所有依赖库的目录
set(DEPENDENCY_LIB_DIRS "")
foreach(dep_lib ${ALL_DEPENDENCY_LIBS})
    get_filename_component(dep_dir ${dep_lib} DIRECTORY)
    list(APPEND DEPENDENCY_LIB_DIRS ${dep_dir})
endforeach()
list(REMOVE_DUPLICATES DEPENDENCY_LIB_DIRS)

message(STATUS "Dependency library directories: ${DEPENDENCY_LIB_DIRS}")

# 配置 libjsonprotobuf.dylib 的 RPATH
set_target_properties(jsonprotobuf PROPERTIES
    # install name
    INSTALL_NAME_DIR "@rpath"
    
    # 开发期 RPATH：指向 third_party
    BUILD_RPATH "${DEPENDENCY_LIB_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    
    # 安装期 RPATH：所有库会在同一目录
    INSTALL_RPATH "@loader_path"
    
    # macOS 必须
    MACOSX_RPATH TRUE
    
    # 输出名称
    OUTPUT_NAME "jsonprotobuf"
    VERSION 1.0.0
    SOVERSION 1
)

# ============================================
# 第三部分：构建可执行程序
# ============================================

add_subdirectory(app)

# 或者直接在这里定义：
add_executable(myapp
    app/main.cpp
)

target_include_directories(myapp PRIVATE
    ${CMAKE_SOURCE_DIR}/libjsonprotobuf/include
)

# 关键：链接 libjsonprotobuf.dylib 和直接依赖
target_link_libraries(myapp PRIVATE
    jsonprotobuf    # 你的库
    protobuf        # 直接使用 protobuf API
    json            # 直接使用 json API
    # 注意：不需要显式链接 abseil，因为它是 jsonprotobuf 的私有依赖
)

# 配置可执行程序的 RPATH
set_target_properties(myapp PROPERTIES
    # 开发期：需要找到 third_party 的库 + 构建目录的 jsonprotobuf
    BUILD_RPATH 
        "${DEPENDENCY_LIB_DIRS}"
        "${CMAKE_BINARY_DIR}/libjsonprotobuf"  # 或 ${CMAKE_BINARY_DIR} 如果在根目录
    
    # 安装期：所有库都在 lib/ 目录
    INSTALL_RPATH "@executable_path/../lib"
    
    MACOSX_RPATH TRUE
)

# ============================================
# 第四部分：安装配置
# ============================================

# 安装 libjsonprotobuf.dylib
install(TARGETS jsonprotobuf
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

# 安装头文件
install(DIRECTORY libjsonprotobuf/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 安装可执行程序
install(TARGETS myapp
    RUNTIME DESTINATION bin
)

# 安装所有第三方依赖库
install(FILES ${ALL_DEPENDENCY_LIBS}
    DESTINATION lib
)

# 安装后修复 libjsonprotobuf.dylib 的依赖路径
install(CODE "
    set(jsonprotobuf_lib \"\${CMAKE_INSTALL_PREFIX}/lib/libjsonprotobuf.dylib\")
    
    message(STATUS \"Fixing libjsonprotobuf.dylib dependencies...\")
    
    # 修改 protobuf 依赖
    execute_process(
        COMMAND install_name_tool 
            -change ${PROTOBUF_LIBRARY} @loader_path/libprotobuf.dylib
            \${jsonprotobuf_lib}
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # 修改所有 absl 依赖
    foreach(absl_lib ${ABSEIL_LIBRARIES})
        get_filename_component(absl_name \${absl_lib} NAME)
        execute_process(
            COMMAND install_name_tool 
                -change \${absl_lib} @loader_path/\${absl_name}
                \${jsonprotobuf_lib}
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 如果 json 不是 header-only
    if(NOT ${JSON_IS_HEADER_ONLY})
        execute_process(
            COMMAND install_name_tool 
                -change ${JSON_LIBRARY} @loader_path/libjson.dylib
                \${jsonprotobuf_lib}
            OUTPUT_QUIET ERROR_QUIET
        )
    endif()
    
    message(STATUS \"Fixed libjsonprotobuf.dylib dependencies\")
")

# 安装后修复 myapp 的依赖路径
install(CODE "
    set(myapp_bin \"\${CMAKE_INSTALL_PREFIX}/bin/myapp\")
    
    message(STATUS \"Fixing myapp dependencies...\")
    
    # 修改 jsonprotobuf 引用
    execute_process(
        COMMAND install_name_tool 
            -change @rpath/libjsonprotobuf.dylib @executable_path/../lib/libjsonprotobuf.dylib
            \${myapp_bin}
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # 修改 protobuf 引用
    execute_process(
        COMMAND install_name_tool 
            -change ${PROTOBUF_LIBRARY} @executable_path/../lib/libprotobuf.dylib
            \${myapp_bin}
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # 修改 json 引用（如果不是 header-only）
    if(NOT ${JSON_IS_HEADER_ONLY})
        execute_process(
            COMMAND install_name_tool 
                -change ${JSON_LIBRARY} @executable_path/../lib/libjson.dylib
                \${myapp_bin}
            OUTPUT_QUIET ERROR_QUIET
        )
    endif()
    
    message(STATUS \"Fixed myapp dependencies\")
")

# 可选：修改第三方库的 install name（使其使用 @rpath）
install(CODE "
    message(STATUS \"Fixing third-party library install names...\")
    
    foreach(dep_lib ${ALL_DEPENDENCY_LIBS})
        get_filename_component(dep_name \${dep_lib} NAME)
        set(installed_lib \"\${CMAKE_INSTALL_PREFIX}/lib/\${dep_name}\")
        
        execute_process(
            COMMAND install_name_tool 
                -id @rpath/\${dep_name}
                \${installed_lib}
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    message(STATUS \"Fixed third-party library install names\")
")

# ============================================
# 第五部分：调试和验证工具
# ============================================

# 打印配置摘要
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Protobuf: ${PROTOBUF_LIBRARY}")
message(STATUS "Abseil: ${list(LENGTH ABSEIL_LIBRARIES)} libraries")
message(STATUS "JSON: ${JSON_IS_HEADER_ONLY}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")

# 构建后自动检查
add_custom_command(TARGET jsonprotobuf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== libjsonprotobuf.dylib dependencies ==="
    COMMAND otool -L $<TARGET_FILE:jsonprotobuf>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== libjsonprotobuf.dylib RPATH ==="
    COMMAND otool -l $<TARGET_FILE:jsonprotobuf> | grep -A 3 LC_RPATH || echo "No RPATH"
    COMMENT "Checking libjsonprotobuf.dylib"
)

add_custom_command(TARGET myapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== myapp dependencies ==="
    COMMAND otool -L $<TARGET_FILE:myapp>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== myapp RPATH ==="
    COMMAND otool -l $<TARGET_FILE:myapp> | grep -A 3 LC_RPATH || echo "No RPATH"
    COMMENT "Checking myapp"
)

# 自定义目标：详细检查
add_custom_target(check-all
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Checking libjsonprotobuf.dylib"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND otool -L $<TARGET_FILE:jsonprotobuf>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Checking myapp"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND otool -L $<TARGET_FILE:myapp>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Testing myapp execution"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND $<TARGET_FILE:myapp> --version || ${CMAKE_COMMAND} -E echo "Add --version support or test command"
    DEPENDS jsonprotobuf myapp
    COMMENT "Running comprehensive checks"
)

# 测试运行时加载
add_custom_target(test-runtime
    COMMAND ${CMAKE_COMMAND} -E echo "Testing runtime library loading..."
    COMMAND DYLD_PRINT_LIBRARIES=1 $<TARGET_FILE:myapp> || true
    DEPENDS myapp
    COMMENT "Testing runtime library resolution"
)

# ============================================
# 使用说明
# ============================================

# 1. 确保 third_party 目录结构正确：
#    third_party/
#    ├── protobuf/lib/libprotobuf.dylib
#    ├── abseil/lib/libabsl_*.dylib
#    └── json/lib/ (或 include/ 如果是 header-only)

# 2. 构建：
#    mkdir build && cd build
#    cmake ..
#    make -j4

# 3. 检查（重要！）：
#    make check-all
#    
#    应该看到：
#    - libjsonprotobuf.dylib 依赖 third_party 中的绝对路径
#    - myapp 依赖 libjsonprotobuf.dylib 和 third_party 的绝对路径

# 4. 测试运行（在构建目录）：
#    ./app/myapp
#    # 应该能正常运行，因为 BUILD_RPATH 指向了 third_party

# 5. 安装：
#    make install
#    
# 6. 验证安装：
#    cd ../install
#    tree .
#    # 应该看到：
#    # ├── bin/myapp
#    # └── lib/
#    #     ├── libjsonprotobuf.dylib
#    #     ├── libprotobuf.dylib
#    #     ├── libabsl_*.dylib (很多个)
#    #     └── libjson.dylib (如果有)
#    
#    otool -L lib/libjsonprotobuf.dylib
#    # 应该看到 @loader_path/libprotobuf.dylib 等
#    
#    otool -L bin/myapp
#    # 应该看到 @executable_path/../lib/... 

# 7. 最终测试：
#    ./bin/myapp
#    # 应该能正常运行！
```

子目录分离的CMkae 配置
```text
# ============================================
# 根目录 CMakeLists.txt
# ============================================

cmake_minimum_required(VERSION 3.15)
project(JsonProtobufProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH TRUE)

# 定义全局变量
set(THIRD_PARTY_ROOT "${CMAKE_SOURCE_DIR}/third_party")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install prefix")

# 导入第三方库的辅助函数
function(import_third_party_lib LIB_NAME LIB_SUBDIR)
    set(LIB_ROOT "${THIRD_PARTY_ROOT}/${LIB_SUBDIR}")
    
    file(GLOB LIB_FILES "${LIB_ROOT}/lib/lib${LIB_NAME}*.dylib")
    
    if(NOT LIB_FILES)
        message(FATAL_ERROR "${LIB_NAME} libraries not found in ${LIB_ROOT}/lib")
    endif()
    
    # 返回找到的库文件列表到父作用域
    set(${LIB_NAME}_LIBRARIES ${LIB_FILES} PARENT_SCOPE)
    set(${LIB_NAME}_INCLUDE_DIR "${LIB_ROOT}/include" PARENT_SCOPE)
    
    message(STATUS "Found ${LIB_NAME}: ${list(LENGTH LIB_FILES)} libraries")
endfunction()

# 导入 Protobuf
import_third_party_lib(protobuf "protobuf")
add_library(protobuf SHARED IMPORTED)
list(GET protobuf_LIBRARIES 0 PROTOBUF_MAIN_LIB)  # 取第一个作为主库
set_target_properties(protobuf PROPERTIES
    IMPORTED_LOCATION "${PROTOBUF_MAIN_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${protobuf_INCLUDE_DIR}"
)

# 导入 Abseil (所有库)
file(GLOB ABSEIL_LIBRARIES "${THIRD_PARTY_ROOT}/abseil/lib/libabsl_*.dylib")
set(abseil_INCLUDE_DIR "${THIRD_PARTY_ROOT}/abseil/include")

add_library(abseil INTERFACE)
target_include_directories(abseil INTERFACE "${abseil_INCLUDE_DIR}")

foreach(absl_lib ${ABSEIL_LIBRARIES})
    get_filename_component(absl_name ${absl_lib} NAME_WE)
    string(REPLACE "lib" "" absl_target ${absl_name})
    
    add_library(${absl_target} SHARED IMPORTED)
    set_target_properties(${absl_target} PROPERTIES IMPORTED_LOCATION ${absl_lib})
    target_link_libraries(abseil INTERFACE ${absl_target})
endforeach()

message(STATUS "Found Abseil: ${list(LENGTH ABSEIL_LIBRARIES)} libraries")

# 导入 JSON
set(JSON_INCLUDE_DIR "${THIRD_PARTY_ROOT}/json/include")
if(EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
    # Header-only
    add_library(json INTERFACE)
    target_include_directories(json INTERFACE "${JSON_INCLUDE_DIR}")
    set(JSON_IS_HEADER_ONLY TRUE)
    set(JSON_LIBRARIES "")
else()
    # 编译版本
    import_third_party_lib(json "json")
    add_library(json SHARED IMPORTED)
    list(GET json_LIBRARIES 0 JSON_MAIN_LIB)
    set_target_properties(json PROPERTIES
        IMPORTED_LOCATION "${JSON_MAIN_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${json_INCLUDE_DIR}"
    )
    set(JSON_IS_HEADER_ONLY FALSE)
endif()

# 收集所有依赖库（供子项目使用）
set(ALL_THIRD_PARTY_LIBS 
    ${protobuf_LIBRARIES}
    ${ABSEIL_LIBRARIES}
    ${JSON_LIBRARIES}
)

# 提取依赖库目录
set(THIRD_PARTY_LIB_DIRS "")
foreach(lib ${ALL_THIRD_PARTY_LIBS})
    get_filename_component(lib_dir ${lib} DIRECTORY)
    list(APPEND THIRD_PARTY_LIB_DIRS ${lib_dir})
endforeach()
list(REMOVE_DUPLICATES THIRD_PARTY_LIB_DIRS)

# 添加子目录
add_subdirectory(libjsonprotobuf)
add_subdirectory(app)

# 全局安装规则
install(FILES ${ALL_THIRD_PARTY_LIBS} DESTINATION lib)

# 修复第三方库的 install name
install(CODE "
    foreach(lib ${ALL_THIRD_PARTY_LIBS})
        get_filename_component(lib_name \${lib} NAME)
        execute_process(
            COMMAND install_name_tool -id @rpath/\${lib_name}
                    \${CMAKE_INSTALL_PREFIX}/lib/\${lib_name}
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
")

# ============================================
# libjsonprotobuf/CMakeLists.txt
# ============================================

add_library(jsonprotobuf SHARED
    src/jsonprotobuf.cpp
    # 添加更多源文件
)

target_include_directories(jsonprotobuf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(jsonprotobuf
    PUBLIC
        protobuf
        json
    PRIVATE
        abseil
)

# 配置 RPATH
set_target_properties(jsonprotobuf PROPERTIES
    INSTALL_NAME_DIR "@rpath"
    
    # 开发期：指向 third_party
    BUILD_RPATH "${THIRD_PARTY_LIB_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    
    # 部署期：所有库在同一目录
    INSTALL_RPATH "@loader_path"
    
    MACOSX_RPATH TRUE
    OUTPUT_NAME "jsonprotobuf"
    VERSION 1.0.0
)

# 安装
install(TARGETS jsonprotobuf
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 修复依赖路径
install(CODE "
    set(lib_path \"\${CMAKE_INSTALL_PREFIX}/lib/libjsonprotobuf.dylib\")
    
    # 修改 protobuf 引用
    foreach(pb_lib ${protobuf_LIBRARIES})
        get_filename_component(pb_name \${pb_lib} NAME)
        execute_process(
            COMMAND install_name_tool -change \${pb_lib} @loader_path/\${pb_name} \${lib_path}
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 修改 abseil 引用
    foreach(absl_lib ${ABSEIL_LIBRARIES})
        get_filename_component(absl_name \${absl_lib} NAME)
        execute_process(
            COMMAND install_name_tool -change \${absl_lib} @loader_path/\${absl_name} \${lib_path}
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 修改 json 引用（如果不是 header-only）
    if(NOT ${JSON_IS_HEADER_ONLY})
        foreach(json_lib ${JSON_LIBRARIES})
            get_filename_component(json_name \${json_lib} NAME)
            execute_process(
                COMMAND install_name_tool -change \${json_lib} @loader_path/\${json_name} \${lib_path}
                OUTPUT_QUIET ERROR_QUIET
            )
        endforeach()
    endif()
")

# ============================================
# app/CMakeLists.txt
# ============================================

add_executable(myapp
    main.cpp
)

target_include_directories(myapp PRIVATE
    ${CMAKE_SOURCE_DIR}/libjsonprotobuf/include
)

# 链接 libjsonprotobuf 和直接依赖
target_link_libraries(myapp PRIVATE
    jsonprotobuf
    protobuf
    json
    # 不需要链接 abseil，它是 jsonprotobuf 的私有依赖
)

# 配置 RPATH
set_target_properties(myapp PROPERTIES
    # 开发期：需要找到 third_party 和 libjsonprotobuf
    BUILD_RPATH 
        "${THIRD_PARTY_LIB_DIRS}"
        "${CMAKE_BINARY_DIR}/libjsonprotobuf"
    
    # 部署期：所有库在 ../lib
    INSTALL_RPATH "@executable_path/../lib"
    
    MACOSX_RPATH TRUE
)

# 安装
install(TARGETS myapp
    RUNTIME DESTINATION bin
)

# 修复依赖路径
install(CODE "
    set(app_path \"\${CMAKE_INSTALL_PREFIX}/bin/myapp\")
    
    # 修改 jsonprotobuf 引用
    execute_process(
        COMMAND install_name_tool 
            -change @rpath/libjsonprotobuf.dylib 
            @executable_path/../lib/libjsonprotobuf.dylib
            \${app_path}
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # 修改 protobuf 引用
    foreach(pb_lib ${protobuf_LIBRARIES})
        get_filename_component(pb_name \${pb_lib} NAME)
        execute_process(
            COMMAND install_name_tool 
                -change \${pb_lib} 
                @executable_path

```
