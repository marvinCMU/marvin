/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

#ifndef FACEMANAGER_H_
#define FACEMANAGER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "SystemParams.h"
#include "SystemUtility.h"

class FaceObject {
public:
	std::string id;
	std::string name;
	int gender;
	std::string location;

	FaceObject () {}
	
	~FaceObject () {}

	FaceObject (std::string i, std::string n, int g, std::string l) {
		id = i;
		name = n;
		gender = g;
		location = l;
	}

	void print () {
		std::cout << "ID: " << id << std::endl;
		std::cout << "Name: " << name << std::endl;
		std::cout << "Gender: " << gender << std::endl;
		std::cout << "Location: " << location << std::endl;
		std::cout << std::endl;
	}
};

class FaceManager {
public:
	std::vector<FaceObject> objVec;
	std::vector<std::string> cmdVec;
	std::string libPath;

	FaceManager (std::string path) {
		libPath = path;
	}

	~FaceManager () {}
	
	// load both object library and command library
	void loadData () {
		std::string objStr = loadObject ();
		std::string cmdStr = loadCommand ();
	}

	// load object library
	std::string loadObject () {
		std::string objPath = libPath + FACE_OBJ_XML;
		std::string objStr = "";
		objVec.clear();

		// check xml file
		std::ifstream objFile (objPath.c_str());
		if (!objFile.is_open()) {
    		std::cout << FACE_MAN_HEAD << "File " << objPath << " cannot be opened!" << std::endl;
			return "";
		}
		objFile.close();

		// load xml object data
		boost::property_tree::ptree tree;
		boost::property_tree::read_xml(objPath, tree, boost::property_tree::xml_parser::trim_whitespace);
		BOOST_FOREACH( boost::property_tree::ptree::value_type const& v, tree.get_child("root") ) {
		    if ( v.first == "face" ) {		
		        std::string id = v.second.get("<xmlattr>.id", "default");
		        std::string name = v.second.get<std::string>("name");
				int gender = v.second.get<int>("gender");
				std::string location = v.second.get<std::string>("location");
				changeWordCase(name, -1);
		        objVec.push_back (* new FaceObject(id, name, gender, location));
		    }
		}

		// return object string
		for (unsigned int i = 0; i < objVec.size(); i++) {
			objStr = objStr + objVec[i].name;
			if (i != objVec.size()-1)
				objStr = objStr + " ";
		}
		return objStr;
	}
	
	// return the object if the the object name found
	FaceObject* findObject (std::string name) {
		for (unsigned int i = 0; i < objVec.size(); i++) {
			if (name.compare (objVec[i].name) == 0)
				return &objVec[i];
		}
		return NULL;
	}

	// insert a new object
	bool insertObject (std::string id, std::string name, int gender, std::string location) {
		objVec.push_back (* new FaceObject(id, name, gender, location));
		updateObjectFile ();
		return true;
	}
	
	// delete the object with the specified name
	bool deleteObject (std::string name) {
		for (unsigned int i = 0; i < objVec.size(); i++) {
			if (name.compare (objVec[i].name) == 0) {
				objVec.erase (objVec.begin() + i);
				updateObjectFile ();
				return true;
			}
		}
		return false;
	}

	// update the object file with new object library
	void updateObjectFile () {
		std::string objPath = libPath + FACE_OBJ_XML;
		std::ofstream objFile (objPath.c_str());
		if (!objFile.is_open()) {
    		std::cout << FACE_MAN_HEAD << "File " << objPath << " cannot be opened!" << std::endl;
			return;
		}
		// xml headers
		objFile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << "\n";
		objFile << "<root xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" << "\n";
		// xml data
		for (unsigned int i = 0; i < objVec.size(); i++) {
			objFile << "\t<face id=\"" << objVec[i].id << "\">" << "\n";
			objFile << "\t\t<name>" << objVec[i].name << "</name>" << "\n";
			objFile << "\t\t<gender>" << objVec[i].gender << "</gender>" << "\n";
			objFile << "\t\t<location>" << objVec[i].location << "</location>" << "\n";
			objFile << "\t</face>" << "\n";
		}
		objFile << "</root>" << "\n";
		objFile.close();
	}

	// get size of the object library
	int objSize () {
		return objVec.size();
	}

	// return whether any objects loaded
	bool hasObj () {
		return (objSize() != 0);
	}

	// print the object library
	void printObject () {
		std::cout << FACE_MAN_HEAD << "Objects:" << std::endl;
		for (unsigned int i = 0; i < objVec.size(); i++)
			objVec[i].print();
		std::cout << std::endl;
	}

