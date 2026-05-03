set(TINYUSB_DIR "${TOP_DIR}/modules/tinyusb")

set(MODULE_OBJ usb_stack)

# source and headers
file(
    GLOB TINYUSB_SRC
    RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
    "${TINYUSB_DIR}/src/*.c"
    "${TINYUSB_DIR}/src/common/*.c"
    "${TINYUSB_DIR}/src/device/*.c"
    "${TINYUSB_DIR}/src/class/cdc/*.c"
    "${TINYUSB_DIR}/src/class/hid/*.c"
    # STM32F411 OTG_FS uses the synopsys/dwc2 DCD. The dcd_dwc2.c source pulls
    # in dwc2_stm32.h via tusb_config.h's CFG_TUSB_MCU=OPT_MCU_STM32F4 selector.
    "${TINYUSB_DIR}/src/portable/synopsys/dwc2/dcd_dwc2.c"
    "${TINYUSB_DIR}/src/portable/synopsys/dwc2/dwc2_common.c"
    "tinyusb_port/*.c"
)
# Exclude host-side class drivers to save flash when only device stack is used
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/cdc/.+host\\.c$")
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/hid/.+host.*\\.c$")
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/msc/.*\\.c$")
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/vendor/.+host\\.c$")
# Add only the needed device classes
list(APPEND TINYUSB_SRC "../../modules/tinyusb/src/class/vendor/vendor_device.c")
file(
    GLOB TINYUSB_INCLUDES_RAW
    RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
    "${TINYUSB_DIR}/src/*.h"
    "${TINYUSB_DIR}/src/common/*.h"
    "${TINYUSB_DIR}/src/device/*.h"
    "${TINYUSB_DIR}/src/class/cdc/*.h"
    "${TINYUSB_DIR}/src/class/hid/*.h"
    "${TINYUSB_DIR}/src/class/vendor/*.h"
    "${TINYUSB_DIR}/src/portable/synopsys/dwc2/*.h"
    "tinyusb_port/*.h"
)
# file(
#     GLOB_RECURSE TINYUSB_INCLUDES_RAW
#     RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
#     "${TINYUSB_DIR}/src/*.h"
# )
unset(TINYUSB_INCLUDES_RAW_tmp)
foreach(item ${TINYUSB_INCLUDES_RAW})
    get_filename_component(dir ${item} DIRECTORY)
    list(APPEND TINYUSB_INCLUDES_RAW_tmp ${dir})
endforeach()
set(TINYUSB_INCLUDES_RAW ${TINYUSB_INCLUDES_RAW_tmp})
unset(TINYUSB_INCLUDES_RAW_tmp)
# deduplicate subdirs
list(REMOVE_DUPLICATES TINYUSB_INCLUDES_RAW)
# sort
list(SORT TINYUSB_INCLUDES_RAW)
# copy over
set(TINYUSB_INCLUDES ${TINYUSB_INCLUDES_RAW})
unset(TINYUSB_INCLUDES_RAW)

# debug print
foreach(item ${TINYUSB_SRC})
    message(STATUS "TINYUSB SRC FILE: ${item}")
endforeach()
foreach(item ${TINYUSB_INCLUDES})
    message(STATUS "TINYUSB INCLUDE DIR: ${item}")
endforeach()

# object
add_library(
    ${MODULE_OBJ}
    OBJECT

    # Source files
    ${TINYUSB_SRC}
    usb.c
)
target_include_directories(
    ${MODULE_OBJ} PUBLIC
    ${TINYUSB_INCLUDES}
    ".."
    "../../modules/cmsis_dap/Firmware/Include"
    "tinyusb_port"
)

# compile options
target_compile_definitions(
    ${MODULE_OBJ} PUBLIC
    CFG_TUSB_CONFIG_FILE="tusb_config.h"
)
target_compile_options(
    ${MODULE_OBJ} PRIVATE
    -Wno-error=format-nonliteral
    -Wno-error=redundant-decls
)
# Propagate HAL/CMSIS include paths and STM32F411xE define so dwc2_stm32.h can
# pull in <stm32f4xx.h> / <stm32f411xe.h> from the STM32CubeF4 submodule.
target_link_libraries(${MODULE_OBJ} PUBLIC stm32_hal)
