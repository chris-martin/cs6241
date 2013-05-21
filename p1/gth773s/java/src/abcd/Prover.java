package abcd;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

import static com.google.common.collect.Maps.newHashMap;
import static java.util.Arrays.asList;

class Prover {

    Graph g;
    Set<Value> phis;
    Map<Value, Integer> active;
    Memo memo = new Memo();

    Prover(Graph g, Set<Value> phis) {
        this.g = g;
        this.phis = phis;
    }

    boolean demandProve(Inequality ineq) {
        active = newHashMap();
        return asList(L.True, L.Reduced).contains(prove(ineq));
    }

    private L prove(Inequality ineq) {
        Value a = ineq.a();
        Value b = ineq.b();
        Integer c = ineq.c();
        {
            L alreadyProven = memo.get(ineq);
            if (alreadyProven != null) return alreadyProven;
        }
        Collection<Inequality> predecessorsOfB = g.getPredecessors(b);
        if (predecessorsOfB.size() == 0) return L.False;
        {
            Integer activeB = active.get(b);
            if (activeB != null) return c > activeB ? L.False : L.Reduced;
        }
        active.put(b, c);
        for (Inequality pred : predecessorsOfB) {
            L subResult = prove(new Inequality(a, pred.a(), c - pred.c()));
            LBound.Action action = isPhi(b) ? LBound.Action.Weaken : LBound.Action.Strengthen;
            memo.modify(action, ineq, subResult);
        }
        active.put(b, null);
        L result = memo.get(ineq);
        return result;
    }

    private boolean isPhi(Value v) {
        return phis.contains(v);
    }

}
