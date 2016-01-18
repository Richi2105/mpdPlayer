/*
 * MPCClient.h
 *
 *  Created on: Jan 16, 2016
 *      Author: richard
 */

#ifndef MPCCLIENT_H_
#define MPCCLIENT_H_

#include <Playback.h>
#include <Playlist.h>
#include <Connection.h>

#include <BehaviorObject.h>

#include <Telegram/Input/Key.h>
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
#include <pthread.h>

using namespace EventSystem;
using namespace DotMatrix;
using namespace mpdAccess;

class MPC_Client {
public:
	MPC_Client(std::string path);
	virtual ~MPC_Client();

	void setBehavior(TokenDaemon::Behavior_Object* behavior);

	void displayStatus();
	void displayArtistList();
	void displayAlbumList(std::string byArtist);
	void displayTitleList(std::string byArtistOrAlbum);

	/**
	 * new, add to existing playlist, remove playlist, play playlist ...
	 */
	void setPlaylist();

	/**
	 * import folder to medialib
	 * -make link (ln -s) into medialib folder
	 * -update medialib
	 */
	void importFolder();

	/**
	 * set Title according to title, album or artist
	 * if album or artist given, set first title of selection
	 */
	void setTitle();

	/**
	 * control main function
	 */
	void keyInput(Key::key_type key);

	/**
	 * start, stop, pause etc;
	 */
	void controlPlayback(Key::key_type key);

	/**
	 * scroll up, down, go into, return, etc;
	 */
	void controlList(Key::key_type key);

	/**
	 * called if menu button pressed
	 */
	void cancel();

	/**
	 * called if ?
	 */
	void success();

	EventSystemClient* getESClient();

private:
	enum inputFocus {PLAYER, LIST};
	inputFocus currentFocus;

	enum listFocus {ARTISTLIST, ALBUMLIST, TRACKLIST};
	listFocus currentList;

	DotMatrixClient* dmclient;
	EventSystemClient* espart;
	Connection* connection;

	TokenDaemon::Behavior_Object* behave;

	Playback* playback;
	Playlist* playlist;

	DisplayList* list;

	pthread_t control;
};

#endif /* MPCCLIENT_H_ */
