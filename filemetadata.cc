/* Created entire with AI
 *
 * My first real attempt at creating something with AI (ZZZCODE.AI)
 *
 * Prompt criteria:
 *
 * create a class that contains space for a filename, file length, block size,
 * creation date, and modification date. Use <chrono> if possible. Make the filename
 * a char array and include a uint8 filename length specifier. Make a method to
 * reverse the filename based on the length specifier. In the printInfo method,
 * print the dates in iso8601 format. Use machine local time instead of zulu time
 * for the time stamp. Add microseconds to the timestamp.
 *
 */

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstring>

class FileInfo {
private:
    char filename[256];
    uint8_t filenameLength;
    size_t fileLength;
    size_t blockSize;
    std::chrono::system_clock::time_point creationDate;
    std::chrono::system_clock::time_point modificationDate;

public:
    FileInfo(const char* name, size_t length, size_t block)
        : fileLength(length), blockSize(block), filenameLength(strlen(name)) {
        strncpy(filename, name, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        creationDate = std::chrono::system_clock::now();
        modificationDate = creationDate;
    }

    void reverseFilename() {
        for (size_t i = 0; i < filenameLength / 2; ++i) {
            std::swap(filename[i], filename[filenameLength - i - 1]);
        }
    }

    void printInfo() const {
        auto to_iso8601 = [](const std::chrono::system_clock::time_point& tp) {
            std::time_t time = std::chrono::system_clock::to_time_t(tp);
            std::tm localTime = *std::localtime(&time);
            std::ostringstream oss;
            oss << std::put_time(&localTime, "%Y-%m-%dT%H:%M:%S");
            oss << '.' << std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count() % 1000000;
            return oss.str();
        };

        std::cout << "Filename: " << filename << "\n"
                  << "File Length: " << fileLength << "\n"
                  << "Block Size: " << blockSize << "\n"
                  << "Creation Date: " << to_iso8601(creationDate) << "\n"
                  << "Modification Date: " << to_iso8601(modificationDate) << "\n";
	}
};

int main(int argc, char **argv)
{
	FileInfo fi("stephen sviatko", 8765, 2345);
	fi.reverseFilename();
	fi.printInfo();
	return 0;
}

