################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../extern/glm/glm/detail/glm.cpp 

OBJS += \
./extern/glm/glm/detail/glm.o 

CPP_DEPS += \
./extern/glm/glm/detail/glm.d 


# Each subdirectory must supply rules for building sources it contributes
extern/glm/glm/detail/%.o: ../extern/glm/glm/detail/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


