CPP_FLAGS:=`pkg-config --cflags raylib`
LD_FLAGS:=`pkg-config --libs raylib`

main: main.cpp
	$(CXX) main.cpp -o main $(CPP_FLAGS) $(LD_FLAGS)