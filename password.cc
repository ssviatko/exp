#include <iostream>
#include <string>

#include <termios.h>
#include <unistd.h>

std::string getpw()
{
	std::string l_password;
	termios oldt;
	tcgetattr(STDIN_FILENO, &oldt);
	termios newt = oldt;
	newt.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	char ch;
	while ((ch = getchar()) != '\n') {
		if (ch == 127) {
			// Handle backspace
			if (!l_password.empty()) {
				l_password.pop_back();
			}
		} else {
			l_password += ch;
		}
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return l_password;
}

int main(int argc, char **argv)
{
	std::cout << "password: ";
	std::string l_pw = getpw();
	std::cout << "Your password is: " << l_pw << std::endl;
	return 0;
}

