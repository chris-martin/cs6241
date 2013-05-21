#ifndef LLVM_PHASEONE_INTEGER
#define LLVM_PHASEONE_INTEGER

using namespace llvm;
using namespace std;

namespace phaseone {

  class Integer {

public:

    static Integer none();

    static Integer some(APInt v);

    inline bool hasValue() { return _hasValue; }

    inline APInt value() { return _value; }

    void print(raw_ostream &o);

private:

    bool _hasValue;

    APInt _value;

  };

  raw_ostream &operator<<(raw_ostream &o, Integer &i);

}

#endif
