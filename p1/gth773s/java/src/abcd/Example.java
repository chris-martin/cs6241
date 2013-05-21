package abcd;

import java.util.Set;

import static com.google.common.collect.Sets.newHashSet;
import static java.util.Arrays.asList;

class Example {

    ValueTable vt = new ValueTable();
    Graph g = new Graph();
    Set<Value> phis = newHashSet();
    Prover prover = new Prover(g, phis);

    Value v(int constant) {
        return vt.v(constant);
    }

    Value v(String nick) {
        return vt.v(nick);
    }

    void add(Value u, Value v, int c) {
        g.add(new Inequality(u, v, c));
    }

    void testPaperExample() {

        add(v(-1), v("st0"), 0);
        add(v("st0"), v("st1"), 0);
        add(v("st1"), v("st2"), 0);
        add(v("st2"), v("st3"), 1);
        add(v("st3"), v("st1"), 0);
        add(v("st3"), v("j0"), 0);
        add(v("j0"), v("j1"), 0);
        add(v("j1"), v("j2"), 0);
        add(v("j2"), v("j3"), 0);
        add(v("j3"), v("j4"), 1);
        add(v("j4"), v("j1"), 0);
        add(v("j3"), v("t0"), 1);
        add(v("j3"), v("A.length"), -1);
        add(v("A.length"), v("limit0"), 0);
        add(v("limit0"), v("limit1"), 0);
        add(v("limit1"), v("limit2"), 0);
        add(v("limit2"), v("limit3"), -1);
        add(v("limit3"), v("limit1"), 0);
        add(v("limit3"), v("limit4"), 0);
        add(v("limit4"), v("j2"), -1);
        add(v("limit2"), v("st2"), -1);

        for (String s : asList("st1", "j1", "limit1")) phis.add(v(s));

        Inequality toProve = new Inequality(v("j2"), v("A.length"), -1);

        System.out.println(prover.demandProve(toProve));
        System.out.println(prover.memo);

    }

    public static void main(String[] args) {
        new Example().testPaperExample();
    }

}
