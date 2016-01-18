/*
 * test.cpp
 *
 *  Created on: Dec 3, 2015
 *      Author: richard
 */

#define DEBUG_OUT

#include <Playback.h>
#include <Playlist.h>
#include <Connection.h>

#include <Telegram/TelegramObject.h>
#include <DisplayPosition.h>
#include <DisplayString.h>
#include <DisplayList.h>
#include <DotMatrixClient.h>
#include <DisplayCommunication.h>
#include <EventSystemClient.h>
#include <Logging/LoggerAdapter.h>

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <exception>

#include "MPCClient.h"

using namespace EventSystem;
using namespace DotMatrix;
using namespace mpdAccess;

using namespace std;



#ifdef RASPBERRY
#define PATH "/home/pi/.config/mpd/socket"
#else
#define PATH "/home/richard/.config/mpd/socket"
#endif //RASPBERRY

//#define PATH "localhost"
/*
std::string getInfo(std::string str, Xmms::Dict info)
{
	std::string retVal;
	std::stringstream stream;

	try
	{
		stream << (info[str]);
	}
	catch (Xmms::no_such_key_error* err)
	{
		stream.str("Unknown");
	}
	std::cout << stream.str() << std::endl;
	retVal = stream.str();
	return retVal;
}
*/
int main(void)
{
/*
	DotMatrixClient* dmclient = new DotMatrixClient();
	DisplayList::setCommunicationModule(dmclient);

	DisplayList* list = new DisplayList(DisplayCommunication::FULL, DisplayCommunication::LEFT);

	printf("Resolution: %d x %d\n", dmclient->getXResolution(), dmclient->getYResolution());


	std::cout << "in main():\n";

	EventSystemClient espart(Telegram::ID_AUDIOPLAYER);
	espart.connectToMaster();
	espart.startReceiving();
	LoggerAdapter::initLoggerAdapter(&espart);

	printf("main: espart initialized\n");



	Connection* c;
	try
	{
		c = new Connection(PATH);
	} catch (std::exception& ex)
	{
		printf("Hellor world\n");
		LoggerAdapter::log(Log::SEVERE, ex.what());
		return -1;
	}

	Playback* playback = new Playback();
	Playlist* playlist = new Playlist();

	mpd_run_toggle_pause(Connection::getConnection());

	printf("Error in Connection: %d\n", mpd_connection_get_error(Connection::getConnection()));

	list->list = playlist->obtainSongsByTag(MPD_TAG_ALBUM, "Nord");
	list->display();

	delete c;
*/
	MPC_Client* client = new MPC_Client(PATH);
	std::string test;

	cout << "enter smth and press return to quit" << endl;
	cin >> test;

	delete client;

	return 0;
}


