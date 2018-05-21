#################
# LinkZED.cmake #
#################

IF(WITH_ZED)
	TARGET_LINK_LIBRARIES(${targetname} ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY} ${CUDA_NPP_LIBRARIES_ZED})
ENDIF()
