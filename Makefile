all: json uglyjson

install: /usr/local/bin/json /usr/local/bin/uglyjson

clean:
	rm -f json json.o uglyjson uglyjson.o

json: json.o
	g++ -o json json.o

json.o: json.cpp
	g++ -c -o json.o json.cpp

uglyjson: uglyjson.o
	g++ -o uglyjson uglyjson.o

uglyjson.o: uglyjson.cpp
	g++ -c -o uglyjson.o uglyjson.cpp

/usr/local/bin/json: json
	sudo cp json /usr/local/bin/json

/usr/local/bin/uglyjson: uglyjson
	sudo cp uglyjson /usr/local/bin/uglyjson
