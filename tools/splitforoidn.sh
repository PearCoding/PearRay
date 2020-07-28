#!/bin/bash
# -*- coding: utf-8 -*-

# Splits given openexr file into three seperate files useable with OpenImageDenoise

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Get location of prsplit (useful if in development environment)
if command -v prsplit &>/dev/null; then
    SPLIT_CMD=prsplit
elif command -v "$THIS_DIR/../build/bin/prsplit" &>/dev/null; then
    SPLIT_CMD="$THIS_DIR/../build/bin/prsplit"
elif command -v "$THIS_DIR/../build/Release/bin/prsplit" &>/dev/null; then
    SPLIT_CMD="$THIS_DIR/../build/Release/bin/prsplit"
else
    echo "prsplit could not be found"
    exit
fi

if [ $# -eq 0 ]; then
    echo "No argument supplied"
    exit
fi

filename="$1"
name=$(basename "$filename" | cut -f 1 -d '.')

# Extract channels
channels=$($SPLIT_CMD -l "$filename")
albedo="[CS*DL]"
if ! echo "$channels" | grep -q --fixed-strings "${albedo}.R" ; then
    albedo=""
fi

normal="normal_geometric"
if ! echo "$channels" | grep -q --fixed-strings "${normal}.x" ; then
    normal="normal"
    if ! echo "$channels" | grep -q --fixed-strings "${normal}.x" ; then
        normal=""
    fi
fi

if [[ -n "$albedo" && -n "$normal" ]]; then
    has_ext=true
else
    has_ext=false
fi

# Split image
$SPLIT_CMD --channels R,G,B "$filename" "${name}_color.exr" 
if $has_ext; then
    if $SPLIT_CMD --channels "${albedo}.R","${albedo}.G","${albedo}.B" "$filename" "${name}_albedo.exr"; then
        $SPLIT_CMD --channels "${normal}.x","${normal}.y","${normal}.z" "$filename" "${name}_normal.exr" 
    else
        echo "No albedo data found. Skipping normal data"
    fi
fi