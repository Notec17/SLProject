#
# CMake configuration for app-Demo-WAI application
#

set(target app-Demo-WAI)

set(compile_definitions
    SL_STARTSCENE=SID_Minimal)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB headers
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AverageTiming.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGui.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGuiInfosDialog.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGuiMenu.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGuiPrefs.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiAbout.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiError.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosFrameworks.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosMapNodeTransform.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosScene.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosSensors.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosTracking.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiProperties.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiSceneGraph.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiSlamLoad.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiStatsDebugTiming.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiStatsTiming.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiStatsVideo.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTrackedMapping.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTransform.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiUIPrefs.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiVideoControls.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiVideoStorage.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTestOpen.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTestWrite.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiSlamParam.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AppWAI.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AppDirectories.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AppWAIScene.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AppWAISlamParamHelper.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/WAICalibration.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/WAIMapStorage.h
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/WAIUtils.h
    )

file(GLOB sources
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AverageTiming.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGui.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGuiInfosDialog.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGuiMenu.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/AppDemoGuiPrefs.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiAbout.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiError.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosFrameworks.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosMapNodeTransform.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosScene.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosSensors.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiInfosTracking.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiProperties.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiSceneGraph.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiSlamLoad.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiStatsDebugTiming.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiStatsTiming.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiStatsVideo.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTrackedMapping.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTransform.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiUIPrefs.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiVideoControls.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiVideoStorage.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTestOpen.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiTestWrite.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/AppDemoGuiSlamParam.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AppWAI.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/AppWAIScene.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/WAICalibration.cpp
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/WAIMapStorage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AppDemoMainGLFW.cpp
    )

# Group source files
set(headers_sources ${sources} ${headers})
source_group_by_path("${CMAKE_CURRENT_SOURCE_DIR}" "\\\\.h$|\\\\.hpp$" "" ${headers_sources})
#source_group_by_path("${CMAKE_CURRENT_SOURCE_DIR}" "\\\\.h$|\\\\.hpp$" "Header Files" ${headers})
#source_group_by_path("${CMAKE_CURRENT_SOURCE_DIR}" "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$" "Source Files" ${sources})

add_executable(${target}
    ${headers}
    ${sources}
    )

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "apps"
    )

target_include_directories(${target}
    PRIVATE
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/Gui/
    ${SL_PROJECT_ROOT}/apps/app-Demo-WAI/source/GuiPrefab/
    PUBLIC

    INTERFACE
    )

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${META_PROJECT_NAME}::lib-WAI
    lib-SLProject

    INTERFACE
    )

target_compile_definitions(${target}
    PRIVATE
    ${compile_definitions}

    PUBLIC
    ${DEFAULT_COMPILE_DEFINITIONS}
    WAI_DLL
    WAI_DLL_IMPORTS

    INTERFACE
    )

target_compile_options(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
    )

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}

    INTERFACE
    )
