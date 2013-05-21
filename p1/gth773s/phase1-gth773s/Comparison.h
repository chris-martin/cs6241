#ifndef LLVM_PHASEONE_ABC_COMPARISON
#define LLVM_PHASEONE_ABC_COMPARISON

using namespace llvm;

namespace phaseone {

  class Comparison {

private:

    TerminatorInst *_terminator;
    ICmpInst *_icmp;

    Value *_v_i;
    Value *_v_j;
    Value *_v_k;

    Value *_w_r;
    Value *_w_s;
    Value *_w_t;

public:

    inline TerminatorInst *terminator() { return _terminator; }
    inline void terminator(TerminatorInst *t) { _terminator = t; }

    inline ICmpInst *icmp() { return _icmp; }
    inline void icmp(ICmpInst *x) { _icmp = x; }

    inline Value *v_i() { return _v_i; };
    inline Value *v_j() { return _v_j; };
    inline Value *v_k() { return _v_k; };

    inline Value *w_r() { return _w_r; };
    inline Value *w_s() { return _w_s; };
    inline Value *w_t() { return _w_t; };

    inline void v_i(Value *x) { _v_i = x; };
    inline void v_j(Value *x) { _v_j = x; };
    inline void v_k(Value *x) { _v_k = x; };

    inline void w_r(Value *x) { _w_r = x; };
    inline void w_s(Value *x) { _w_s = x; };
    inline void w_t(Value *x) { _w_t = x; };

    void setValue(int operand, int which, Value *v);

  };

}

#endif
