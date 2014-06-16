#ifndef READOBJECT_H_
#define READOBJECT_H_

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include "tinyxml.h"
#include "tinystr.h"

#define DEF_URL		"http://www.google.com"

using namespace std;

class RetailObject {
public:
	string product;
	string name;
	string color;
	string price;
	string review;
	string option;

	RetailObject () {}

	RetailObject (string t, string n, string c, string p, string r, string o) {
		product = t;
		name = n;
		color = c;
		price = p;
		review = r;
		option = o;
	}

	void printRetailObject () {
		cout << "Obj: " << product << endl;
		cout << "Name: " << name << endl;
		cout << "Color: " << color << endl;
		cout << "Price: " << price << endl;
		cout << "Review: " << review << endl;
		cout << "Option: " << option << endl;
		cout << endl;
	}
};

class OfficeObject {
public:
	OfficeObject() {}
};

class ObjectDatabase {
public:
	vector<RetailObject> ros;
	int objNum;

	ObjectDatabase() {}

	void loadData(const char* pFilename, int allObjNum) {
		RetailObject ro;
		TiXmlElement *RootElement;
		TiXmlElement *FirstObject;
		TiXmlAttribute *IDAttribute;
		TiXmlElement *NameElement;
		TiXmlElement *ColorElement;
		TiXmlElement *PriceElement;
		TiXmlElement *ReviewElement;
		TiXmlElement *OptionElement;

		TiXmlDocument *myDocument = new TiXmlDocument(pFilename);
		myDocument->LoadFile();
		
		// root element
		RootElement = myDocument->RootElement();
		
		// load objects
		for (int n=0; n<allObjNum; n++ ) {
			if (n==0) {	// the first node
				FirstObject = RootElement->FirstChildElement();
			} else {	// following nodes
				FirstObject = FirstObject->NextSiblingElement();
			}
			IDAttribute = FirstObject->FirstAttribute();
			NameElement = FirstObject->FirstChildElement();
			ColorElement = NameElement->NextSiblingElement();
			PriceElement = ColorElement->NextSiblingElement();
			ReviewElement = PriceElement->NextSiblingElement();
			OptionElement = ReviewElement->NextSiblingElement();

			// save the data to the object class
			ro.product = IDAttribute->Value();
			ro.name = NameElement->FirstChild()->Value();
			ro.color = ColorElement->FirstChild()->Value();
			ro.price = PriceElement->FirstChild()->Value();
			ro.review = ReviewElement->FirstChild()->Value();
			ro.option = OptionElement->FirstChild()->Value();	

			ros.push_back(ro);
			objNum = ros.size();
		}	
	}

	void printObjectDatabase () {
		RetailObject rotemp;
		for (unsigned int i=0; i<ros.size(); i++) {
			rotemp = ros[i];
			rotemp.printRetailObject();
		}
	}

	RetailObject findObject (string s) {
		RetailObject robt;
		for (int i=0; i<objNum; i++) {
			robt = ros[i];
			if (s.compare(robt.product) == 0){
				return robt;
			}
		}
		robt.product = s;
		robt.name = "N/A";
		robt.color = "N/A";
		robt.price = "N/A";
		robt.review = DEF_URL;
		robt.option = DEF_URL;
		return robt;
	}
};

/*int main (int argc, char *argv[]) {

	ObjectDatabase *odb = new ObjectDatabase();
	odb->loadData("../../code_data/object_list.xml", 12);
	odb->printObjectDatabase();
	cout << "<Database> " << odb->objNum << " objects loaded" << endl;
	return 0; 
}*/

#endif

