#include "PhaseOne.h"
#include <sstream>

using namespace llvm;
using namespace std;

namespace phaseone {

  Pi *Pi::create(Value *value, BasicBlock *from, BasicBlock *to, bool taken) {
    Pi *pi = new Pi();
    pi->_from = from;
    pi->_to = to;
    pi->_taken = taken;
    Instruction *insertBefore = to->begin();
    PHINode *phi = PHINode::Create(value->getType(), pi->createPhiName(value), insertBefore);
    //errs() << "inserting\n";
    //phi->print(errs());
    //errs() << "\n";
    //insertBefore->print(errs());
    //errs() << "\n";
    phi->addIncoming(value, from);
    pi->_phi = phi;
    return pi;
  }

  std::string Pi::createPhiName(Value *v) {
    std::stringstream name;
    name << "pi-" << (!v->hasName() ? "anon" : v->getNameStr())
         << "_" << (!_from->hasName() ? "anon" : _from->getNameStr())
         << "_" << (_taken ? "T" : "F");
    return name.str();
  }

  void Pi::updatePhiName() {
    _phi->setName(createPhiName(value()));
  }

  Value *Pi::value() {
    return _phi->getIncomingValue(0);
  }

}
