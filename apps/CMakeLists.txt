if(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "Android")
    add_subdirectory(exercices)
    add_subdirectory(app-Demo-Node)
    add_subdirectory(app-Demo-WAI)
    add_subdirectory(app-Demo-SLProject)
	add_subdirectory(app-CameraCalibration)
	add_subdirectory(app-Test-WAI)
else()
    if("${SL_APP}" MATCHES "SL_APP_WAI")
        add_subdirectory(app-Demo-WAI)
    else()
        add_subdirectory(app-Demo-SLProject)
    endif()
endif()
