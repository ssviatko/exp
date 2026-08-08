/*
 * compile with g++ rtmiditest.cc -l asound -l pthread -l rtmidi
 * install rtmidi, alsa-utils, aj2midid if they are not available
 *
 * 1. Start this program
 * 2. Start a2jmidid in another console
 * 3. Start qjackctl
 * 4. Start sequencer (reaper)
 * 5. In qjackctl graph view, tie reaper midi output to a2j input for this program
 * 6. Send MIDI data
 */

#include <iostream>
#include <iomanip>
#include <vector>

#include "rtmidi/RtMidi.h"

void mycallback( double deltatime, std::vector<unsigned char> *message, void *userData )
{
	unsigned int nBytes = message->size();
	for ( unsigned int i=0; i<nBytes; i++ )
		std::cout << "Byte " << std::hex << i << " = " << (int)message->at(i) << ", " << std::dec;
	if ( nBytes > 0 )
	std::cout << "stamp = " << deltatime << std::endl;
}

int main(int argc, char **argv)
{
	try {
		RtMidiIn midiin;
		midiin.openVirtualPort("RtMidi Input");
		midiin.setCallback(&mycallback);
		midiin.ignoreTypes(false, true, false);
		char input;
		std::cin.get(input);
	} catch (RtMidiError &error) {
		// Handle the exception here
		error.printMessage();
	}
	return 0;
}

