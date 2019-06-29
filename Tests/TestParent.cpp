#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "TestParent: Running from " << argv[0] << "\n";
    std::string command(argv[1]);
    for (int i = 2; i < argc; i++)
    {
        command += " ";
        command += argv[i];
    }
    std::cout << "TestParent: Starting key daemon using command \"" 
            << command << "\"\n";
    int retVal = system(command.c_str());

    if (retVal != 0)
    {
        return 1;
    }
    return 0;
}
