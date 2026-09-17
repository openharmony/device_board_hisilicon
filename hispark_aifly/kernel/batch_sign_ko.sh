#!/bin/bash
# Copyright (c) 2026 HiSilicon (Shanghai) Technologies Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

# Configuration
SIGN_ALG="sha256"

# Logging functions
error_exit() {
    echo -e "\033[31m[ERROR] $1\033[0m" >&2
    exit 1
}

info_log() {
    echo -e "\033[32m[INFO] $1\033[0m"
}

# Check if .ko is already signed
is_signed() {
    local file="$1"
    if tail -c 20 "$file" 2>/dev/null | grep -q "~Module signature appended~"; then
        return 0
    fi
    return 1
}

# Auto-detect OHOS_OUTDIR if not set
if [ -z "${OHOS_OUTDIR:-}" ]; then
    OHOS_OUTDIR=$(find "$(pwd)" -maxdepth 4 -type d -name "out" 2>/dev/null | head -1)
    if [ -z "$OHOS_OUTDIR" ]; then
        error_exit "OHOS_OUTDIR not set and could not be auto-detected."
    fi
    info_log "Auto-detected OHOS_OUTDIR: $OHOS_OUTDIR"
fi

# Auto-detect KERNEL_VERSION if not set
if [ -z "${KERNEL_VERSION:-}" ]; then
    KERNEL_VERSION=$(ls -d "${OHOS_OUTDIR}/kernel/"* 2>/dev/null | xargs -n1 basename | head -1)
    if [ -z "$KERNEL_VERSION" ]; then
        error_exit "KERNEL_VERSION not set and could not be auto-detected."
    fi
    info_log "Auto-detected KERNEL_VERSION: $KERNEL_VERSION"
fi

# Resolve signing tool and key paths
SIGN_TOOL="${OHOS_OUTDIR}/kernel/${KERNEL_VERSION}/scripts/sign-file"
PRIVATE_KEY="${OHOS_OUTDIR}/kernel/${KERNEL_VERSION}/certs/signing_key.pem"
PUBLIC_KEY="${OHOS_OUTDIR}/kernel/${KERNEL_VERSION}/certs/signing_key.x509"

# Validate signing tools and keys
[ -x "$SIGN_TOOL" ] || error_exit "Signing tool not found or not executable: $SIGN_TOOL"
[ -f "$PRIVATE_KEY" ] || error_exit "Private key not found: $PRIVATE_KEY"
[ -f "$PUBLIC_KEY" ] || error_exit "Public key not found: $PUBLIC_KEY"

# Parse arguments: -o <output_dir> <input_path1> [input_path2] ...
OUTPUT_DIR=""
INPUT_PATHS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -o)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        *)
            INPUT_PATHS+=("$1")
            shift
            ;;
    esac
done

if [ ${#INPUT_PATHS[@]} -eq 0 ]; then
    error_exit "Usage: $0 -o <output_dir> <input_ko_path1> [input_ko_path2] ..."
fi

if [ -z "$OUTPUT_DIR" ]; then
    error_exit "Output directory (-o) is required."
fi

mkdir -p "$OUTPUT_DIR"
info_log "Output directory: $OUTPUT_DIR"

# Collect input KO files with their source directories
KO_FILES=()
KO_SOURCE_DIRS=()
for INPUT in "${INPUT_PATHS[@]}"; do
    if [ -d "$INPUT" ]; then
        mapfile -d '' DIR_KOS < <(find "$INPUT" -type f -name "*.ko" -print0)
        for ko_path in "${DIR_KOS[@]}"; do
            KO_FILES+=("$ko_path")
            KO_SOURCE_DIRS+=("$INPUT")
        done
    elif [ -f "$INPUT" ]; then
        KO_FILES+=("$INPUT")
        KO_SOURCE_DIRS+=("")
    else
        error_exit "Input path not found: $INPUT"
    fi
done

if [ ${#KO_FILES[@]} -eq 0 ]; then
    info_log "No .ko files found in provided input paths"
    exit 0
fi

TOTAL=${#KO_FILES[@]}
SIGNED=0
FAILED=0

info_log "Found $TOTAL .ko file(s) to sign"
info_log "Starting signing process..."

for i in "${!KO_FILES[@]}"; do
    KO="${KO_FILES[$i]}"
    SOURCE_DIR="${KO_SOURCE_DIRS[$i]}"

    # Preserve relative directory structure for directory inputs
    if [ -n "$SOURCE_DIR" ]; then
        REL_PATH="${KO#${SOURCE_DIR}/}"
        KO_OUT="$OUTPUT_DIR/$REL_PATH"
    else
        KO_OUT="$OUTPUT_DIR/$(basename "$KO")"
    fi

    # Skip cp if source and output are the same file
    if [ "$(realpath "$KO" 2>/dev/null)" != "$(realpath "$KO_OUT" 2>/dev/null)" ]; then
        mkdir -p "$(dirname "$KO_OUT")"
        cp -f "$KO" "$KO_OUT"
    fi

    # Skip if already signed
    if is_signed "$KO_OUT"; then
        info_log "[$((++SIGNED))/$TOTAL] Already signed: $(basename "$KO_OUT")"
        continue
    fi

    if "$SIGN_TOOL" "$SIGN_ALG" "$PRIVATE_KEY" "$PUBLIC_KEY" "$KO_OUT" >/dev/null 2>&1; then
        info_log "[$((++SIGNED))/$TOTAL] Signed: $(basename "$KO_OUT")"
    else
        echo -e "\033[31m[FAIL] $(basename "$KO_OUT")\033[0m" >&2
        ((FAILED++))
    fi
done

if [ $FAILED -gt 0 ]; then
    error_exit "Signing completed with $FAILED error(s). $SIGNED/$TOTAL succeeded."
fi

info_log "All $TOTAL .ko file(s) signed successfully!"
exit 0
