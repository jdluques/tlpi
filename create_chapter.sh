#!/usr/bin/env bash

set -e

if [ $# -ne 1 ]; then
  echo "Usage: $0 <chapter_directory>"
  echo "Example: $0 04_file_io"
  exit 1
fi

CHAPTER="$1"

if [ -d "$CHAPTER" ]; then
  echo "Directory '$CHAPTER' already exists"
  exit 1
fi

mkdir -p "$CHAPTER"/{bin,generated}

echo "Created:"
echo " $CHAPTER/"
echo "  $CHAPTER/bin/"
echo "  $CHAPTER/generated/"
