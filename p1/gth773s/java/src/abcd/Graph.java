package abcd;

import com.google.common.collect.ArrayListMultimap;
import com.google.common.collect.Multimap;

import java.util.Collection;

class Graph {

    Multimap<Value, Inequality> predecessors = ArrayListMultimap.create();

    void add(Inequality i) {
        predecessors.put(i.b(), i);
    }

    Collection<Inequality> getPredecessors(Value v) {
        return predecessors.get(v);
    }

}
