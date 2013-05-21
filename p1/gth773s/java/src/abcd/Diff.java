package abcd;

class Diff {

    private Value a;
    private Value b;

    Diff(Value a, Value b) {
        this.a = a;
        this.b = b;
    }

    Value a() {
        return a;
    }

    Value b() {
        return b;
    }

    boolean isConstant() {
        return a == b || (a.isConstant() && b.isConstant());
    }

    Integer constantValue() {
        if (a == b) return 0;
        if (a.isConstant() && b.isConstant()) return b.constant - a.constant;
        return null;
    }

    LBound fixedBound() {
        Integer value = constantValue();
        return value == null ? null : LBound.fixed(value);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Diff diff = (Diff) o;
        if (a != null ? !a.equals(diff.a) : diff.a != null) return false;
        if (b != null ? !b.equals(diff.b) : diff.b != null) return false;
        return true;
    }

    @Override
    public int hashCode() {
        int result = a != null ? a.hashCode() : 0;
        result = 31 * result + (b != null ? b.hashCode() : 0);
        return result;
    }

    @Override
    public String toString() {
        return "(" + a + ", " + b + ")";
    }

}
