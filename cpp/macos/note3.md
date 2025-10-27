## 优化3

> 是这样的， josnprotobuf 动态库肯定也是 在 third_party 中保存的 (注意 jsonprobuf 在 third_party ，而且这个动态库是由我构建编译的，我后面会调用它)，那么我应该如何完美的构建  jsonprobuf 结构呢？ 以及我在业务模块调用 jsonprobuf 动态库。（对了，json 库用的是 jsoncpp）
>
> 
```text
# ============================================
# 项目结构：
# MyProject/
# ├── third_party/
# │   ├── protobuf/
# │   │   ├── lib/libprotobuf.dylib
# │   │   └── include/google/protobuf/...
# │   ├── abseil/
# │   │   ├── lib/libabsl_*.dylib (很多个)
# │   │   └── include/absl/...
# │   ├── jsoncpp/
# │   │   ├── lib/libjsoncpp.dylib
# │   │   └── include/json/...
# │   └── jsonprotobuf/
# │       ├── lib/libjsonprotobuf.dylib  ← 你编译的
# │       └── include/jsonprotobuf/...
# ├── libjsonprotobuf_build/  ← 第一步：构建 jsonprotobuf
# │   ├── CMakeLists.txt
# │   ├── include/jsonprotobuf/
# │   │   └── jsonprotobuf.h
# │   └── src/
# │       └── jsonprotobuf.cpp
# └── business_module/  ← 第二步：业务模块
#     ├── CMakeLists.txt
#     └── src/
#         └── main.cpp
# ============================================

# ============================================
# 第一部分：构建 libjsonprotobuf.dylib
# libjsonprotobuf_build/CMakeLists.txt
# ============================================

cmake_minimum_required(VERSION 3.15)
project(JsonProtobufLibrary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MACOSX_RPATH TRUE)

# 定义 third_party 根目录（相对于这个 CMakeLists.txt）
set(THIRD_PARTY_ROOT "${CMAKE_SOURCE_DIR}/../third_party")

# ============================================
# 1.1 导入 Protobuf
# ============================================

set(PROTOBUF_ROOT "${THIRD_PARTY_ROOT}/protobuf")

# 查找主 protobuf 库
find_library(PROTOBUF_LIBRARY
    NAMES protobuf libprotobuf
    PATHS "${PROTOBUF_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

# 可能还有其他 protobuf 库（protoc, protobuf-lite 等）
file(GLOB PROTOBUF_ALL_LIBRARIES "${PROTOBUF_ROOT}/lib/libprotobuf*.dylib")

add_library(protobuf SHARED IMPORTED)
set_target_properties(protobuf PROPERTIES
    IMPORTED_LOCATION "${PROTOBUF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PROTOBUF_ROOT}/include"
)

message(STATUS "Protobuf library: ${PROTOBUF_LIBRARY}")
message(STATUS "Protobuf includes: ${PROTOBUF_ROOT}/include")

# ============================================
# 1.2 导入 Abseil
# ============================================

set(ABSEIL_ROOT "${THIRD_PARTY_ROOT}/abseil")

file(GLOB ABSEIL_LIBRARIES "${ABSEIL_ROOT}/lib/libabsl_*.dylib")

if(NOT ABSEIL_LIBRARIES)
    message(FATAL_ERROR "No Abseil libraries found in ${ABSEIL_ROOT}/lib")
endif()

# 创建一个 INTERFACE 库管理所有 absl 依赖
add_library(abseil INTERFACE)
target_include_directories(abseil INTERFACE "${ABSEIL_ROOT}/include")

# 为每个 absl 库创建导入目标
foreach(absl_lib ${ABSEIL_LIBRARIES})
    get_filename_component(absl_name ${absl_lib} NAME_WE)
    string(REPLACE "lib" "" absl_target ${absl_name})
    
    add_library(${absl_target} SHARED IMPORTED)
    set_target_properties(${absl_target} PROPERTIES
        IMPORTED_LOCATION ${absl_lib}
    )
    
    target_link_libraries(abseil INTERFACE ${absl_target})
endforeach()

message(STATUS "Found ${list(LENGTH ABSEIL_LIBRARIES)} Abseil libraries")

# ============================================
# 1.3 导入 JsonCpp
# ============================================

set(JSONCPP_ROOT "${THIRD_PARTY_ROOT}/jsoncpp")

find_library(JSONCPP_LIBRARY
    NAMES jsoncpp libjsoncpp
    PATHS "${JSONCPP_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

add_library(jsoncpp SHARED IMPORTED)
set_target_properties(jsoncpp PROPERTIES
    IMPORTED_LOCATION "${JSONCPP_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${JSONCPP_ROOT}/include"
)

message(STATUS "JsonCpp library: ${JSONCPP_LIBRARY}")
message(STATUS "JsonCpp includes: ${JSONCPP_ROOT}/include")

# ============================================
# 1.4 收集所有依赖信息
# ============================================

set(ALL_DEPENDENCY_LIBS
    ${PROTOBUF_ALL_LIBRARIES}
    ${ABSEIL_LIBRARIES}
    ${JSONCPP_LIBRARY}
)

# 提取依赖库目录（用于 RPATH）
set(DEPENDENCY_DIRS "")
foreach(lib ${ALL_DEPENDENCY_LIBS})
    get_filename_component(lib_dir ${lib} DIRECTORY)
    list(APPEND DEPENDENCY_DIRS ${lib_dir})
endforeach()
list(REMOVE_DUPLICATES DEPENDENCY_DIRS)

message(STATUS "Dependency directories: ${DEPENDENCY_DIRS}")

# ============================================
# 1.5 构建 libjsonprotobuf.dylib
# ============================================

add_library(jsonprotobuf SHARED
    src/jsonprotobuf.cpp
    # 添加其他源文件
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
        protobuf    # PUBLIC: 因为头文件可能暴露 protobuf 类型
        jsoncpp     # PUBLIC: 同理
    PRIVATE
        abseil      # PRIVATE: 内部实现使用
)

# 关键：配置 RPATH
set_target_properties(jsonprotobuf PROPERTIES
    # install name 使用 @rpath（灵活）
    INSTALL_NAME_DIR "@rpath"
    
    # 开发期 RPATH：指向 third_party 依赖
    BUILD_RPATH "${DEPENDENCY_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    
    # 安装到 third_party 后：使用 @loader_path
    # 因为所有依赖都在 third_party 的各自目录
    INSTALL_RPATH 
        "@loader_path/../protobuf/lib"
        "@loader_path/../abseil/lib"
        "@loader_path/../jsoncpp/lib"
    
    MACOSX_RPATH TRUE
    OUTPUT_NAME "jsonprotobuf"
    VERSION 1.0.0
    SOVERSION 1
)

# ============================================
# 1.6 安装到 third_party
# ============================================

# 安装库到 third_party/jsonprotobuf/lib
install(TARGETS jsonprotobuf
    LIBRARY DESTINATION "${THIRD_PARTY_ROOT}/jsonprotobuf/lib"
)

# 安装头文件到 third_party/jsonprotobuf/include
install(DIRECTORY include/
    DESTINATION "${THIRD_PARTY_ROOT}/jsonprotobuf/include"
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 安装后修复依赖路径
install(CODE "
    set(installed_lib \"${THIRD_PARTY_ROOT}/jsonprotobuf/lib/libjsonprotobuf.dylib\")
    
    message(STATUS \"Fixing libjsonprotobuf.dylib dependencies...\")
    
    # 修改 protobuf 依赖
    foreach(pb_lib ${PROTOBUF_ALL_LIBRARIES})
        get_filename_component(pb_name \${pb_lib} NAME)
        execute_process(
            COMMAND install_name_tool 
                -change \${pb_lib} 
                @loader_path/../protobuf/lib/\${pb_name}
                \${installed_lib}
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 修改 abseil 依赖
    foreach(absl_lib ${ABSEIL_LIBRARIES})
        get_filename_component(absl_name \${absl_lib} NAME)
        execute_process(
            COMMAND install_name_tool 
                -change \${absl_lib} 
                @loader_path/../abseil/lib/\${absl_name}
                \${installed_lib}
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 修改 jsoncpp 依赖
    get_filename_component(jsoncpp_name \"${JSONCPP_LIBRARY}\" NAME)
    execute_process(
        COMMAND install_name_tool 
            -change ${JSONCPP_LIBRARY} 
            @loader_path/../jsoncpp/lib/\${jsoncpp_name}
            \${installed_lib}
        OUTPUT_QUIET ERROR_QUIET
    )
    
    message(STATUS \"Fixed all dependencies in libjsonprotobuf.dylib\")
    message(STATUS \"Installed to: \${installed_lib}\")
")

# ============================================
# 1.7 验证工具
# ============================================

add_custom_command(TARGET jsonprotobuf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== Building libjsonprotobuf.dylib ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Location: $<TARGET_FILE:jsonprotobuf>"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== Dependencies ==="
    COMMAND otool -L $<TARGET_FILE:jsonprotobuf>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== RPATH ==="
    COMMAND otool -l $<TARGET_FILE:jsonprotobuf> | grep -A 3 LC_RPATH || echo "No RPATH"
)

# 自定义目标：检查安装后的库
add_custom_target(verify-install
    COMMAND ${CMAKE_COMMAND} -E echo "=== Verifying installed library ==="
    COMMAND otool -L "${THIRD_PARTY_ROOT}/jsonprotobuf/lib/libjsonprotobuf.dylib"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== Checking dependencies exist ==="
    COMMAND ls -la "${THIRD_PARTY_ROOT}/protobuf/lib/libprotobuf.dylib" || echo "Missing!"
    COMMAND ls -la "${THIRD_PARTY_ROOT}/jsoncpp/lib/libjsoncpp.dylib" || echo "Missing!"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "All dependencies should use @loader_path"
    COMMENT "Verifying installation"
)

# ============================================
# 构建步骤总结：
# cd libjsonprotobuf_build
# mkdir build && cd build
# cmake ..
# make -j4
# make install  # 安装到 third_party/jsonprotobuf
# make verify-install  # 验证安装
# ============================================


# ============================================
# 第二部分：业务模块使用 libjsonprotobuf.dylib
# business_module/CMakeLists.txt
# ============================================

cmake_minimum_required(VERSION 3.15)
project(BusinessModule)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MACOSX_RPATH TRUE)

# 定义 third_party 根目录
set(THIRD_PARTY_ROOT "${CMAKE_SOURCE_DIR}/../third_party")

# ============================================
# 2.1 导入 libjsonprotobuf.dylib
# ============================================

set(JSONPROTOBUF_ROOT "${THIRD_PARTY_ROOT}/jsonprotobuf")

find_library(JSONPROTOBUF_LIBRARY
    NAMES jsonprotobuf libjsonprotobuf
    PATHS "${JSONPROTOBUF_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

add_library(jsonprotobuf SHARED IMPORTED)
set_target_properties(jsonprotobuf PROPERTIES
    IMPORTED_LOCATION "${JSONPROTOBUF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${JSONPROTOBUF_ROOT}/include"
)

message(STATUS "Found JsonProtobuf: ${JSONPROTOBUF_LIBRARY}")

# ============================================
# 2.2 导入 Protobuf（因为业务代码也需要）
# ============================================

set(PROTOBUF_ROOT "${THIRD_PARTY_ROOT}/protobuf")

find_library(PROTOBUF_LIBRARY
    NAMES protobuf libprotobuf
    PATHS "${PROTOBUF_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

add_library(protobuf SHARED IMPORTED)
set_target_properties(protobuf PROPERTIES
    IMPORTED_LOCATION "${PROTOBUF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PROTOBUF_ROOT}/include"
)

# ============================================
# 2.3 导入 JsonCpp（因为业务代码也需要）
# ============================================

set(JSONCPP_ROOT "${THIRD_PARTY_ROOT}/jsoncpp")

find_library(JSONCPP_LIBRARY
    NAMES jsoncpp libjsoncpp
    PATHS "${JSONCPP_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

add_library(jsoncpp SHARED IMPORTED)
set_target_properties(jsoncpp PROPERTIES
    IMPORTED_LOCATION "${JSONCPP_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${JSONCPP_ROOT}/include"
)

# ============================================
# 2.4 收集所有依赖（用于 RPATH）
# ============================================

# 业务模块直接依赖的库
set(DIRECT_DEPENDENCIES
    ${JSONPROTOBUF_LIBRARY}
    ${PROTOBUF_LIBRARY}
    ${JSONCPP_LIBRARY}
)

# 提取目录
set(DEPENDENCY_DIRS "")
foreach(lib ${DIRECT_DEPENDENCIES})
    get_filename_component(lib_dir ${lib} DIRECTORY)
    list(APPEND DEPENDENCY_DIRS ${lib_dir})
endforeach()
list(REMOVE_DUPLICATES DEPENDENCY_DIRS)

# ============================================
# 2.5 构建业务可执行程序
# ============================================

add_executable(business_app
    src/main.cpp
    # 添加其他业务源文件
)

target_include_directories(business_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(business_app PRIVATE
    jsonprotobuf  # 你的库
    protobuf      # 直接使用
    jsoncpp       # 直接使用
    # 注意：不需要链接 abseil，因为它是 jsonprotobuf 的私有依赖
)

# 配置 RPATH
set_target_properties(business_app PROPERTIES
    # 开发期 RPATH：指向 third_party 的各个库目录
    BUILD_RPATH "${DEPENDENCY_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    
    # 部署期 RPATH：根据你的部署结构调整
    # 方案A：如果部署时所有库在同一目录
    # INSTALL_RPATH "@executable_path/../lib"
    
    # 方案B：如果保持 third_party 结构
    INSTALL_RPATH 
        "@executable_path/../third_party/jsonprotobuf/lib"
        "@executable_path/../third_party/protobuf/lib"
        "@executable_path/../third_party/jsoncpp/lib"
    
    MACOSX_RPATH TRUE
)

# ============================================
# 2.6 安装配置（可选）
# ============================================

# 如果需要部署，可以选择不同策略

# 策略A：保持 third_party 结构
install(TARGETS business_app
    RUNTIME DESTINATION bin
)

# 策略B：将所有库集中到一个 lib 目录
option(BUNDLE_ALL_LIBS "Copy all libraries to lib directory" OFF)

if(BUNDLE_ALL_LIBS)
    # 收集所有需要的库（包括间接依赖）
    file(GLOB PROTOBUF_ALL_LIBS "${PROTOBUF_ROOT}/lib/*.dylib")
    file(GLOB ABSEIL_ALL_LIBS "${THIRD_PARTY_ROOT}/abseil/lib/*.dylib")
    
    set(ALL_LIBS_TO_BUNDLE
        ${JSONPROTOBUF_LIBRARY}
        ${PROTOBUF_ALL_LIBS}
        ${ABSEIL_ALL_LIBS}
        ${JSONCPP_LIBRARY}
    )
    
    # 安装所有库
    install(FILES ${ALL_LIBS_TO_BUNDLE}
        DESTINATION lib
    )
    
    # 修改可执行文件的 RPATH
    set_target_properties(business_app PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
    )
    
    # 修复所有库的依赖路径
    install(CODE "
        set(app_path \"\${CMAKE_INSTALL_PREFIX}/bin/business_app\")
        set(lib_dir \"\${CMAKE_INSTALL_PREFIX}/lib\")
        
        message(STATUS \"Fixing business_app dependencies...\")
        
        # 修改 app 的直接依赖
        foreach(lib ${DIRECT_DEPENDENCIES})
            get_filename_component(lib_name \${lib} NAME)
            execute_process(
                COMMAND install_name_tool 
                    -change \${lib} @executable_path/../lib/\${lib_name}
                    \${app_path}
                OUTPUT_QUIET ERROR_QUIET
            )
        endforeach()
        
        # 修改 jsonprotobuf 的依赖路径（指向同一 lib 目录）
        set(installed_jsonprotobuf \"\${lib_dir}/libjsonprotobuf.dylib\")
        
        foreach(lib ${PROTOBUF_ALL_LIBS} ${ABSEIL_ALL_LIBS})
            get_filename_component(lib_name \${lib} NAME)
            execute_process(
                COMMAND install_name_tool 
                    -change @loader_path/../protobuf/lib/\${lib_name} @loader_path/\${lib_name}
                    \${installed_jsonprotobuf}
                OUTPUT_QUIET ERROR_QUIET
            )
            execute_process(
                COMMAND install_name_tool 
                    -change @loader_path/../abseil/lib/\${lib_name} @loader_path/\${lib_name}
                    \${installed_jsonprotobuf}
                OUTPUT_QUIET ERROR_QUIET
            )
        endforeach()
        
        message(STATUS \"All dependencies fixed\")
    ")
endif()

# ============================================
# 2.7 验证和调试工具
# ============================================

add_custom_command(TARGET business_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== business_app dependencies ==="
    COMMAND otool -L $<TARGET_FILE:business_app>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== business_app RPATH ==="
    COMMAND otool -l $<TARGET_FILE:business_app> | grep -A 3 LC_RPATH || echo "No RPATH"
)

# 测试运行
add_custom_target(test-run
    COMMAND ${CMAKE_COMMAND} -E echo "=== Testing business_app ==="
    COMMAND $<TARGET_FILE:business_app> || echo "Failed to run"
    DEPENDS business_app
)

# 详细的依赖检查
add_custom_target(check-deps
    COMMAND ${CMAKE_COMMAND} -E echo "=== Checking all dependencies ==="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "1. business_app:"
    COMMAND otool -L $<TARGET_FILE:business_app>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "2. libjsonprotobuf.dylib:"
    COMMAND otool -L "${JSONPROTOBUF_LIBRARY}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "3. Testing with DYLD_PRINT_LIBRARIES:"
    COMMAND DYLD_PRINT_LIBRARIES=1 $<TARGET_FILE:business_app> 2>&1 | head -20
    DEPENDS business_app
)

# ============================================
# 使用步骤总结
# ============================================

# 第一步：构建 libjsonprotobuf.dylib
# cd libjsonprotobuf_build
# mkdir build && cd build
# cmake ..
# make -j4
# make install  # 安装到 third_party/jsonprotobuf/
# make verify-install

# 第二步：构建业务模块
# cd ../../business_module
# mkdir build && cd build
# cmake ..
# make -j4

# 第三步：测试运行
# make test-run
# 或直接运行：
# ./business_app

# 第四步：详细检查（如果有问题）
# make check-deps

# 第五步：部署（可选）
# cmake -DBUNDLE_ALL_LIBS=ON ..
# make install
# cd ../install
# ./bin/business_app
```

