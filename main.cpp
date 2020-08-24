#include <iostream>
#include "Json.hpp"


using namespace std;

int main()
{
    PrintJsonArray(cout)
            .BeginObject()
            .Key("hello")
            .String("ME")
            .EndObject()
            .BeginArray()
            .String("hihihi")
            .Null()
            .EndArray();

    cout << endl;

    PrintJsonArray(cout)
            .String("hohoho")
            .Boolean(false)
            .Null()
            .Number(1)
            .BeginObject()
            .Key("biba");
    return 0;
}
