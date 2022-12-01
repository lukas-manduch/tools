#!/bin/bash
# This script was used for downloading some data from The Guardian webpage.
# I have it here only for reference. It has few interesting constructs

# TODO: Currently are errors in response determined by checking number
#       of webpages. It should be instead done by checking field in
#       response that says if response is ok

if [ $# -le 1 ]; then
    echo "You can pass config file as argument"
fi

echo "Program downloader";
echo "-----------------------------"
echo "-----------------------------"

set -o noclobber

# Constants, that can be set by config
DIRECTORY=~"/Documents"
KEY="test"

function check_error {
    if [ $? -gt 0 ]; then
        echo "Something went wrong"
        exit 2
    fi
}

# Load config and clear bad rows from there
if [ -f "$1" ]; then
    tmp_file="$( mktemp )";
    egrep '^#|^[^ ]*=[^;&]*'  "$1" >> "$tmp_file"
    check_error
    source $tmp_file
    api_key="api-key=""$KEY"

else
    echo "No config file specified, continue[y/N]?"
    read resp
    if [ "$resp" != "y" ] ; then
        exit 0
    fi
fi

check_error
# ==========================================

# Set variables
link='http://content.guardianapis.com/search'
fields='show-fields=wordcount%2Cbyline'
from_date='from-date='
to_date='to-date='
page_str='page='
sub_dir="guardian"
day_offset=1
tmp_file=""
# ==========================================

# ============== FUNCTIONS =================
# ==========================================
function quit {
    echo ""
    if [ "$file" != "" ]; then
        echo "Consider deleting file $file"
    fi
    rm $tmp_file 2>/dev/null
    echo "Stopping"
    exit 2;
}


function create_file {
    file="$1"
    offset="$2"
    page=1
    echo -n "Downloading  $( date --date="$offset days ago" "+%Y-%m-%d") "
    > "$1"
    if [ $? -gt 0 ]; then
        echo "Error opening file for writing $file"
        return 1
    fi

    while [ 1 ]; do
        url="$link?$api_key&$fields&\
page-size=100&\
$from_date$( date --date="$offset days ago" "+%Y-%m-%d")&\
$to_date$( date --date="$offset days ago" "+%Y-%m-%d")&\
$page_str$page"

        ret="$( curl $url  2>/dev/null )"
        total_pages=$( echo "$ret" | jq ".response.pages" )

        # Check if data is (probably) ok
        if [[ ! "$total_pages" =~ ^[0-9]+$ ]] ; then
            echo "Parsing error"
            tmp_last_message_file="$( mktemp )"
            echo "$ret" >> "$tmp_last_message_file"
            echo  "URL: " "$url"
            echo "Last response is saved in file $tmp_last_message_file"
            return 1
        fi

        # Save data
        echo "$ret" | jq ".response.results" >> "$file"

        if [ "$page" -eq "1" ]; then
            echo -n "$total_pages pages"
        fi

        if [ "$total_pages" -gt "$page" ]; then
            page=$(( "$page" + 1 ))
        else
            echo ""
            echo "Finished"
            return 0;
        fi
        echo -n "."
        sleep 0.2
    done
} # create_file
# ==========================================
# ============= END FUNCTIONS ==============
# ==========================================

trap quit EXIT                  # Cleanup on exit

# ==========================================
# ================= MAIN ===================
# ==========================================

# Create base directory, if script was launched for the first time
eval directory="$DIRECTORY/$sub_dir";
if [ ! -d "$directory" ]; then
    mkdir "$directory";
    if [ "$?" -gt 0 ]; then
        echo "Cannot create base directory $directory, exitting";
        exit 1;
    fi
fi
# ==================================================

# For each day find its file and if it doesn't exists, than download it
while [ 1 ]; do
    day_offset=$(( "$day_offset" + 1 ))
    file_name="$directory/$( date --date="$day_offset days ago" "+%Y-%m-%d")"

    sleep 0.01

    if [ -f "$file_name" ]; then # Info output
        current_size=$(du -b -h $file_name 2>/dev/null |  cut -f 1 | cut -f 1 -d ' ' )
        if [ "$current_size" == "0" ]; then
            echo "File $file_name seems to be empty, consider deleting...";
        else
            echo "$file_name   $current_size";
        fi
        continue # Go to next file
    fi

    # File does not exist
    create_file "$file_name" "$day_offset"

    if [ $? -gt 0 ]; then # Error creating file
        echo "Error, quitting"
        exit $?
    fi
done
# ==================================================

echo "Script finished" # Never happens
