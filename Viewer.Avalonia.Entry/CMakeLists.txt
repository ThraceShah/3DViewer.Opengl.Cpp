project(Viewer.Avalonia.Entry)

 # 设置C#项目路径
 set(CS_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Viewer.Avalonia.Entry.csproj")

 # 添加一个自定义目标来构建C#项目
 if(Debug)
    add_custom_target(BuildCSharpProject ALL
    COMMAND dotnet build ${CS_PROJECT_PATH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building C# project using dotnet"
    DEPENDS vgo )
else()
    add_custom_target(BuildCSharpProject ALL
    COMMAND dotnet build ${CS_PROJECT_PATH} -c Release
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building C# project using dotnet"
    DEPENDS vgo )
endif()

