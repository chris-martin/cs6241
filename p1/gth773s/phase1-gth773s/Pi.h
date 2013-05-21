#ifndef LLVM_PHASEONE_PI
#define LLVM_PHASEONE_PI

namespace phaseone {

  class Pi {

public:

    static Pi *create(Value *value, BasicBlock *from, BasicBlock *to, bool taken);

    inline PHINode *phi() { return _phi; }

    Value *value();

    inline BasicBlock *from() { return _from; }

    inline BasicBlock *to() { return _to; }

    inline bool taken() { return _taken; }

    void updatePhiName();

private:

    std::string createPhiName(Value *v);

    PHINode *_phi;

    BasicBlock *_from;

    BasicBlock *_to;

    bool _taken;

  };

}

#endif

