#/bin/sh -

# This script is made to be called regularly to pull changes from remote to
# local git repository.

# -C -- noclobber
set -C;

USE_LOG=0
VERSION=1

print_info() {
	printf "$@"
	printf '\n'
	if test $USE_LOG; then
		printf "$@" | logger -p user.info
	fi
}

print_error() {
	printf "$@"
	printf '\n'
	if test $USE_LOG; then
		printf "$@" | logger -p user.error
	fi
}

usage() {
	echo "Usage: gitclone.sh clone and sync git repositories"
	echo " --help      | -h   Print help and exit"
	echo " --version   | -v   Print version and exit"
	echo " --directory | -d   Directory in which git repo is / should be cloned to"
	echo " --url       | -u   Url of git repo. Mandatory"
	echo " --log       | -l   Log all messages also to system logger"


}
version() {
	echo "$VERSION"
	exit 0
}

clone() {
	ERROR=$( git clone "$URL" "$DIR" 2>&1 )
	if test $? -ne 0; then 
		print_error "Git clone of repo %s failed with message %s" "$URL"  "$ERROR"
		exit 1
	fi
}

sync() {
	(
		cd $DIR;
		git fetch --all
	)
	local exit_code=$?
	if test "$exit_code" -ne 0; then 
		print_error "Git fetch of '%s' failed with code %d" "$URL" 
		exit 1
	fi

}

while test $# -gt 0
do
	case $1 in
		--help | --hel | --he | --h | '--?' | -help | -hel | -he | -h | '-?' )
			usage
			exit 0
			;;
		--version | -v )
			version
			exit 0
			;;
		--directory | -d )
			shift
			DIR="$1"
			;;
		--url | -u )
			shift
			URL="$1"
			;;
		--log | -l )
			USE_LOG=1
			;;
		-*)
			print_error "Unrecognized option: $1"
			exit 1
			;;
		*)
			print_error "Unknown option"
			exit 1
			;;
	esac
	shift
done

if test -z "$DIR" -o -z "$URL"; then
	print_error "Directory and url are required"
	exit 1
fi

prepare_ssh() {
	# First check if known_hosts exist
	if test ! -d ~/.ssh/; then
		print_info "Creating ssh folder"
		mkdir ~/.ssh
		chmod 0700 ~/.ssh
	fi

	if test ! -f ~/.ssh/known_hosts; then
		print_info "Creating ssh known_hosts"
		touch ~/.ssh/known_hosts
		chmod 0600 ~/.ssh/known_hosts
	fi

	# Extract domain from url
	DOMAIN=$( echo "$URL" | sed  's/^.*@//' | sed 's/:.*$//' )

	# Check if fingerprints are already in file
	# if ssh-keygen -F "$DOMAIN" | grep -q .; then
	if test "$( ssh-keygen -F "$DOMAIN" | wc -l )" = '0'; then
		print_info "Adding domain '%s' to known_hosts" "$DOMAIN"
		# -H output as hash
		ssh-keyscan -H "$DOMAIN"  2>&1 | sed '/^#/d' >> ~/.ssh/known_hosts
	fi
}




if echo "$URL" | grep -q  -v '^http'; then
	prepare_ssh
fi


if test -d  "$DIR"; then
	sync
else
	clone
fi

