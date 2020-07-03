# This file can be used to make your life easier on Linux
# Use "source project.sh" to establish a environment to work with PearRay

[[ "${BASH_SOURCE[0]}" == "${0}" ]] && { echo >&2 "Script project.sh has to be sourced. Use 'source ${BASH_SOURCE[0]}' instead. Aborting"; exit 1; }

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Make sure we search for special types of build types
if [ -d "${THIS_DIR}/build/Release" ]; then
    WRK_DIR="${THIS_DIR}/build/Release"
elif [ -d "${THIS_DIR}/build/Debug" ]; then
    WRK_DIR="${THIS_DIR}/build/Debug"
elif [ -d "${THIS_DIR}/build" ]; then
    WRK_DIR="${THIS_DIR}/build"
else
    WRK_DIR="${THIS_DIR}"
fi

export PATH="${WRK_DIR}/bin:${PATH:-}"
export LD_LIBRARY_PATH="${WRK_DIR}/bin:${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${WRK_DIR}/bin:${PYTHONPATH:-}"
