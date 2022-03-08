CURRENT_FILE="$0"
UPDATE_STARTER="updt"

echo "Currently running ${CURRENT_FILE}"

newExe=$(find -regextype posix-extended -regex ".*v[0-9]{1,}\\.[0-9]{1,}\\.[0-9]{1,}\\.update$")
newExe="$(basename -- $newExe)"

oldExe=$(find -regextype posix-extended -regex ".*v[0-9]{1,}\\.[0-9]{1,}\\.[0-9]{1,}\\.exe$")
oldExe="$(basename -- $oldExe)"

echo "$newExe"
echo "${oldExe}"
find -regextype posix-extended -regex ".*v[0-9]{1,}\\.[0-9]{1,}\\.[0-9]{1,}\\.exe$"

rm "${oldExe}"

mv "${newExe}" "${oldExe}"

./${oldExe}

rm "${UPDATE_STARTER}"
rm "${CURRENT_FILE}"