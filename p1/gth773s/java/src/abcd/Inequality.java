package abcd;

// b - a <= c
class Inequality {

    private Diff diff;
    private int c;

    Inequality(Diff diff, int c) {
        this.diff = diff;
        this.c = c;
    }

    Inequality(Value a, Value b, int c) {
        this(new Diff(a, b), c);
    }

    Diff diff() {
        return diff;
    }

    Value a() {
        return diff.a();
    }

    Value b() {
        return diff.b();
    }

    int c() {
        return c;
    }

    @Override
    public String toString() {
        return "{ " + diff.b()
            + " - " + diff.a()
            + " ≤ " + c + " }";
    }

}
