/**
  * Christian Seremetis
  * 14 February 2019
  * DJI Guidance API simple position test
  */


/*****************************************************
* We don't use DJI_event or DJI_lock in this implementation
* TODO: integrate DJI_event, DJI_lock, and pthreads
******************************************************/

//Not included: must compile with libDJI_Guidance.so
//to include function implementations

//API headers
#include <DJI_guidance.h>
#include <DJI_utility.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h> //for time functions

using namespace std;

//define the serial port we're connecting to
e_vbus_index sensor_id = e_vbus1;

/** Stores just the position data so we don't have to */
typedef struct Position {
	float x;
	float y;
	float z;
} _Position;

typedef struct Velocity {
	float x;
	float y;
	float z;
} _Velocity;

ostream& operator<<(ostream& o, Position p) {
	stringstream ss;
	ss << p.x << " ";
	ss << p.y << " ";
	ss << p.z << " ";
	o << ss.str();
	return o;
}

/** Holds all the data we collect so we can use it. */
vector<Position> pdata;
vector<Velocity> vdata;

FILE* pf = fopen("position_output.csv", "w");
FILE* vf = fopen("velocity_output.csv", "w");

/** Callback optimized for velocity, ultrasonic, and motion data. */
int _callback(int data_type, int data_len, char* content) {
	//get position data if collected info is motion data
	if (e_motion == data_type && NULL != content) {
		motion *m = (motion*)content; //convert content to motion data
		Position p;
		Velocity v;

		p.x = m->position_in_global_x;
		p.y = m->position_in_global_y;
		p.z = m->position_in_global_z;
		pdata.push_back(p);

		v.x = m->velocity_in_global_x;
		v.y = m->velocity_in_global_y;
		v.z = m->velocity_in_global_z;
		vdata.push_back(v);
		
		fprintf(pf, "%f, %f, %f\n", m->position_in_global_x, m->position_in_global_y, m->position_in_global_z);

		fprintf(vf, "%f, %f, %f\n", v.x, v.y, v.z);
	}

	return 0;
}

int main(int argc, char const *argv[])
{
	fprintf(pf, "'X', 'Y', 'Z'\n");
	fprintf(vf, "'X', 'Y', 'Z'\n");
	reset_config(); //clear previous data subscriptions
	cout << "configuring reader...." << endl;

	//connect to Guidance, print if error
	int err_code = init_transfer();

	if (err_code) {
		cout << "Connected to Guidance" << endl;
	}

	//set _callback as the event handler for all Guidance events
	err_code = set_sdk_event_handler(_callback);
	
	cout << "beginning transfer...." << endl;
 	
	string command = " ";
	while(cin>>command) {
		err_code = start_transfer();
		//We gather the data and parse it via the callback
		err_code = stop_transfer();
		sleep(2500);
		if (command == "exit") {
			break;
		}
		err_code = start_transfer();
	}

	err_code = release_transfer();
	fclose(pf);
	fclose(vf);

	string option;
	
	while (cin >> option) {
		if (option == "vp") {
			int index = 0;
			stringstream os;
			cin >> index;
			cout << pdata[index] << endl;
		} else if (option == "vi") {
			cout << pdata.size() << endl;
			for (uint64_t i = 0; i < pdata.size(); i++) {
				cout << pdata[i] << endl;
			}
		}
	}

		return 0;
	}
