all:main.cpp
	g++ -std=c++11 -Os main.cpp -o enml2html

clean:
	rm enml2html -fr