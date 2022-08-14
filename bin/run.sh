#!/bin/zsh
set -eu

BD="$(pwd)/$(dirname $0)/.."
source "${BD}/bin/variables.sh"

OPT_LEVEL=0
while getopts o: flag; do
    case "${flag}" in
    o) OPT_LEVEL=${OPTARG} ;;
    esac
done

# Accept case-insensitive mode by converting to uppercase
MODE=${@:$OPTIND:1}
MODE=${MODE:u}

if [ "${MODE}" = "DEBUG" ]; then
    OPT_LEVEL=0 # force no optimization in debug mode
fi

function analyze_mem() {
    echo "Memory report analysis started."
    echo
    if [ $(ls /tmp/pointers | wc -l) -ne 0 ]; then
        echo "\e[33mFAIL:\e[0m Memory leak detected."
        for f in $(ls /tmp/pointers); do
            echo "$f - $(cat /tmp/pointers/$f)"
        done
    else
        echo "\e[32mSUCCESS:\e[0m No memory leak detected."
    fi
    set -e
    echo
    echo "Memory report analysis completed."
}

pushd "${BD}"
echo "Closing running instance"
set +e
echo "${BD}/test/artifacts"
/bin/rm -rf "${BD}/test/artifacts"
/bin/rm -rf /tmp/pointers/*
/bin/rm -rf ${ARTIFACT_FOLDER}

PID=$(pgrep ${APP_NAME})
set -e
if ! [ "${PID}" = "" ]; then
    echo $PID
    kill ${PID}
else
    echo "No process was running."
fi

if ! [ -f "${MAKE_FILE}" ]; then
    echo "No makefile found."
    echo "Calling bin/makeMakefile.sh"
    ./bin/makeMakefile.sh
fi

echo "Running"
if [ "${MODE}" = "TEST" ] || [ "${MODE}" = "DEBUG" ]; then
    mkdir -p /tmp/pointers
    # Set up dir entries for testing.
    mkdir -p "${ARTIFACT_FOLDER}/empty/" \
        "${ARTIFACT_FOLDER}/non-empty/inner/inner_l2" \
        "${ARTIFACT_FOLDER}/non-empty-0/inner/inner_l2" \
        "${ARTIFACT_FOLDER}/empty-0"

    touch "${ARTIFACT_FOLDER}/non-empty/inner/file.txt"
    touch "${ARTIFACT_FOLDER}/non-empty/inner/inner_l2/file.txt"
    touch "${ARTIFACT_FOLDER}/delete_me.txt"
    touch "${ARTIFACT_FOLDER}/delete_me_2.txt"

    make MODE=TEST OPT=${OPT_LEVEL} 2>&1
    if [ "${MODE}" = "TEST" ]; then
        # Remove previous logs.
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
        analyze_mem
        echo
    else
        lldb ./"${BUILD_DIR}/${APP_NAME}"
    fi
else
    echo "Error: accpepted modes are 'test' or 'debug'"
    exit 1
fi
popd
