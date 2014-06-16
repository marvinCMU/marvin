/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

#ifndef XXXMANAGER_H_
#define XXXMANAGER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "SystemParams.h"
#include "SystemUtility.h"

class XXXObject {
public:
	std::string id;
	std::string var1;
	int var2;

	XXXObject () {}
	
	~XXXObject () {}

	XXXObject (std::string id, std::string v1, int v2) {
		id = i;
		var1 = v1;
		var2 = v2;
	}

	void print () {
		std::cout << "ID: " << id << std::endl;
		std::cout << "Var1: " << var1 << std::endl;
		std::cout << "Var2: " << var2 << std::endl;
		std::cout << std::endl;
	}
};

class XXXManager {
public:
	std::vector<xxxObject> objVec;
	std::vector<std::string> cmdVec;
	std::string libPath;

	XXXManager (std::string path) {
		libPath = path;
	}

	~XXXManager () {}
	
	// load both object library and command library
	void loadData () {
		std::string objStr = loadObject ();
		std::string cmdStr = loadCommand ();
	}

	// load object library
	std::string loadObject () {
		std::string objPath = libPath + XXX_OBJ_XML;
		std::string objStr = "";
		objVec.clear();

		// check xml file
		std::ifstream objFile (objPath.c_str());
		if (!objFile.is_open()) {
    		std::cout << XXX_MAN_HEAD << "File " << objPath << " cannot be opened!" << std::endl;
			return "";
		}
		objFile.close();

		// load xml object data
		boost::property_tree::ptree tree;
		boost::property_tree::read_xml(objPath, tree, boost::property_tree::xml_parser::trim_whitespace);
		BOOST_FOREACH( boost::property_tree::ptree::value_type const& v, tree.get_child("root") ) {
		    if ( v.first == "xxx" ) {		
		        std::string id = v.second.get("<xmlattr>.id", "default");
		        std::string var1 = v.second.get<std::string>("var1");
				int var2 = v.second.get<int>("var2");
		        objVec.push_back (* new FaceObject(id, var1, var2));
		    }
		}

		// return object string
		for (int i = 0; i < objVec.size(); i++) {
			objStr = objStr + objVec[i].name;
			if (i != objVec.size()-1)
				objStr = objStr + " ";
		}
		return objStr;
	}
	
	// return the object if the the object name found
	FaceObject* findObject (std::string id) {
		for (int i = 0; i < objVec.size(); i++) {
			if (id.compare (objVec[i].id) == 0)
				return &objVec[i];
		}
		return NULL;
	}

	// insert a new object
	bool insertObject (std::string id, std::string var1, int car2) {
		objVec.push_back (* new FaceObject(id, var1, var2));
		updateObjectFile ();
		return true;
	}
	
	// delete the object with the specified name
	bool deleteObject (std::string id) {
		for (int i = 0; i < objVec.size(); i++) {
			if (id.compare (objVec[i].id) == 0) {
				objVec.erase (objVec.begin() + i);
				updateObjectFile ();
				return true;
			}
		}
		return false;
	}

	// update the object file with new object library
	void updateObjectFile () {
		// to be implemented
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
		for (int i = 0; i < objVec.size(); i++)
			objVec[i].print();
	}

	// load command library
	std::string loadCommand () {
		std::string cmdPath = libPath + XXX_CMD_XML;
		std::string cmdStr = "";
		cmdVec.clear();
		
		// load command data
		std::ifstream cmdFile (cmdPath.c_str());	
        std::string line;
		if (!cmdFile.is_open()) {
    		std::cout << XXX_MAN_HEAD << "File " << cmdPath << " cannot be opened!" << std::endl;
			return "";
		}
		while(getline(cmdFile, line)) {
			cmdVec.push_back (line);
    	}
		if (cmdFile.bad()) {
			std::cout << XXX_MAN_HEAD << "Errors while reading file " << cmdPath << std::endl;
			cmdVec.clear();
			return "";
		}
		cmdFile.close();
	
		// return command string
		for (int i = 0; i < cmdVec.size(); i++) {
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
		for (int i = 0; i < cmdVec.size(); i++) {
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
		std::string cmdPath = libPath + XXX_CMD_XML;
		std::ofstream cmdFile (cmdPath.c_str());
		if (!cmdFile.is_open()) {
    		std::cout << XXX_MAN_HEAD << "File " << cmdPath << " cannot be opened!" << std::endl;
			return;
		}
		for (int i = 0; i < cmdVec.size(); i++)
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
		for (int i = 0; i < cmdVec.size(); i++)
			std::cout << cmdVec[i] << std::endl;
	}

	// return whether any data loaded
	bool hasData () {
		return (hasObj() && hasCmd());
	}

};

/*int main (int argc, char *argv[]) {
	XXXManager *manager = new XXXManager("/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_xxx/");
	std::cout << manager->loadObject() << std::endl;
	manager->printObject();
	std::cout << manager->loadCommand() << std::endl;
	manager->printCommand();
	return 0;
}*/

#endif

