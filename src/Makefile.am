@CODE_COVERAGE_RULES@
bin_PROGRAMS=speedtest
INCLUDE = @LIBXML_CFLAGS@
speedtest_SOURCES=main.cpp SpeedTest.cpp SpeedTestClient.cpp MD5Util.cpp
speedtest_CXXFLAGS=-std=c++11 -Wall -Werror -pthread $(EXTRA_CFLAGS)
speedtest_LDADD=$(EXTRA_LIBS) -lpthread
