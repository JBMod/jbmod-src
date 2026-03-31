// Simple test script for Daslang integration
// This script demonstrates basic Daslang syntax

// Define a simple function
def add(a: int, b: int): int {
    return a + b;
}

// Test the function
var result = add(5, 3);
print("5 + 3 = " + string(result));

// Test with floats
def multiply(a: float, b: float): float {
    return a * b;
}

var product = multiply(2.5, 4.0);
print("2.5 * 4.0 = " + string(product));

// Test control flow
if (result > 5) {
    print("Result is greater than 5");
} else {
    print("Result is 5 or less");
}

// Simple loop
var sum = 0;
for (i in 0..4) {
    sum += i;
}
print("Sum of 0..4 = " + string(sum));