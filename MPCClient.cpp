/*
 * MPCClient.cpp
 *
 *  Created on: Jan 16, 2016
 *      Author: richard
 */

#include "MPCClient.h"

#include <mpd/client.h>

#include <BehaviorObject.cpp>
#include <FunctionContainer.cpp>
#include <FunctionDescription.cpp>
#include <FunctionList.cpp>
#include <TokenConfig.cpp>
#include <TokenObject.cpp>

#include <FunctionContainer.h>
#include <FunctionDescription.h>
#include <FunctionList.h>
#include <TokenConfig.h>
#include <TokenObject.h>
#include <BehaviorObject.h>
#include <FunctionList.h>




#include <Telegram/Telegram.h>
#include <Telegram/TelegramObject.h>

using namespace TokenDaemon;

inline void callFunction(MPC_Client* client, Function_Description* fdesc, FunctionContainer* container)
{
	printf("callFunction(): %s @ %d\n", fdesc->getFunctionDescription(), container->getIndex(fdesc->getFunctionDescription()));
	switch (container->getIndex(fdesc->getFunctionDescription()))
	{
	case 0: client->displayArtistList(); break;
	case 1: client->displayAlbumList(""); break;
	case 2: client->displayTitleList(""); break;
	case 3: client->displayStatus(); break;
//	strcat("no action for function ", fdesc->getFunctionDescription())

	default: {
		std::string message = "no action for function ";
		message.append(fdesc->getFunctionDescription());
		LoggerAdapter::log(Log::WARNING, message);
		break;
	}
	}
}


void* control_thread(void* arg)
{
	MPC_Client* client = (MPC_Client*) arg;
	EventSystemClient* esc = client->getESClient();

	FunctionContainer* myFunctions = new FunctionContainer();

	myFunctions->add("display_AllArtists");
	myFunctions->add("display_AllAlbums");
	myFunctions->add("display_AllTitles");
	myFunctions->add("display_Status");

	void* data = malloc(DATASIZE);

	Telegram* telegram = new Telegram("");
	Telegram_Object* objTelegram = new Telegram_Object();
	Key* pressedKey = new Key("STOP", false);

	while (true)
	{
		esc->receive(data, false);
		telegram->deserialize(data);

#ifdef DEBUG_OUT
		printf("Received data of type %d\n", telegram->getType());
#endif //DEBUG_OUT

		switch(telegram->getType())	{
	case Telegram::INPUT: {
		objTelegram->deserialize(data, pressedKey);
		client->keyInput(pressedKey->getKeyIdentifier());
		break;
	}
	case Telegram::MEDIA: {
		break;
	}
	case Telegram::REQUEST: {
		Function_List* flist = new Function_List(esc->getUniqueIdentifier(), myFunctions->getSize(), myFunctions->getAsList());
		printf("Function 3: %s\n", flist->getFunctionDescription(2)->getFunctionDescription());
		objTelegram->setType((Telegram::telegram_type)Function_List::REQUEST_ANSWER);
		objTelegram->setIdentifier(telegram->getSourceID());
		objTelegram->setObject(flist);
		printf("myFunctions->getAsList: OK\n %p:\n", objTelegram->getObject());
		printf("%d\n", objTelegram->getObject()->getSerializedSize());
		printf("estimated size: %d\n", objTelegram->getSerializedSize());
		esc->send(objTelegram);
		delete flist;
		myFunctions->freeList();
		break;
	}
	case Token_Object::TOKEN_NEXT: {
		Token_Object* to = new Token_Object();
		objTelegram->deserialize(data, to);
		callFunction(client, to->getFunction(), myFunctions);
		break;
	}
	case Behavior_Object::BEHAVIOR: {
		Behavior_Object* behavior = new Behavior_Object();
		objTelegram->deserialize(data, behavior);
		client->setBehavior(behavior);
		break;
	}
	default: {
		LoggerAdapter::log(Log::WARNING, "no action for type " + telegram->getType());
		break;
	}
		}

	}
	return (void*) 0;
}

MPC_Client::MPC_Client(std::string path) {

	this->dmclient = new DotMatrixClient();
	DisplayList::setCommunicationModule(dmclient);

	this->list = new DisplayList(DisplayCommunication::FULL, DisplayCommunication::LEFT);

	printf("Resolution: %d x %d\n", dmclient->getXResolution(), dmclient->getYResolution());


	std::cout << "in main():\n";

	this->espart = new EventSystemClient(Telegram::ID_AUDIOPLAYER);
	espart->connectToMaster();
	espart->startReceiving();
	LoggerAdapter::initLoggerAdapter(espart);

	try
	{
		this->connection = new Connection(path);
	} catch (std::exception& ex)
	{
		LoggerAdapter::log(Log::SEVERE, ex.what());
	}

	this->playback = new Playback();
	this->playlist = new Playlist();

	int error = pthread_create(&(this->control), NULL, control_thread, this);
	if (error != 0)
	{
		LoggerAdapter::log(Log::SEVERE, "could not start control thread: " + error);
	}
}

MPC_Client::~MPC_Client() {

	int error = pthread_cancel(this->control);
	if (error != 0)
	{
		LoggerAdapter::log(Log::SEVERE, "could not send cancel to control thread: " + error);
	}

	error = pthread_join(this->control, NULL);
	if (error != 0)
	{
		LoggerAdapter::log(Log::SEVERE, "could not join control thread: " + error);
	}

	delete this->espart;
	delete this->dmclient;
	delete this->playback;
	delete this->playlist;
	delete this->connection;
	delete this->list;
}

