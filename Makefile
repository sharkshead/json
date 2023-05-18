all: json

install: json
	sudo cp json /usr/local/bin/json

json: json.o
	g++ -o json json.o

json.o: json.cpp
	g++ -c -o json.o json.cpp
