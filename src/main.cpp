#include "PIDStudio.h"

int main(int argc, char* argv[])
{
    auto* application = new PIDStudio();
    int result = application->run();
    delete application;
    return result;
}
