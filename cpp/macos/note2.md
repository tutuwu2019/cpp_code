# 优化2


```text
# ============================================
# 项目结构：
# MyProject/
# ├── CMakeLists.txt (这个文件)
# ├── third_party/
# │   ├── jsonprotobuf/
# │   │   ├── lib/
# │   │   │   └── libjsonprotobuf.dylib  ← 你的第三方库
# │   │   └── include/
# │   │       └── jsonprotobuf.h
# │   ├── protobuf/
# │   │   ├── lib/
# │   │   │   ├── libprotobuf.dylib
# │   │   │   └── libprotoc.dylib (可能有)
# │   │   └── include/
# │   │       └── google/protobuf/...
# │   ├── abseil/
# │   │   ├── lib/
# │   │   │   ├── libabsl_base.dylib
# │   │   │   ├── libabsl_strings.dylib
# │   │   │   └── ... (很多个 absl 库)
# │   │   └── include/
# │   │       └── absl/...
# │   └── jsoncpp/
# │       ├── lib/
# │       │   └── libjsoncpp.dylib
# │       └── include/
# │           └── json/json.h
# └── myapp/
#     ├── CMakeLists.txt
#     └── main.cpp
# ============================================

cmake_minimum_required(VERSION 3.15)
project(MyBusinessApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MACOSX_RPATH TRUE)

# ============================================
# 第一步：定义 third_party 路径
# ============================================

set(THIRD_PARTY_ROOT "${CMAKE_SOURCE_DIR}/third_party")

# 定义每个库的根目录
set(JSONPROTOBUF_ROOT "${THIRD_PARTY_ROOT}/jsonprotobuf")
set(PROTOBUF_ROOT "${THIRD_PARTY_ROOT}/protobuf")
set(ABSEIL_ROOT "${THIRD_PARTY_ROOT}/abseil")
set(JSONCPP_ROOT "${THIRD_PARTY_ROOT}/jsoncpp")

# 安装目录
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install prefix")

# ============================================
# 第二步：导入所有第三方库
# ============================================

# --- 1. JsonProtobuf (你的主要依赖库) ---
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

# 检查 jsonprotobuf 依赖了什么
execute_process(
    COMMAND otool -L "${JSONPROTOBUF_LIBRARY}"
    OUTPUT_VARIABLE JSONPROTOBUF_DEPS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "JsonProtobuf dependencies:\n${JSONPROTOBUF_DEPS}")

# --- 2. Protobuf ---
# 找到所有 protobuf 库
file(GLOB PROTOBUF_LIBRARIES "${PROTOBUF_ROOT}/lib/libproto*.dylib")

if(NOT PROTOBUF_LIBRARIES)
    message(FATAL_ERROR "Protobuf libraries not found in ${PROTOBUF_ROOT}/lib")
endif()

# 创建主 protobuf 目标（通常是 libprotobuf.dylib）
find_library(PROTOBUF_MAIN_LIBRARY
    NAMES protobuf libprotobuf
    PATHS "${PROTOBUF_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

add_library(protobuf SHARED IMPORTED)
set_target_properties(protobuf PROPERTIES
    IMPORTED_LOCATION "${PROTOBUF_MAIN_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PROTOBUF_ROOT}/include"
)

message(STATUS "Found Protobuf: ${PROTOBUF_MAIN_LIBRARY}")
message(STATUS "All Protobuf libraries: ${PROTOBUF_LIBRARIES}")

# --- 3. Abseil (很多个库) ---
file(GLOB ABSEIL_LIBRARIES "${ABSEIL_ROOT}/lib/libabsl_*.dylib")

if(NOT ABSEIL_LIBRARIES)
    message(WARNING "No Abseil libraries found in ${ABSEIL_ROOT}/lib")
    set(ABSEIL_LIBRARIES "")
else()
    message(STATUS "Found ${list(LENGTH ABSEIL_LIBRARIES)} Abseil libraries")
endif()

# 创建 INTERFACE 库来管理所有 absl
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
    
    # 链接到 abseil INTERFACE
    target_link_libraries(abseil INTERFACE ${absl_target})
endforeach()

# --- 4. JsonCpp ---
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

message(STATUS "Found JsonCpp: ${JSONCPP_LIBRARY}")

# ============================================
# 第三步：收集所有依赖库信息
# ============================================

# 所有需要的动态库文件
set(ALL_THIRD_PARTY_LIBS
    ${JSONPROTOBUF_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${ABSEIL_LIBRARIES}
    ${JSONCPP_LIBRARY}
)

# 去重
list(REMOVE_DUPLICATES ALL_THIRD_PARTY_LIBS)

# 提取所有库的目录（用于 BUILD_RPATH）
set(ALL_THIRD_PARTY_DIRS "")
foreach(lib ${ALL_THIRD_PARTY_LIBS})
    get_filename_component(lib_dir ${lib} DIRECTORY)
    list(APPEND ALL_THIRD_PARTY_DIRS ${lib_dir})
endforeach()
list(REMOVE_DUPLICATES ALL_THIRD_PARTY_DIRS)

message(STATUS "")
message(STATUS "=== Third Party Summary ===")
message(STATUS "Total libraries: ${list(LENGTH ALL_THIRD_PARTY_LIBS)}")
message(STATUS "Library directories: ${ALL_THIRD_PARTY_DIRS}")
message(STATUS "")

# ============================================
# 第四步：构建业务模块（你的可执行程序）
# ============================================

add_subdirectory(myapp)

# 或者直接在这里定义：
add_executable(myapp
    myapp/main.cpp
    # 添加你的其他业务代码
)

# 设置头文件搜索路径
target_include_directories(myapp PRIVATE
    ${JSONPROTOBUF_ROOT}/include
    ${PROTOBUF_ROOT}/include
    ${JSONCPP_ROOT}/include
    # ${ABSEIL_ROOT}/include  # 如果业务代码直接用 absl
)

# 链接库
target_link_libraries(myapp PRIVATE
    jsonprotobuf    # 主要依赖
    protobuf        # 如果业务代码直接使用 protobuf API
    jsoncpp         # 如果业务代码直接使用 jsoncpp API
    # abseil        # 通常不需要，除非业务代码直接用
)

# 配置 RPATH（关键！）
set_target_properties(myapp PROPERTIES
    # 开发期：指向 third_party 的各个库目录
    BUILD_RPATH "${ALL_THIRD_PARTY_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    
    # 部署期：所有库会被复制到 lib/ 目录
    INSTALL_RPATH "@executable_path/../lib"
    
    MACOSX_RPATH TRUE
)

# ============================================
# 第五步：安装配置
# ============================================

# 安装可执行程序
install(TARGETS myapp
    RUNTIME DESTINATION bin
)

# 安装所有第三方库
install(FILES ${ALL_THIRD_PARTY_LIBS}
    DESTINATION lib
)

# 可选：安装头文件（如果要作为 SDK 发布）
install(DIRECTORY 
    ${JSONPROTOBUF_ROOT}/include/
    ${PROTOBUF_ROOT}/include/
    ${JSONCPP_ROOT}/include/
    DESTINATION include
    FILES_MATCHING 
        PATTERN "*.h" 
        PATTERN "*.hpp"
        PATTERN "*.inc"
)

# ============================================
# 第六步：修复依赖路径（核心部分）
# ============================================

# 修复 myapp 的依赖路径
install(CODE "
    set(myapp_path \"\${CMAKE_INSTALL_PREFIX}/bin/myapp\")
    message(STATUS \"Fixing myapp dependencies...\")
    
    # 修复 jsonprotobuf
    execute_process(
        COMMAND install_name_tool 
            -change \"${JSONPROTOBUF_LIBRARY}\" 
            @executable_path/../lib/libjsonprotobuf.dylib
            \"\${myapp_path}\"
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # 修复 protobuf
    foreach(pb_lib ${PROTOBUF_LIBRARIES})
        get_filename_component(pb_name \${pb_lib} NAME)
        execute_process(
            COMMAND install_name_tool 
                -change \${pb_lib} 
                @executable_path/../lib/\${pb_name}
                \"\${myapp_path}\"
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 修复 jsoncpp
    execute_process(
        COMMAND install_name_tool 
            -change \"${JSONCPP_LIBRARY}\" 
            @executable_path/../lib/libjsoncpp.dylib
            \"\${myapp_path}\"
        OUTPUT_QUIET ERROR_QUIET
    )
    
    message(STATUS \"Fixed myapp dependencies\")
")

# 修复 libjsonprotobuf.dylib 的依赖路径
# （因为它依赖 protobuf、abseil、jsoncpp）
install(CODE "
    set(jsonprotobuf_path \"\${CMAKE_INSTALL_PREFIX}/lib/libjsonprotobuf.dylib\")
    message(STATUS \"Fixing libjsonprotobuf.dylib dependencies...\")
    
    # 修复 protobuf 依赖
    foreach(pb_lib ${PROTOBUF_LIBRARIES})
        get_filename_component(pb_name \${pb_lib} NAME)
        execute_process(
            COMMAND install_name_tool 
                -change \${pb_lib} 
                @loader_path/\${pb_name}
                \"\${jsonprotobuf_path}\"
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 修复 abseil 依赖
    foreach(absl_lib ${ABSEIL_LIBRARIES})
        get_filename_component(absl_name \${absl_lib} NAME)
        execute_process(
            COMMAND install_name_tool 
                -change \${absl_lib} 
                @loader_path/\${absl_name}
                \"\${jsonprotobuf_path}\"
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    # 修复 jsoncpp 依赖
    execute_process(
        COMMAND install_name_tool 
            -change \"${JSONCPP_LIBRARY}\" 
            @loader_path/libjsoncpp.dylib
            \"\${jsonprotobuf_path}\"
        OUTPUT_QUIET ERROR_QUIET
    )
    
    message(STATUS \"Fixed libjsonprotobuf.dylib dependencies\")
")

# 修复其他库的 install name（让它们使用 @rpath）
install(CODE "
    message(STATUS \"Fixing library install names...\")
    
    foreach(lib ${ALL_THIRD_PARTY_LIBS})
        get_filename_component(lib_name \${lib} NAME)
        set(installed_lib \"\${CMAKE_INSTALL_PREFIX}/lib/\${lib_name}\")
        
        # 修改库的 install name
        execute_process(
            COMMAND install_name_tool 
                -id @rpath/\${lib_name}
                \"\${installed_lib}\"
            OUTPUT_QUIET ERROR_QUIET
        )
    endforeach()
    
    message(STATUS \"Fixed all install names\")
")

# 高级：修复库之间的依赖（如果 protobuf 依赖 abseil）
install(CODE "
    message(STATUS \"Fixing inter-library dependencies...\")
    
    # 检查并修复 protobuf 对 abseil 的依赖
    foreach(pb_lib ${PROTOBUF_LIBRARIES})
        get_filename_component(pb_name \${pb_lib} NAME)
        set(installed_pb \"\${CMAKE_INSTALL_PREFIX}/lib/\${pb_name}\")
        
        foreach(absl_lib ${ABSEIL_LIBRARIES})
            get_filename_component(absl_name \${absl_lib} NAME)
            
            execute_process(
                COMMAND install_name_tool 
                    -change \${absl_lib} 
                    @loader_path/\${absl_name}
                    \"\${installed_pb}\"
                OUTPUT_QUIET ERROR_QUIET
            )
        endforeach()
    endforeach()
    
    message(STATUS \"Fixed inter-library dependencies\")
")

# ============================================
# 第七步：调试和验证工具
# ============================================

# 构建后自动检查
add_custom_command(TARGET myapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "myapp dependencies (BUILD)"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND otool -L $<TARGET_FILE:myapp>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "myapp RPATH:"
    COMMAND otool -l $<TARGET_FILE:myapp> | grep -A 3 LC_RPATH || echo "No RPATH"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Checking myapp configuration"
)

# 检查 jsonprotobuf 的依赖
add_custom_target(check-jsonprotobuf
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "libjsonprotobuf.dylib dependencies"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND otool -L "${JSONPROTOBUF_LIBRARY}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "This shows what libjsonprotobuf.dylib needs."
    COMMAND ${CMAKE_COMMAND} -E echo "Make sure these libraries are in third_party!"
    COMMENT "Checking libjsonprotobuf.dylib"
)

# 检查所有库
add_custom_target(check-all-libs
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "All Third Party Libraries"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "${ALL_THIRD_PARTY_LIBS}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Checking each library..."
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Listing all libraries"
)

# 测试运行
add_custom_target(test-run
    COMMAND ${CMAKE_COMMAND} -E echo "Testing myapp..."
    COMMAND $<TARGET_FILE:myapp> || ${CMAKE_COMMAND} -E echo "Failed to run"
    DEPENDS myapp
    COMMENT "Testing myapp execution"
)

# 验证安装后的配置
add_custom_target(verify-install
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Verifying installation..."
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Installed libraries:"
    COMMAND ls -lh "${CMAKE_INSTALL_PREFIX}/lib/" || true
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "myapp dependencies:"
    COMMAND otool -L "${CMAKE_INSTALL_PREFIX}/bin/myapp"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "libjsonprotobuf.dylib dependencies:"
    COMMAND otool -L "${CMAKE_INSTALL_PREFIX}/lib/libjsonprotobuf.dylib"
    COMMENT "Verifying installation"
)

# ============================================
# 打印配置摘要
# ============================================

message(STATUS "")
message(STATUS "╔════════════════════════════════════════════╗")
message(STATUS "║  Build Configuration Summary               ║")
message(STATUS "╠════════════════════════════════════════════╣")
message(STATUS "║ JsonProtobuf: ${JSONPROTOBUF_LIBRARY}")
message(STATUS "║ Protobuf:     ${list(LENGTH PROTOBUF_LIBRARIES)} libraries")
message(STATUS "║ Abseil:       ${list(LENGTH ABSEIL_LIBRARIES)} libraries")
message(STATUS "║ JsonCpp:      ${JSONCPP_LIBRARY}")
message(STATUS "║ ")
message(STATUS "║ Build RPATH:  ${ALL_THIRD_PARTY_DIRS}")
message(STATUS "║ Install path: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "╚════════════════════════════════════════════╝")
message(STATUS "")

# ============================================
# 使用说明
# ============================================

# 0. 确认目录结构：
#    third_party/
#    ├── jsonprotobuf/lib/libjsonprotobuf.dylib
#    ├── protobuf/lib/libprotobuf.dylib
#    ├── abseil/lib/libabsl_*.dylib
#    └── jsoncpp/lib/libjsoncpp.dylib

# 1. 检查 jsonprotobuf 依赖了什么：
#    otool -L third_party/jsonprotobuf/lib/libjsonprotobuf.dylib
#    确保依赖的库都在 third_party 中

# 2. 构建：
#    mkdir build && cd build
#    cmake ..
#    make -j4

# 3. 检查依赖（重要！）：
#    make check-jsonprotobuf
#    make check-all-libs

# 4. 测试运行（开发期）：
#    ./myapp/myapp
#    # 应该能运行，因为 BUILD_RPATH 指向了 third_party

# 5. 安装：
#    make install

# 6. 验证安装：
#    make verify-install
#    
#    或手动：
#    cd ../install
#    otool -L bin/myapp
#    # 应该看到 @executable_path/../lib/...
#    
#    otool -L lib/libjsonprotobuf.dylib
#    # 应该看到 @loader_path/...

# 7. 最终测试：
#    cd ../install
#    ./bin/myapp
#    # 应该完美运行！

# 8. 如果出错：
#    DYLD_PRINT_LIBRARIES=1 ./bin/myapp
#    # 查看实际加载了哪些库
#    
#    DYLD_PRINT_RPATHS=1 ./bin/myapp
#    # 查看 rpath 解析过程
```


