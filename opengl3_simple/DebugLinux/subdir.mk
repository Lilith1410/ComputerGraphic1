################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Model.cpp \
../OpenGL3Viewer.cpp \
../RGBCubeModel.cpp \
../TetraederModel.cpp \
../ZahnRadModel.cpp \
../main.cpp \
../opengl_utils.cpp 

OBJS += \
./Model.o \
./OpenGL3Viewer.o \
./RGBCubeModel.o \
./TetraederModel.o \
./ZahnRadModel.o \
./main.o \
./opengl_utils.o 

CPP_DEPS += \
./Model.d \
./OpenGL3Viewer.d \
./RGBCubeModel.d \
./TetraederModel.d \
./ZahnRadModel.d \
./main.d \
./opengl_utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


