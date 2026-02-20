#!/usr/bin/env zsh
set -eu
setopt +o nomatch
BD="$(pwd)/$(dirname $0)/.."
APP_NAME="mylibc"
ARTIFACT_FOLDER="test/artifacts"
LOG_FILE_ERR="${ARTIFACT_FOLDER}/err.log"
FLAGS="-Wall -Werror -Wextra -std=c2x -pedantic -fsanitize=address"
DEBUG_FLAGS="-O0 -g -D_TEST -D_MEMORY_CHECK"
RELEASE_FLAGS="-O3"
BUILD_DIR="build"
DIST_DIR="dist"
DEBUGGER="lldb"
POINTER_DIR="/tmp/pointers"

if [ "$(uname -s)" = "Linux" ]; then
    FLAGS="${FLAGS} -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_GNU_SOURCE"
    DEBUGGER="gdb"
fi

# Accept case-insensitive mode by converting to uppercase
MODE=${@:$OPTIND:1}
MODE=${MODE:u}

function f_analyze_mem() {
    echo "Memory report analysis started."
    echo
    if [ $(ls "${POINTER_DIR}" | wc -l) -ne 0 ]; then
        echo "\e[33mFAIL:\e[0m Memory leak detected."
        for f in $(ls "${POINTER_DIR}"); do
            echo "$f - $(cat ${POINTER_DIR}/$f)"
        done
    else
        echo "\e[32mSUCCESS:\e[0m No memory leak detected."
    fi
    set -e
    echo
    echo "Memory report analysis completed."
}

pushd "${BD}" >/dev/null
echo "Closing running instance"
set +e
rm -rf "${BUILD_DIR}"
rm -rf "${POINTER_DIR}"
rm -rf "${ARTIFACT_FOLDER}"
rm -rf "${DIST_DIR}"

PID=$(pgrep ${APP_NAME})
set -e
if ! [ "${PID}" = "" ]; then
    echo $PID
    kill ${PID}
else
    echo "No process was running."
fi

mkdir -p "${BUILD_DIR}"

if [ "${MODE}" = "TEST" ] || [ "${MODE}" = "DEBUG" ] || [ "${MODE}" = "MODULE" ]; then
    if [ "${MODE}" = "MODULE" ]; then
        clang -c src/main-test.c $(echo ${FLAGS} ${DEBUG_FLAGS}) -D_MODULE -o ${BUILD_DIR}/${APP_NAME}.o
    else
        clang src/main-test.c $(echo ${FLAGS} ${DEBUG_FLAGS}) -o ${BUILD_DIR}/${APP_NAME}
    fi
    mkdir -p "${POINTER_DIR}"
    # Set up dir entries for testing.
    mkdir -p "${ARTIFACT_FOLDER}/empty/" \
        "${ARTIFACT_FOLDER}/non-empty/inner/inner_l2" \
        "${ARTIFACT_FOLDER}/non-empty-0/inner/inner_l2" \
        "${ARTIFACT_FOLDER}/empty-0"

    touch "${ARTIFACT_FOLDER}/non-empty/inner/file.txt"
    touch "${ARTIFACT_FOLDER}/non-empty/inner/inner_l2/file.txt"
    touch "${ARTIFACT_FOLDER}/delete_me.txt"
    touch "${ARTIFACT_FOLDER}/delete_me_2.txt"

    if [ "${MODE}" = "TEST" ]; then
        ./"${BUILD_DIR}/${APP_NAME}" 2>"${LOG_FILE_ERR}"
        RET_VAL=$?
        echo "================================================================================"
        if [ ${RET_VAL} -ne 0 ]; then
            echo -e "\n\n\e[31mFAIL:\e[0m Execution interrupted with error code ${RET_VAL}.\n\n"
            exit ${RET_VAL}
        fi
        if [ -f "${LOG_FILE_ERR}" ]; then
            if [ "$(cat ${LOG_FILE_ERR})" = "" ]; then
                echo -e "\n\n\e[32mSUCCESS:\e[0m All tests passed.\n\n"
            else
                echo -e "\n\n\e[31mFAIL:\e[0m The content of ${LOG_FILE_ERR} follows.\n\n"
                cat "${LOG_FILE_ERR}"
            fi
        else
            echo -e "\n\n\e[31mApplication not run.\e[0m\n\n"
        fi
        echo "================================================================================"
        f_analyze_mem
        echo
    elif [ "${MODE}" = "DEBUG" ]; then
        ${DEBUGGER} ./"${BUILD_DIR}/${APP_NAME}"
    else
        mkdir ${DIST_DIR}
        cp src/mylibc.h "${BUILD_DIR}/${APP_NAME}.o" "${DIST_DIR}"
        echo "----------------------------------------"
        echo "------- Compiled in MODULE mode --------"
        echo "----------------------------------------"
    fi
elif [ "${MODE}" = "LIB" ]; then
    mkdir ${DIST_DIR}
    clang -c src/main-test.c $(echo ${FLAGS} ${RELEASE_FLAGS}) -o "${BUILD_DIR}/${APP_NAME}.o"
    ar rcs "${BUILD_DIR}/lib${APP_NAME}.a" "${BUILD_DIR}/${APP_NAME}.o"
    cp src/mylibc.h "${BUILD_DIR}/lib${APP_NAME}.a" "${DIST_DIR}"
    echo "----- Dist folder -----"
    ls -1 "${DIST_DIR}"
    echo "-----    Usage    -----"
    echo "- Add ${APP_NAME}.h to your projects."
    echo "- Compile your projects using \n\t\$ clang <your-translation-units> -L<path-to-lib${APP_NAME}> -l${APP_NAME} -o <my_program>"
elif [ "${MODE}" = "RELEASE" ]; then
    rm -rf ${DIST_DIR}
    mkdir ${DIST_DIR}
    clang -c src/main-test.c $(echo ${FLAGS} ${RELEASE_FLAGS}) -o "${BUILD_DIR}/${APP_NAME}.o"
    cp src/mylibc.h "${BUILD_DIR}/${APP_NAME}.o" "${DIST_DIR}"
    echo "----- Dist folder -----"
    ls -1 "${DIST_DIR}"
    echo "-----    Usage    -----"
    echo "- Add ${APP_NAME}.h to your projects."
    echo "- Compile your projects using \n\t\$ clang <your-translation-units> <path-to-'${APP_NAME}.o'> -o <my_program>"
else
    echo "Error: accepted modes are:\n\t- 'test'\n\t- 'debug'\n\t- 'release'\n\t- lib"
    exit 1
fi
popd