> 常见问题

```text
// ============================================
// myapp/main.cpp - 业务模块示例
// ============================================

#include <iostream>
#include <string>

// JsonCpp 头文件
#include <json/json.h>

// Protobuf 头文件
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

// JsonProtobuf 头文件（假设接口）
#include <jsonprotobuf.h>  // 或者你的实际头文件路径

// 如果有自己定义的 proto 文件
// #include "myproto.pb.h"

int main(int argc, char* argv[]) {
    std::cout << "=== MyApp Started ===" << std::endl;
    
    // ----------------------------------------
    // 1. 测试 JsonCpp
    // ----------------------------------------
    std::cout << "\n[1] Testing JsonCpp..." << std::endl;
    
    Json::Value root;
    root["name"] = "MyApp";
    root["version"] = "1.0.0";
    root["features"] = Json::arrayValue;
    root["features"].append("json");
    root["features"].append("protobuf");
    
    Json::StreamWriterBuilder writer;
    std::string json_str = Json::writeString(writer, root);
    std::cout << "JSON: " << json_str << std::endl;
    
    // ----------------------------------------
    // 2. 测试 Protobuf
    // ----------------------------------------
    std::cout << "\n[2] Testing Protobuf..." << std::endl;
    
    // 假设你有一个 proto 定义
    // MyProto::Person person;
    // person.set_name("Alice");
    // person.set_age(30);
    // 
    // std::string proto_data;
    // person.SerializeToString(&proto_data);
    // std::cout << "Protobuf serialized: " << proto_data.size() << " bytes" << std::endl;
    
    // 或者只是测试 protobuf 基本功能
    std::cout << "Protobuf version: " 
              << GOOGLE_PROTOBUF_VERSION << std::endl;
    
    // ----------------------------------------
    // 3. 测试 JsonProtobuf 库（你的主要功能）
    // ----------------------------------------
    std::cout << "\n[3] Testing JsonProtobuf..." << std::endl;
    
    // 假设你的 jsonprotobuf 库提供了这样的接口：
    // JsonProtobuf::Converter converter;
    // 
    // // JSON -> Protobuf
    // std::string json = R"({"name": "Bob", "age": 25})";
    // auto proto = converter.JsonToProto(json);
    // 
    // // Protobuf -> JSON
    // std::string result_json = converter.ProtoToJson(proto);
    // std::cout << "Converted: " << result_json << std::endl;
    
    // 如果没有具体接口，至少测试能否链接
    std::cout << "JsonProtobuf library linked successfully!" << std::endl;
    
    // ----------------------------------------
    // 4. 业务逻辑示例
    // ----------------------------------------
    std::cout << "\n[4] Running business logic..." << std::endl;
    
    // 你的实际业务代码
    // ...
    
    std::cout << "\n=== MyApp Finished ===" << std::endl;
    return 0;
}

// ============================================
// 可能遇到的问题和解决方案
// ============================================

/*
问题1: 编译时找不到头文件
---------------------------------------
错误信息：
  fatal error: 'jsonprotobuf.h' file not found
  
解决方案：
  在 CMakeLists.txt 中添加：
  target_include_directories(myapp PRIVATE
      ${JSONPROTOBUF_ROOT}/include
  )

问题2: 链接时找不到符号
---------------------------------------
错误信息：
  Undefined symbols for architecture arm64:
    "_some_function", referenced from:
    
解决方案：
  确保 target_link_libraries 包含了需要的库：
  target_link_libraries(myapp PRIVATE
      jsonprotobuf
      protobuf
      jsoncpp
  )

问题3: 运行时找不到库
---------------------------------------
错误信息：
  dyld: Library not loaded: /path/to/libjsonprotobuf.dylib
  Reason: image not found
  
解决方案A - 开发期：
  检查 BUILD_RPATH 是否正确：
  otool -l myapp | grep -A 3 LC_RPATH
  
  应该包含 third_party 的路径
  
解决方案B - 部署期：
  1. 确保执行了 make install
  2. 检查 install/lib/ 下是否有所有库
  3. 检查路径：
     otool -L install/bin/myapp
     应该显示 @executable_path/../lib/...

问题4: jsonprotobuf 找不到它的依赖（protobuf/abseil）
---------------------------------------
错误信息：
  dyld: Library not loaded: /old/path/libprotobuf.dylib
  Referenced from: .../libjsonprotobuf.dylib
  
解决方案：
  这是因为 install 脚本中的 install_name_tool 没有正确执行
  
  手动修复：
  cd install/lib
  
  # 查看 jsonprotobuf 的依赖
  otool -L libjsonprotobuf.dylib
  
  # 手动修复每个依赖
  install_name_tool -change \
      /old/path/libprotobuf.dylib \
      @loader_path/libprotobuf.dylib \
      libjsonprotobuf.dylib

问题5: 多级依赖问题（protobuf 依赖 abseil）
---------------------------------------
错误信息：
  dyld: Library not loaded: /path/to/libabsl_base.dylib
  Referenced from: .../libprotobuf.dylib
  
解决方案：
  需要修复 protobuf 对 abseil 的引用
  
  cd install/lib
  
  # 检查 protobuf 依赖
  otool -L libprotobuf.dylib
  
  # 修复所有 abseil 引用
  for absl in libabsl_*.dylib; do
      install_name_tool -change \
          /old/path/$absl \
          @loader_path/$absl \
          libprotobuf.dylib
  done

问题6: 在其他机器上运行失败
---------------------------------------
原因：
  RPATH 中包含了绝对路径（如 BUILD_RPATH）
  
检查：
  otool -l install/bin/myapp | grep -A 3 LC_RPATH
  
  不应该有 /Users/xxx/... 这样的绝对路径
  
解决：
  确保 CMakeLists.txt 中：
  BUILD_WITH_INSTALL_RPATH FALSE
  INSTALL_RPATH "@executable_path/../lib"

问题7: 某些 abseil 库找不到
---------------------------------------
原因：
  可能有些 abseil 库在 third_party 中缺失
  
诊断：
  1. 检查 jsonprotobuf 需要哪些 absl:
     otool -L third_party/jsonprotobuf/lib/libjsonprotobuf.dylib | grep absl
     
  2. 检查 protobuf 需要哪些 absl:
     otool -L third_party/protobuf/lib/libprotobuf.dylib | grep absl
     
  3. 确保 third_party/abseil/lib/ 中有所有需要的库
  
解决：
  补充缺失的 abseil 库到 third_party/abseil/lib/

问题8: JsonCpp 的头文件路径问题
---------------------------------------
不同版本的 JsonCpp 头文件路径可能不同：
  
  版本1: #include <json/json.h>
  版本2: #include <jsoncpp/json/json.h>
  
检查：
  ls third_party/jsoncpp/include/
  
调整代码中的 #include 路径
*/

// ============================================
// 调试技巧
//
```
