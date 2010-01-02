TEST_DIRECTORY=$(pwd)
PATH=$TEST_DIRECTORY/../src:$PATH
export PATH

say_color () {
    (
	case "$1" in
	    error) tput bold; tput setaf 1;; # bold red
	    skip)  tput bold; tput setaf 2;; # bold green
	    pass)  tput setaf 2;;            # green
	    info)  tput setaf 3;;            # brown
	    *) test -n "$quiet" && return;;
	esac
	shift
	printf "* %s" "$*"
	tput sgr0
	echo
    )
}

error () {
	say_color error "error: $*"
	exit 1
}

say () {
	say_color info "$*"
}


test_run_ () {
	eval >&3 2>&4 "$1"
	eval_ret="$?"
	return 0
}

test_ok_ () {
	test_success=$(($test_success + 1))
	say_color "" "  ok $test_count: $@"
}

test_failure_ () {
	test_failure=$(($test_failure + 1))
	say_color error "FAIL $test_count: $1"
	shift
	echo "$@" | sed -e 's/^/	/'
}

test_expect_success () {
    test_count=$(($test_count+1))
    say >&3 "expecting success: $2"
    test_run_ "$2"
    if [ "$?" = 0 -a "$eval_ret" = 0 ]
    then
	test_ok_ "$1"
    else
	test_failure_ "$@"
    fi
    echo >&3 ""
}

test "${test_description}" != "" ||
error "Test script did not set test_description."


exec 5>&1
if test "$verbose" = "t"
then
	exec 4>&2 3>&1
else
	exec 4>/dev/null 3>/dev/null
fi

test_failure=0
test_count=0
test_fixed=0
test_broken=0
test_success=0


test_cmp() {
    diff -u "$@"
}

test_done () {
    msg="$test_count test(s)"

    case "$test_failure" in
	0)
	    say_color pass "passed all $msg"
	    cd "$(dirname "$remove_trash")" &&
	    rm -rf "$(basename "$remove_trash")"
	    
	    exit 0 ;;
	
	*)
	    say_color error "failed $test_failure among $msg"
	    exit 1 ;;
	
    esac
}

test_must_fail () {
    "$@"
    test $? -gt 0 -a $? -le 129 -o $? -gt 192
}

test="trashdir.$(basename "$0" .sh)"
TRASH_DIRECTORY="$TEST_DIRECTORY/$test" 
remove_trash=$TRASH_DIRECTORY
mkdir -p $TRASH_DIRECTORY
cd $TRASH_DIRECTORY