CC=g++ -g -Wall --std=c++14

# List of source files for your thread library
THREAD_SOURCES=cpu.cpp cpu_impl.cpp thread.cpp thread_impl.cpp mutex.cpp mutex_impl.cpp cv.cpp cv_impl.cpp ucontext_wrapper.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

all: libthread.o app

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	ld -r -o $@ ${THREAD_OBJS}

# Compile an application program
app: app.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${THREAD_OBJS} libthread.o app
