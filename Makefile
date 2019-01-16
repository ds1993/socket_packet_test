all: testserver testclient

testserver: testserver.c
	g++ -g -o $@ $<

testclient: testclient.c
	g++ -g -o $@ $<

clean:
	rm -rf testserver testclient
