package abcd;

import java.util.Map;

import static com.google.common.collect.Maps.newHashMap;

class ValueTable {

    int nextName = 1;

    int nextName() {
        return nextName++;
    }

    Map<Integer, Value> byName = newHashMap();
    Map<String, Value> byNick = newHashMap();
    Map<Integer, Value> byConstant = newHashMap();

    private Value newValue() {
        Value v = new Value();
        v.vt = this;
        v.name = nextName();
        byName.put(v.name, v);
        return v;
    }

    private Value newValue(String nick) {
        Value v = newValue();
        v.nick = nick;
        byNick.put(nick, v);
        return v;
    }

    private Value constant(int constant) {
        Value v = newValue();
        v.constant = constant;
        byConstant.put(constant, v);
        return v;
    }

    Value v(String nick) {
        Value v = byNick.get(nick);
        if (v == null) v = newValue(nick);
        return v;
    }

    Value v(int constant) {
        Value v = byConstant.get(constant);
        if (v == null) v = constant(constant);
        return v;
    }

}
