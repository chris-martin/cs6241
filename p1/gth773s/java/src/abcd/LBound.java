package abcd;

import com.google.common.base.Function;
import com.google.common.base.Joiner;
import com.google.common.collect.Lists;

import java.util.Collection;
import java.util.Collections;
import java.util.List;

import static com.google.common.collect.Lists.newArrayList;

class LBound {

    private Integer provenTrue;
    private Integer provenReduced;
    private Integer provenFalse;

    L get(int c) {
        if (provenTrue != null && provenTrue <= c) return L.True;
        if (provenFalse != null && provenFalse >= c) return L.False;
        if (provenReduced != null && provenReduced <= c) return L.Reduced;
        return null;
    }

    enum Action { Strengthen, Weaken }

    void modify(Action action, L l, int c) {
        switch (action) {
            case Strengthen: strengthen(l, c); break;
            case Weaken: weaken(l, c); break;
            default: throw new Error();
        }
    }

    private static boolean isMaxBound(L l) {
        return l == L.False;
    }

    private void strengthen(L l, int c) {
        if (l != null) {
            put(l, strongerBound(l, get(l), c));
        }
    }

    private static Integer strongerBound(L l, Integer a, Integer b) {
        if (a == null) return b;
        if (b == null) return a;
        return isMaxBound(l) ? Math.max(a, b) : Math.min(a, b);
    }

    private void weaken(L l, int c) {
        if (l == null) {
            clear();
        } else {
            put(l, weakerBound(l, get(l), c));
        }
    }

    private static Integer weakerBound(L l, Integer a, Integer b) {
        if (a == null || b == null) return null;
        return isMaxBound(l) ? Math.min(a, b) : Math.max(a, b);
    }

    boolean isVacuous() {
        return provenTypes().size() == 0;
    }

    private void clear() {
        provenTrue = provenReduced = provenFalse = null;
    }

    private Integer get(L l) {
        switch (l) {
            case True: return provenTrue;
            case Reduced: return provenReduced;
            case False: return provenFalse;
            default: throw new Error();
        }
    }

    private void put(L l, Integer c) {
        switch (l) {
            case True: provenTrue = c; break;
            case Reduced: provenReduced = c; break;
            case False: provenFalse = c; break;
            default: throw new Error();
        }
    }

    private Collection<L> provenTypes() {
        List<L> list = newArrayList();
        if (provenTrue != null) list.add(L.True);
        if (provenReduced != null) list.add(L.Reduced);
        if (provenFalse != null) list.add(L.False);
        return list;
    }

    static LBound fixed(int c) {
        LBound bound = new LBound();
        bound.provenTrue = c;
        bound.provenFalse = c - 1;
        return bound;
    }

    public String toString(final Diff diff) {
        List<L> keys = newArrayList(provenTypes());
        Collections.sort(keys);
        return Joiner.on("\n").join(Lists.transform(keys, new Function<L, String>() {
            public String apply(L l) {
                return l + ":\t" + diff.b() + " - " + diff.a() + " ≤ " + get(l);
            }
        }));
    }

}
