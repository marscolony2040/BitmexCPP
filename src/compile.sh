#!/bin/bash
g++ test.cpp bitmex.cpp encrypt++/encoder.cpp -l:libcryptopp.a -lcurl -lpthread -lcpprest
exit 0
