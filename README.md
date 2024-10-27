# defFlag
The library that's is highly inspired by the Go's Flag library

# Documentation

## Methods

- **Parse**(argumentsCount, arguments, ignoreUnexpectedFlags, startFromNext) parses all flags
    1. **argumentsCount** - size of the **arguments**
    2. **arguments** - list of the arguments, the reason for storing that as **char\* []** is because most of the time
    you will pass **argc** and **argv** from the `int main(int argc, char* argv[])` function
    3. **ignoreUnexpectedFlags** - just skips unspecified flags on true and throws an exception on false
    4. **startFromNext** - skips the first element on true
    
        Returns the index of the first argument in tail and **argumentsCount** if there is no tail.

- **Set**(name, defaultValue, usage) sets the flag info
    1. **name** - the name of the flag
    2. **defaultValue** - the default value of the flag
    3. **usage** - text that will be printed when calling **PrintUsage** method

        Returns the reference to the argument so you can use it to access arguments's value after calling **Parse**.

- **Lookup**(name)
    1. **name** - a name of the argument

        Returns the Flag::Entry structure for a specified flag.

- **PrintUsage**(os) prints the information about every flag
    1. **os** - std::ostream for saving text

## Usage

1. Create an instance of the **def::Flag** class
2. Initialise flags by calling the **Set** method
3. Call the **def::Flag::Parse** method
4. Now you can access values of the flags

## Behaviour

1. You can initialise values in different ways:
    - `-name=flag`
    - `-name flag`
    - `--name=flag`
    - `--name flag`
2. You can't do that:
    - `name=flag`
    - `name flag` (in this case it begins to be treated as a tail)
3. Bool can be treated in different ways: if a flag has a type of **bool** then if no value specified for the argument it's value is set to true but also you can set it's value directly, for example, if the default value for a flag is **true**:
    - `-bool_flag` (now *bool_flag* is true)
    - `--bool_flag` (the same as before)
    - `-bool_flag [arg]` (*arg* can be: **T**, **t**, **F**, **f**, **True**, **TRUE**, **False**, **FALSE**, **true**, **false**, **0**, **1**)

4. Tail is a list of arguments that starts from an index returned by the **Parse** method:
    - `-flag_name flag_argument tail_argument_1 tail_argument_2 ...`

5. If you want to specify negative numeric literal as a value for the flag:
    - `-money=-21` (correct way so the value of *money* flag is *-21*)
    - `-money -21` (incorrect way because in this case *money* doesn't have a value at all and *-21* is treated as a flag with the name of *21* and it will throw an exception because numeric literals can't be used as names for flags)

6. If you specify the help flag as the first argument you'll see the usage (basicaly it will call **PrintUsage method**)
    - Possible flags: `-h`, `-help`, `--help`
