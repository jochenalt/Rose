/*
 * UI.h
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#ifndef SRC_UI_UI_H_
#define SRC_UI_UI_H_

class UI {
public:
	UI();
	virtual ~UI();
	static UI& getInstance();
	void setup(int argc, char *argv[]);
};



#endif /* SRC_UI_UI_H_ */
