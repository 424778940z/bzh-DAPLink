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
    # "${TINYUSB_DIR}/src/portable/wch/*.c" # wch port not compatible with latest wch hal
    "tinyusb_port/wch/*.c"
    "tinyusb_port/*.c"
)
# Exclude host-side class drivers to save flash when only device stack is used
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/cdc/.+host\\.c$")
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/hid/.+host.*\\.c$")
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/msc/.*\\.c$")
list(FILTER TINYUSB_SRC EXCLUDE REGEX "class/vendor/.+host\\.c$")
list(FILTER TINYUSB_SRC EXCLUDE REGEX "tinyusb_port/wch/hcd_.*\\.c$")
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
    # "${TINYUSB_DIR}/src/portable/wch/*.h" # wch port not compatible with latest wch hal
    "tinyusb_port/wch/*.h"
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
)
