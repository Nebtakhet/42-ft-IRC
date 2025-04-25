#!/bin/bash

# Run the netcat command 1200 times
for i in {1..1200}
do
    echo "Starting client #$i"
    {
        echo "PASS hola"
        echo "JOIN #channhive"
    } | netcat localhost 6667 &
done

# Wait for all background processes to finish
wait