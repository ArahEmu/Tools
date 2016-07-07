#include "Main.h"
#include "WinsockDetours.h"
#include "RC4Disabler.h"

void main()
{
    WinsockDetours::init();
    RC4Disabler::init();
}
