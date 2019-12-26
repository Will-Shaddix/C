#MultiThreaded HttpServer
This is my first program implementing multithreading, occasionally breaks with high thread counts 
and a high number of requests at the same time. 

To run, use the '''make''' Command with the Makefile, then ./httpserver, when running it, 
you can use flags to specify the name of the log file -l, how many threads with -n, 
the ip address of the server with -A, and the port number with -P

Ex. 
make \n
./httpserver -P 8080 -n 4 -A 192.1.1.128 -l log1