	// load command library
	std::string loadCommand () {
		std::string cmdPath = libPath + FACE_CMD_XML;
		std::string cmdStr = "";
		cmdVec.clear();
		
		// load command data
		std::ifstream cmdFile (cmdPath.c_str());	
        std::string line;
		if (!cmdFile.is_open()) {
    		std::cout << FACE_MAN_HEAD << "File " << cmdPath << " cannot be opened!" << std::endl;
			return "";
		}
		while(getline(cmdFile, line)) {
			cmdVec.push_back (line);
    	}
		if (cmdFile.bad()) {
			std::cout << FACE_MAN_HEAD << "Errors while reading file " << cmdPath << std::endl;
			cmdVec.clear();
			return "";
		}
		cmdFile.close();
	
		// return command string
		for (unsigned int i = 0; i < cmdVec.size(); i++) {
			cmdStr = cmdStr + cmdVec[i];
			if (i != cmdVec.size()-1)
				cmdStr = cmdStr + " ";
		}
		return cmdStr;
	}

	// insert a new command
	bool insertCommand (std::string command) {
		if (command.compare("") == 0) return false;
		cmdVec.push_back(command);
		updateCommandFile ();
		return true;
	}

	// delete the specified command
	bool deleteCommand (std::string command) {
		if (command.compare("") == 0) return false;
		for (unsigned int i = 0; i < cmdVec.size(); i++) {
			if (command.compare(cmdVec[i]) == 0) {
				cmdVec.erase(cmdVec.begin() + i);
				updateCommandFile ();
				return true;
			}
		}
		return false;
	}

	// update the command file with new command library
	void updateCommandFile () {
		std::string cmdPath = libPath + FACE_CMD_XML;
		std::ofstream cmdFile (cmdPath.c_str());
		if (!cmdFile.is_open()) {
    		std::cout << FACE_MAN_HEAD << "File " << cmdPath << " cannot be opened!" << std::endl;
			return;
		}
		for (unsigned int i = 0; i < cmdVec.size(); i++)
			cmdFile << cmdVec[i] << "\n";
		cmdFile.close();
	}

	// get size of the command library
	int cmdSize () {
		return cmdVec.size();
	}

	// return whether any commands loaded
	bool hasCmd () {
		return (cmdSize() != 0);
	}

	// print the command library
	void printCommand () {
		std::cout << FACE_MAN_HEAD << "Commands:" << std::endl;
		for (unsigned int i = 0; i < cmdVec.size(); i++)
			std::cout << cmdVec[i] << std::endl;
		std::cout << std::endl;
	}

	// return whether any data loaded
	bool hasData () {
		return (hasObj() && hasCmd());
	}
	
	// update command library, object library and speech library
	void updateFile () {
		updateCommandFile ();
		updateObjectFile ();
        updateSpeechFile ();
	}

	// update speech library
	void updateSpeechFile () {
        updateCorpusFile ();
        updateLanguageFile ();
	}

	// update the speech corpus file for commands
	void updateCorpusFile () {
		std::string lanPath = libPath + FACE_LAN;
		std::string corPath = libPath + FACE_COR;
		std::ifstream lanFile (lanPath.c_str());
		std::ofstream corFile (corPath.c_str());
	    std::string line;
		// check files
		if (!lanFile.is_open()) {
    		std::cout << FACE_MAN_HEAD << "File " << lanPath << " cannot be opened!" << std::endl;
			return;
		}
		if (!corFile.is_open()) {
    		std::cout << FACE_MAN_HEAD << "File " << corPath << " cannot be opened!" << std::endl;
			return;
		}
		// replace all occurences of [xxx] to respective strings, multiple occurences of [xxx] might occur
		while(getline(lanFile, line)) {
			// continue if no [xxx] found
			size_t index = line.find("[xxx]", 0);
			if (index == std::string::npos) {
				corFile << line << "\n";
				continue;
			}
			for (unsigned int i = 0; i < objVec.size(); i++) {
				std::string linetmp = line;
				index = linetmp.find("[xxx]", 0);
		 		while (index != std::string::npos) {
					linetmp.replace(index, 5, objVec[i].name);
					index = linetmp.find("[xxx]", index);
				}
				corFile << linetmp << "\n";
			}
    	}
		if (lanFile.bad()) {
			std::cout << FACE_MAN_HEAD << "Errors while reading file " << lanPath << std::endl;
			return;
		}
		lanFile.close();
		corFile.close();
	}

	// update the speech language model file and dictionary file
	void updateLanguageFile () {
		// to be implemented, use C++ HTTP POST techniques
	}

	// print the command library and the object library
	void printData () {
		printCommand ();
		printObject ();
	}
};

/*int main (int argc, char *argv[]) {
	FaceManager *manager = new FaceManager("/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_face/");
	std::cout << manager->loadObject() << std::endl;
	manager->printObject();
	std::cout << manager->loadCommand() << std::endl;
	manager->printCommand();
	FaceObject* f = manager->findObject("Denver");
	f->print();
	manager->insertObject("id5", "hi", 1, "das");
	manager->printObject();
	manager->deleteObject("hi");
	manager->printObject();
	return 0;
}*/

#endif

