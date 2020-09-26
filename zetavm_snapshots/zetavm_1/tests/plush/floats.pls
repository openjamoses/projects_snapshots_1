#language "lang/plush/0"

fp_assert = function (xVal, xExpect, errMsg)
{
    var err = xVal - xExpect;
    if (err < 0)
        err = -err;

    assert (err < 0.001f, errMsg);
};

// assigning floats;
x = 1.0f;
y = 1e2f;

// arithmetic
x = 1.0f + 3.5f;
fp_assert(x, 4.5f, "float addition doesn't work");
y = 5.5f - 2.8f;
fp_assert(y, 2.7f, "float substraction doesn't work");
w = 9.0f / 3.0f;
fp_assert(w, 3.0f, "float division doesn't work");
z = 4.4f * 3.3f;
fp_assert(z, 14.52f, "float multiplication doesn't work");

// automatic type casting
x = 4.5f * 5.4f / 3 + 3 - 2.5f;
fp_assert(x, 8.6f, "float auto casting and arithmetic doesn't work");

// Import the math package
var math = import "std/math/0";

assert (1e20f < math.INF);

fp_assert(math.max(3.0f, 2.8f), 3.0f, "max function on floats is not working");
assert(math.max(3,2) == 3, "max function on ints is not working");
assert(math.max(0, -0.5f) == 0);

fp_assert(math.min(3.0f, 2.8f), 2.8f, "min function on floats is not working");
assert(math.min(3,2) == 2, "min function on ints is not working");

fp_assert(math.abs(-3.5f), 3.5f, "abs function on floats is not working");
assert(math.abs(-3) == 3, "abs function on ints is not working");

assert (math.floor(3.7f) == 3);
assert (math.floor(0.0f) == 0);
assert (math.floor(-3.3f) == -4);

x = math.sin(50.8f);
fp_assert(x, 0.50942f, "sin for float doesn't work");
y = math.sqrt(30.8f);
fp_assert(y, 5.54977f, "sqrt for float doesn't work");
z = math.cos(40);
fp_assert(z, -0.66693f, "cos for float doesn't work");

fp_assert(math.sin(math.PI), 0, "sin of PI == 0 is not working");
fp_assert(math.sin(math.PI/2), 1, "sin of PI/2 == 1 is not working");

// comparisons
assert(1.0f < 4.0f);
assert(1.0f <= 4.0f);
assert(4.0f > 1.0f);
assert(4.0f >= 1.0f);
assert(2.2f > 2.1f);
assert(1.11f < 1.12f);
assert(-1.1f < 2.0f);
assert(10000 > -0.0004f);
assert(0.005f > 0.0005f);
assert(0.0004f <= 0.0005f);
x = 1.0f == 1.0f;
assert(typeof x == "bool");
assert(1.0f == 1.0f);

// float-string conversion
x = $str_to_f32("1.0");
fp_assert(x, 1.0f, "string to float doesn't work");
assert($f32_to_str(1.0f) == "1.000000");
assert (math.isNaN($str_to_f32("10e777f")));
