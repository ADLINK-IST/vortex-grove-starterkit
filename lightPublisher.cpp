/*
*                         Vortex OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2017 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/

#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

#include "VortexGrove_DCPS.hpp"

/* C API for GrovePi Module */
#include "grovepi.h"

using namespace std;



int main (int argc, char *argv[])
{

	/* Get the program parameters
	 * Parameters: rotaryPublisher [Number of cycles] */
	int cycles;

	if (argv[1])
	{
		cycles = atoi(argv[1]);
		cout << "=== [lightPublisher] Cycles: " << cycles << endl;
	}
	else
	{
		cycles = 10000;
		cout << "=== [lightPublisher] Cycles: 10,000, CTRL - C to exit." << endl;
	}


    /** A dds::domain::DomainParticipant is created for the default domain. */
    dds::domain::DomainParticipant participant(org::opensplice::domain::default_id());
	cout << "=== [lightPublisher] Participant Created" << endl;
	
	/* A dds::topic::qos::TopicQos is created with Reliability set to Reliable to
	* guarantee delivery. */

    dds::topic::qos::TopicQos reliableTopicQos = participant.default_topic_qos() 
		<< dds::core::policy::Reliability::Reliable();

    /* Set the Reliable TopicQos as the new default */
    participant.default_topic_qos(reliableTopicQos);
	
	/* A dds::topic::Topic is created for the Analogue Sensor types on the
	* domain participant. 
	* The Topic data type used for the sensors is defined in the IDL file.
	* Here we create a Topic of type VortexGrove::AnalogueSensor called 
	* VortexGrove_LightSensor */

	dds::topic::Topic<VortexGrove::AnalogueSensor> LightSensorTopic(participant,
		"VortexGrove_LightSensor", reliableTopicQos);
	cout << "=== [lightPublisher] Topic VortexGrove_LightSensor Created " << endl;

	/* Create a Publisher and Data Writer.
	 * This Publisher will use the Analogue Sensor Topic to pass Temperature and 
	 * Lux values into the DDS */

	dds::pub::qos::PublisherQos pubQos = participant.default_publisher_qos();

	dds::pub::Publisher lightPublisher(participant, pubQos);

	dds::pub::qos::DataWriterQos dwQos = LightSensorTopic.qos();

	dds::pub::DataWriter<VortexGrove::AnalogueSensor> sensorDW(lightPublisher, LightSensorTopic, dwQos);
	cout << "=== [lightPublisher] Publisher and DataWriter Created " << endl;

	/* Initialise Grove Pi Board */

	if (init() == -1)
	{
		cout << "=== [lightPublisher] GrovePi Failed to initialise" << endl;
		cout << "=== [lightPublisher] Check the GrovePi is online (green LED on top)\n"
			 << "=== [lightPublisher] Test the I2C port connection with command: \"i2cdetect -y 1\"" << endl;
		return 0;
	}
	else
	{
		cout << "=== [lightPublisher] Grove Pi initialised" << endl;
	}
	

    /* Create a unique reference for the Lux values.
	 * This reference is also used as the Topic Key 
	 * The unit measurement of light is called Lux */

	short Lux = 200;

	int count = 0;
	int lightData = 0;

	int lightPin = 0;

	/* Set the pins to input */
	pinMode(0, 0);

	/* Create a Topic Instance of the VortexGrove_AnalogueSensor Topic data type */
	VortexGrove::AnalogueSensor	currVal;
	
	for (int i = 0; i < cycles; i++) 
	{

		/* Read data from pin 0 */
		lightData = analogRead(lightPin);

		/* Pass the data to the Topic Instance */
		currVal.id() = Lux;
		currVal.rValue() = lightData;
		cout << "=== [lightPublisher] Sending Sample No.[" 
			<< count << "] Data: [" << lightData << "]" << endl;

		/* Send the data sample to the data writer */
		sensorDW << currVal;
		
		/* Wait for a second */
		sleep(1);
		count++;
		
	}
    return 0;
}
