cmake_minimum_required(VERSION 3.2)
project(ExecHeur)

# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
	main_heur_sbrp.cpp
	AlnsSBRP.cpp
	BestSolutionList.cpp
	Parameters.cpp
	LoadSBRP.cpp
	CostFunctionSBRP.cpp
	SequentialInsertionSBRP.cpp
	InsRmvMethodSBRP.cpp
	RouteFeasibility.cpp
	RecourseLowerBound.cpp
	RegretInsertionSBRP.cpp
	RandomRemoveSBRP.cpp
	RelatednessRemoveSBRP.cpp
)

# Target: exec_heur
add_executable(exec_heur ${SOURCES})

# Usage message when typing make in the build directory
add_custom_target(usage
    COMMAND ${CMAKE_COMMAND} -E echo "Call: ./exec_heur to see how to use the executable"
)

