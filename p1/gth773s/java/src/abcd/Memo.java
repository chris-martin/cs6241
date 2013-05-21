package abcd;

import com.google.common.base.Function;
import com.google.common.base.Joiner;
import com.google.common.collect.Lists;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;

import static com.google.common.collect.Lists.newArrayList;
import static com.google.common.collect.Maps.newHashMap;

class Memo {

    private Map<Diff, LBound> map = newHashMap();

    L get(Inequality inequality) {
        Diff diff = inequality.diff();
        LBound bound = diff.isConstant() ? diff.fixedBound() : map.get(diff);
        return bound == null ? null : bound.get(inequality.c());
    }

    void modify(LBound.Action action, Inequality inequality, L l) {
        Diff diff = inequality.diff();
        LBound bound = map.get(diff);
        if (bound == null) map.put(diff, bound = new LBound());
        bound.modify(action, l, inequality.c());
        if (bound.isVacuous()) map.remove(diff);
    }

    @Override
    public String toString() {
        List<Diff> keys = newArrayList(map.keySet());
        Collections.sort(keys, valuePairComparator);
        return Joiner.on("\n").join(Lists.transform(keys, valuePairToString));
    }

    private Function<Diff, String> valuePairToString = new Function<Diff, String>() {
        public String apply(Diff diff) {
            return map.get(diff).toString(diff);
        }
    };

    private static Comparator<Diff> valuePairComparator = new Comparator<Diff>() {
        public int compare(Diff a, Diff b) {
            int result = compare(a.b(), b.b());
            return result != 0 ? result : compare(a.a(), b.a());
        }
        private int compare(Value a, Value b) {
            if (a == b) return 0;
            if (a.isConstant() != b.isConstant())
                return Boolean.valueOf(a.isConstant())
                    .compareTo(Boolean.valueOf(b.isConstant()));
            else if (a.isConstant())
                return Integer.valueOf(a.constant).compareTo(Integer.valueOf(b.constant));
            else
                return a.nick.compareTo(b.nick);
        }
    };

}
