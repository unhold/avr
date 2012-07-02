# !/bin/sh
# Call this file from Eclipse scanner profile instead of the compiler directly. It will pick up the defines set in the make file
# which makes the indexing work almost flawlessly.
# The code highlighting with #ifdef sections will also be correct.

export SCANNER_INFO_EXPORT_FILE="$1 $2 $3 $4 $5 $6 $7 $8"
make scanner-info
