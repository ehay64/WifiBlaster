#! /usr/bin/awk -f

BEGIN {

}

{
    system("printf \"%x\" " "$3");
}

END {

}