## 完整版

```text
# JsonProtobuf 完整实战指南

## 第一阶段：构建 libjsonprotobuf.dylib

### 目录结构
```
MyProject/
├── third_party/
│   ├── protobuf/
│   │   ├── lib/libprotobuf.dylib
│   │   └── include/google/protobuf/...
│   ├── abseil/
│   │   ├── lib/libabsl_*.dylib
│   │   └── include/absl/...
│   ├── jsoncpp/
│   │   ├── lib/libjsoncpp.dylib
│   │   └── include/json/...
│   └── jsonprotobuf/  ← 将来安装到这里
│       ├── lib/
│       └── include/
└── libjsonprotobuf_build/
    ├── CMakeLists.txt
    ├── include/jsonprotobuf/
    │   └── jsonprotobuf.h
    └── src/
        └── jsonprotobuf.cpp
```

### 文件内容

#### libjsonprotobuf_build/include/jsonprotobuf/jsonprotobuf.h
```cpp
#ifndef JSONPROTOBUF_H
#define JSONPROTOBUF_H

#include <string>
#include <google/protobuf/message.h>
#include <json/json.h>

namespace jsonprotobuf {

class Converter {
public:
    // Protobuf Message 转 JSON
    static bool MessageToJson(
        const google::protobuf::Message& message,
        Json::Value& json_output);
    