void MPC_Client::setBehavior(TokenDaemon::Behavior_Object* behavior)
{
	this->behave = behavior;
}

EventSystemClient* MPC_Client::getESClient()
{
	return this->espart;
}

void MPC_Client::displayStatus()
{
	this->currentFocus = PLAYER;
	this->list->clear();
	this->list->addEntry(this->playback->getCurrentSongInfo(Playback::ARTIST));
	this->list->addEntry(this->playback->getCurrentSongInfo(Playback::ALBUM));
	this->list->addEntry(this->playback->getCurrentSongInfo(Playback::TRACKNR) + " - " + this->playback->getCurrentSongInfo(Playback::TITLE));
	this->list->display();
}
void MPC_Client::displayArtistList()
{
	this->currentFocus = LIST;
	this->currentList = ARTISTLIST;
	this->list->setList(this->playlist->obtainArtistByTag());
	this->list->display();
}
void MPC_Client::displayAlbumList(std::string byArtist)
{
	this->currentFocus = LIST;
	this->currentList = ALBUMLIST;
	if (byArtist.compare("") != 0)
		this->list->setList(this->playlist->obtainAlbumByTag(MPD_TAG_ALBUM_ARTIST, byArtist));
	else
		this->list->setList(this->playlist->getAlbumList());
	this->list->display();
}
void MPC_Client::displayTitleList(std::string byArtistOrAlbum)
{
	this->currentFocus = LIST;
	this->currentList = TRACKLIST;
	if (byArtistOrAlbum.compare("") != 0)
		this->list->setList(this->playlist->obtainSongsByTag(MPD_TAG_ALBUM, byArtistOrAlbum));
	else
		this->list->setList(this->playlist->getTitleList());
	this->list->display();
}

void MPC_Client::keyInput(Key::key_type key)
{
	if (key == Key::KEY_MENU)
		this->cancel();
	else
	{
		switch (this->currentFocus)
		{
		case PLAYER: this->controlPlayback(key); break;
		case LIST: this->controlList(key); break;
		default: break;
		}
	}
}

void MPC_Client::controlPlayback(Key::key_type key)
{
	switch (key)
	{
	case Key::KEY_PLAY: {
		if (this->playback->getState() == Playback::STOP)
			this->playback->setPlayback(Playback::PLAY);
		else
			this->playback->setPlayback(Playback::PLAYPAUSE);
		break;
	}
	case Key::KEY_STOP: {
		this->playback->setPlayback(Playback::STOP);
		break;
	}
	case Key::KEY_NEXT: {
		if (this->playback->getState() == Playback::STOP) {
			this->playback->setPlayback(Playback::PLAY);
			this->playback->setPlayback(Playback::PLAYPAUSE);
		}
			this->playback->setPlayback(Playback::NEXT);
		break;
	}
	case Key::KEY_PREVIOUS: {
		if (this->playback->getState() == Playback::STOP) {
			this->playback->setPlayback(Playback::PLAY);
			this->playback->setPlayback(Playback::PLAYPAUSE);
		}
		this->playback->setPlayback(Playback::PREVIOUS);
		break;
	}
	default: break;
	}

	this->displayStatus();

}

void MPC_Client::controlList(Key::key_type key)
{
	switch (key)
	{
	case Key::KEY_ENTER:
	case Key::KEY_NEXT: {
		switch (this->currentList)
		{
		case ARTISTLIST: this->displayAlbumList(this->list->getEntryAt(this->list->getSelectedEntry())); break;
		case ALBUMLIST: this->displayTitleList(this->list->getEntryAt(this->list->getSelectedEntry())); break;
		case TRACKLIST: this->controlList(Key::KEY_STOP); break;	//TODO: get id of title (getByName), setID to play
		default: break;
		}
		break;
	}

	case Key::KEY_KNOB_DOWN: this->list->scrollDown(1); this->list->display(); break;
	case Key::KEY_KNOB_UP: this->list->scrollUp(1); this->list->display(); break;

	case Key::KEY_PREVIOUS: {
		switch (this->currentList)
		{
		case ARTISTLIST: this->controlList(Key::KEY_STOP); break;
		case ALBUMLIST: this->displayArtistList(); break;
		case TRACKLIST: this->displayAlbumList(""); break;
		default: break;
		}
		break;
	}

	case Key::KEY_STOP: this->currentFocus = PLAYER; this->displayStatus(); break;

	case Key::KEY_PLAY: {	//TODO: set selection to play in new playlist // or popup, select: add to playlist, new playlist
		switch (this->currentList)
		{
		case ARTISTLIST: break;
		case ALBUMLIST: break;
		case TRACKLIST: break;
		default: break;
		}
		break;
	}

	default: break;

	}
}

void MPC_Client::cancel()
{
	Token_Object* to = new Token_Object(this->behave->getUID_onCancel(), this->behave->getFunc_onCancel());
	Telegram_Object* objTelegram = new Telegram_Object(this->behave->getTokenID(), to);
	objTelegram->setType((Telegram::telegram_type)Token_Object::TOKEN_NEXT);
	this->espart->send(objTelegram);

	delete to;
	delete objTelegram;
}

void MPC_Client::success()
{
	Token_Object* to = new Token_Object(this->behave->getUID_onSuccess(), this->behave->getFunc_onSuccess());
	Telegram_Object* objTelegram = new Telegram_Object(this->behave->getTokenID(), to);
	objTelegram->setType((Telegram::telegram_type)Token_Object::TOKEN_NEXT);
	this->espart->send(objTelegram);

	delete to;
	delete objTelegram;
}

