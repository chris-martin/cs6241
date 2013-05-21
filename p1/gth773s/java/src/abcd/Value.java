package abcd;

class Value {

    ValueTable vt;
    int name;
    String nick;
    Integer constant;

    boolean isConstant() {
        return nick == null;
    }

    @Override
    public String toString() {
        return nick != null ? nick : Integer.toString(constant);
    }

}