    static std::string MessageToJsonString(
        const google::protobuf::Message& message);
    
    // JSON 转 Protobuf Message
    static bool JsonToMessage(
        const Json::Value& json_input,
        google::protobuf::Message* message);
    
    static bool JsonStringToMessage(
        const std::string& json_string,
        google::protobuf::Message* message);
};

} // namespace jsonprotobuf

#endif // JSONPROTOBUF_H
```

#### libjsonprotobuf_build/src/jsonprotobuf.cpp
```cpp
#include "jsonprotobuf/jsonprotobuf.h"
#include <google/protobuf/util/json_util.h>
#include <json/reader.h>
#include <json/writer.h>
#include <absl/strings/string_view.h>  // 使用 abseil

namespace jsonprotobuf {

bool Converter::MessageToJson(
    const google::protobuf::Message& message,
    Json::Value& json_output) 
{
    // 使用 protobuf 的 JSON 工具
    std::string json_string;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    
    auto status = google::protobuf::util::MessageToJsonString(
        message, &json_string, options);
    
    if (!status.ok()) {
        return false;
    }
    
    // 使用 jsoncpp 解析
    Json::CharReaderBuilder reader_builder;
    Json::CharReader* reader = reader_builder.newCharReader();
    
    std::string errors;
    bool ok = reader->parse(
        json_string.c_str(),
        json_string.c_str() + json_string.size(),
        &json_output,
        &errors);
    
    delete reader;
    return ok;
}

std::string Converter::MessageToJsonString(
    const google::protobuf::Message& message) 
{
    Json::Value json_value;
    if (!MessageToJson(message, json_value)) {
        return "";
    }
    
    Json::StreamWriterBuilder writer_builder;
    return Json::writeString(writer_builder, json_value);
}

bool Converter::JsonToMessage(
    const Json::Value& json_input,
    google::protobuf::Message* message) 
{
    // 将 jsoncpp 的 Value 转为字符串
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, json_input);
    
