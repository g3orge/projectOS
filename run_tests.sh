#!/bin/bash
# This is a script that enables the user to make
# multiple client requests to the server 
# argument 1 specifies how many times the client program
# is going to execute 
# argument 2 is specifies the name of the executable

# if no arguments are giver default values are : 
# executable = client 
# number of repetitions = 100

pgrep -l server > /dev/null
if [ $? -eq 1 ]; then
    echo "Server not running";
    echo "run ./server before the script";
    # commit suicide
    return
    kill $$
fi
# Get the first argument, how many repetitions
if [ $1 ]; then 
    n=$1;
else
    n=100; 
fi

if [[ -x $2 ]]; then
    for (( i=1; i<=n; i++ ))
    do
        ./$2 rand  # call the client as specified by second argument
        sleep 0.2  # just a little delay in order
        # to have the rand function generate
        # different orders
        echo "Order $i sent ";
    done
else
    if [[ -x client ]]; then

        for (( i=1; i<=n; i++ ))
        do
            ./client rand  # call the client as default
            sleep 0.01     # sleep a bit to get different numbers in the PRG
            echo "Order $i sent!";
        done 
    else
        echo "specify the name of the executable";
    fi	
fi
