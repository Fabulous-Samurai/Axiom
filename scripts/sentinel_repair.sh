#!/bin/bash
# scripts/sentinel_repair.sh

repair_cmake_missing_source() {
    local MISSING_FILE=$1
    local CMAKE_FILE="CMakeLists.txt"

    echo "REPAIR: Adding $MISSING_FILE to CMakeLists.txt"

    # Yedekle
    cp "$CMAKE_FILE" "${CMAKE_FILE}.sentinel_backup"

    # Dosyayı target_sources'a ekle
    # Son .cpp satırından sonra ekle
    sed -i "/\.cpp$/a\\    $MISSING_FILE" "$CMAKE_FILE"

    echo "APPLIED: Added $MISSING_FILE to build"
}

repair_missing_include() {
    local SOURCE_FILE=$1
    local MISSING_HEADER=$2

    echo "REPAIR: Adding #include for $MISSING_HEADER in $SOURCE_FILE"

    cp "$SOURCE_FILE" "${SOURCE_FILE}.sentinel_backup"

    # İlk #include satırından ÖNCE ekle
    sed -i "0,/#include/s//#include <$MISSING_HEADER>\n&/" "$SOURCE_FILE"

    echo "APPLIED: Added #include <$MISSING_HEADER>"
}

repair_pythonpath() {
    echo "REPAIR: Setting PYTHONPATH"
    export PYTHONPATH="${PYTHONPATH:+$PYTHONPATH:}$(pwd)/build"
    echo "APPLIED: PYTHONPATH=$PYTHONPATH"
}

repair_pip_module() {
    local MODULE=$1
    echo "REPAIR: Installing Python module $MODULE"
    pip install "$MODULE" 2>&1
    echo "APPLIED: pip install $MODULE"
}

repair_permission() {
    local FILE=$1
    echo "REPAIR: Setting executable permission on $FILE"
    chmod +x "$FILE"
    echo "APPLIED: chmod +x $FILE"
}
