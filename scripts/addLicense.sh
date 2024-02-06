##
# Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

COPYRIGHT_C="\
/*
 * Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
"

COPYRIGHT_UNI="\
//
// Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
//
// SPDX-License-Identifier: MIT
//
"

COPYRIGHT_META="\
##
# Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##
"

if [ ! -z "$1" ]; then
  DIR="$(readlink -f "$1")"
else
  DIR="$(dirname $(dirname "$(readlink -f "$0")"))"
fi

function insertCopyright {
  FILENAME=${1}
  COPYRIGHT=${2}
  grep "Copyright" "${FILENAME}" > /dev/null
  if [ $? -eq 1 ]; then
    echo "${COPYRIGHT}" | cat - "${FILENAME}" > /tmp/out && mv /tmp/out "${FILENAME}"; 
  fi
}

for i in $(find ${DIR} -type f -name "*.c"); 
do
  insertCopyright "${i}" "${COPYRIGHT_C}"
done

for i in $(find ${DIR} -type f -name "*.uni"); 
do
  insertCopyright "${i}" "${COPYRIGHT_UNI}"
done

for i in $(find ${DIR} -type f -name "*.dsc" -o -name "*.dec" -o -name "*.inf"); 
do
  insertCopyright "${i}" "${COPYRIGHT_META}"
done

