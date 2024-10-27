import torch

# 5 x 3 matrix
x = torch.ones(5, 3);
x = torch.empty(5, 3);
x = torch.rand(5, 3, dtype=torch.double);

print("pytorch tensor randoms are form 0 to 1");
print("datatype of tensor elemnt:", x.dtype);
print(x);

y = torch.rand(5, 3);

print("element wise add:");
print(x + y);

y.add_(x);
print("methods that end with _ are inplace methods");
print(y);

print("is x instance of tensor",isinstance(x, torch.Tensor), sep=": ");

comp_val = 1 + 2j;
print("python has built in complex values,", comp_val, "-> r/i", comp_val.real, comp_val.imag);
print('\tuse "abs(comp_value)" to get magnitude,', abs(comp_val));
print(f'formatted strings start with f", inline insertions {comp_val:5.3f}');

usr_in = input("enter someting: ");


class Clas:
    # elemnt scope hel
    def __init__(self, a, b):
        self.varA = a;
        self.varB = b;

    # opeartor overloading
    def __add__(self, other):
        return Clas(self.varA + other.varA, self.varB + other.varB)

    def moo() -> str:
        """
        Returns a string representing the sound a cow makes.
        
        Returns:
            str: The sound a cow makes.
        """
        return "moo";

class Derived(Clas):
    # methods of same name override base classes methods

    def __init__(self, a, b):
        super().__init__(a, b); # specify parents method

    def moo() -> str:
        return "meow";

# things cant be empty, so pass is used to indicate nothing
class Derived2(Clas):
    # in this case it just inerits everyhting as-is from base
    pass