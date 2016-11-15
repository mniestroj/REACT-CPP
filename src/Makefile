AR		= ar
CXX		= g++
RM		= rm -f
MY_CXXFLAGS	= -Wall -MMD -c -I. -std=c++11
MY_LDFLAGS	= -Wall -shared
SHARED_LIB	= lib$(LIBRARY_NAME).so.$(VERSION)
STATIC_LIB	= lib$(LIBRARY_NAME).a.$(VERSION)
STATIC_FPIC_LIB = lib$(LIBRARY_NAME)_fpic.a
SOURCES		= $(wildcard *.cpp */*.cpp)
DEPENDENCIES    = $(SOURCES:%.cpp=%.d)
SHARED_OBJECTS	= $(SOURCES:%.cpp=%.o)
STATIC_OBJECTS	= $(SOURCES:%.cpp=%.s.o)

all:	CXXFLAGS = -g
all:	LDFLAGS = -g
all:	shared static

release:	CXXFLAGS = -O2
release:	LDFLAGS = -O2
release:	shared static

-include ${DEPENDENCIES}

shared:	${SHARED_OBJECTS} ${SHARED_LIB}

static_fpic: ${SHARED_OBJECTS} ${STATIC_FPIC_LIB}

static:	${STATIC_OBJECTS} ${STATIC_LIB}

${SHARED_LIB}: ${SHARED_OBJECTS}
	${CXX} ${MY_LDFLAGS} ${LDFLAGS} -Wl,-soname,lib$(LIBRARY_NAME).so.$(SONAME) -o $@ ${SHARED_OBJECTS} -lev -lcares

${STATIC_LIB}: ${STATIC_OBJECTS}
	$(AR) rcs ${STATIC_LIB} ${STATIC_OBJECTS}

${STATIC_FPIC_LIB}: ${SHARED_OBJECTS}
	$(AR) rcs ${STATIC_FPIC_LIB} ${SHARED_OBJECTS}

clean:
	${RM} *.obj *~* ${SHARED_OBJECTS} ${STATIC_OBJECTS} ${SHARED_LIB} ${STATIC_LIB}

${SHARED_OBJECTS}:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} ${MY_CXXFLAGS} -fpic -o $@ ${@:%.o=%.cpp}

${STATIC_OBJECTS}:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} ${MY_CXXFLAGS} -o $@ ${@:%.s.o=%.cpp}