    // 使用 protobuf 工具解析
    google::protobuf::util::JsonParseOptions options;
    auto status = google::protobuf::util::JsonStringToMessage(
        json_string, message, options);
    
    return status.ok();
}

bool Converter::JsonStringToMessage(
    const std::string& json_string,
    google::protobuf::Message* message) 
{
    Json::CharReaderBuilder reader_builder;
    Json::CharReader* reader = reader_builder.newCharReader();
    
    Json::Value json_value;
    std::string errors;
    bool ok = reader->parse(
        json_string.c_str(),
        json_string.c_str() + json_string.size(),
        &json_value,
        &errors);
    
    delete reader;
    
    if (!ok) {
        return false;
    }
    
    return JsonToMessage(json_value, message);
}

} // namespace jsonprotobuf
```

### 构建步骤

```bash
# 1. 进入构建目录
cd libjsonprotobuf_build
mkdir build && cd build

# 2. 配置
cmake ..

# 你应该看到类似输出：
# -- Protobuf library: /path/to/third_party/protobuf/lib/libprotobuf.dylib
# -- Found 50 Abseil libraries
# -- JsonCpp library: /path/to/third_party/jsoncpp/lib/libjsoncpp.dylib

# 3. 编译
make -j4

# 4. 查看构建结果
otool -L libjsonprotobuf.dylib

# 应该看到类似：
# libjsonprotobuf.dylib:
#     @rpath/libjsonprotobuf.dylib
#     /path/to/third_party/protobuf/lib/libprotobuf.dylib
#     /path/to/third_party/abseil/lib/libabsl_*.dylib
#     /path/to/third_party/jsoncpp/lib/libjsoncpp.dylib

# 5. 安装到 third_party
make install

# 6. 验证安装
ls -la ../../third_party/jsonprotobuf/lib/
ls -la ../../third_party/jsonprotobuf/include/

# 7. 检查安装后的库
otool -L ../../third_party/jsonprotobuf/lib/libjsonprotobuf.dylib

# 应该看到：
# libjsonprotobuf.dylib:
#     @rpath/libjsonprotobuf.dylib
#     @loader_path/../protobuf/lib/libprotobuf.dylib  ← 相对路径
#     @loader_path/../abseil/lib/libabsl_*.dylib      ← 相对路径
#     @loader_path/../jsoncpp/lib/libjsoncpp.dylib    ← 相对路径
```

## 第二阶段：业务模块

```
