// HelloWorld with no CRT
#pragma comment(linker, "/ENTRY:main")
#pragma comment(linker, "/NODEFAULTLIB")
#include <Windows.h>
int main()
{
    MessageBoxA(0, "Hello", "World", 0);
    return 0;
}